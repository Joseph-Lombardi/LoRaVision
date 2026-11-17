#include <Arduino.h>
#include <SPI.h>
#include <Arducam_Mega.h>
#include "LoRaWan_APP.h"

// LoRaVision - transmision de JPEG completo con ACK por paquete
// Heltec WiFi LoRa 32 V3 + ArduCam Mega 5MP-AF
// Capturar imagen completa en RAM antes de liberar SPI de la camara.

#define CAM_SCK 6
#define CAM_MISO 5
#define CAM_MOSI 4
#define CAM_CS 7

#define RF_FREQUENCY 915000000
#define TX_POWER 14
#define LORA_BW 0
#define LORA_SF 7
#define LORA_CR 1

#define MAX_JPEG 70000
#define CHUNK_SIZE 64
#define MAX_RETRIES 5
#define ACK_TIMEOUT 3000

#define PKT_START 0xA1
#define PKT_DATA  0xA2
#define PKT_END   0xA3
#define PKT_ACK   0xB1

Arducam_Mega cam(CAM_CS);
static RadioEvents_t RadioEvents;

uint8_t *imagen = nullptr;
uint32_t imagenLen = 0;

volatile bool txFinished = false;
volatile bool txFailed = false;
volatile bool ackReceived = false;
volatile uint16_t ackSequence = 0;

void OnTxDone() {
  Radio.Sleep();
  txFinished = true;
}

void OnTxTimeout() {
  Radio.Sleep();
  txFailed = true;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  if (size == 3 && payload[0] == PKT_ACK) {
    ackSequence = ((uint16_t)payload[1] << 8) | payload[2];
    ackReceived = true;
  }
  Radio.Sleep();
}

void OnRxTimeout() { Radio.Sleep(); }
void OnRxError() { Radio.Sleep(); }

bool enviarSeguro(uint8_t *datos, uint8_t longitud, uint16_t secuencia) {
  for (int intento = 1; intento <= MAX_RETRIES; intento++) {
    txFinished = false;
    txFailed = false;
    ackReceived = false;

    Radio.Sleep();
    Radio.Send(datos, longitud);

    unsigned long inicio = millis();
    while (!txFinished && !txFailed && millis() - inicio < 5000) {
      Radio.IrqProcess();
      delay(1);
    }

    if (txFinished && !txFailed) {
      Radio.Rx(0);
      inicio = millis();

      while (millis() - inicio < ACK_TIMEOUT) {
        Radio.IrqProcess();
        if (ackReceived && ackSequence == secuencia) {
          Radio.Sleep();
          return true;
        }
        delay(1);
      }
    }

    Serial.printf("Reintentando paquete %u (%d/%d)\n",
                  secuencia, intento, MAX_RETRIES);
    Radio.Sleep();
    delay(150);
  }
  return false;
}

void configurarRadio() {
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetTxConfig(
    MODEM_LORA, TX_POWER, 0, LORA_BW, LORA_SF, LORA_CR,
    8, false, true, 0, 0, false, 3000
  );

  Radio.SetRxConfig(
    MODEM_LORA, LORA_BW, LORA_SF, LORA_CR,
    0, 8, 0, false, 0, true, 0, 0, false, true
  );
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== LoRaVision: Emisor completo ===");

  SPI.begin(CAM_SCK, CAM_MISO, CAM_MOSI, CAM_CS);

  if (cam.begin() != CAM_ERR_SUCCESS) {
    Serial.println("ERROR: ArduCam");
    return;
  }

  cam.setAutoWhiteBalance(1);
  cam.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_MODE_DEFAULT);
  cam.setColorEffect(CAM_COLOR_FX_NONE);
  cam.setSaturation(CAM_STAURATION_LEVEL_DEFAULT);
  cam.setContrast(CAM_CONTRAST_LEVEL_DEFAULT);
  cam.setBrightness(CAM_BRIGHTNESS_LEVEL_DEFAULT);
  cam.setAutoExposure(1);
  delay(3000);

  Serial.println("Capturando fotografia...");
  if (cam.takePicture(CAM_IMAGE_MODE_VGA, CAM_IMAGE_PIX_FMT_JPG)
      != CAM_ERR_SUCCESS) {
    Serial.println("ERROR: captura");
    return;
  }

  uint32_t total = cam.getTotalLength();
  if (total < 4 || total > MAX_JPEG) {
    Serial.printf("Tamano no admitido: %lu\n", (unsigned long)total);
    return;
  }

  imagen = (uint8_t *)malloc(total);
  if (!imagen) {
    Serial.println("ERROR: memoria insuficiente");
    return;
  }

  uint32_t leidosTotal = 0;
  while (leidosTotal < total) {
    uint8_t cantidad = (uint8_t)min((uint32_t)128, total - leidosTotal);
    uint8_t leidos = cam.readBuff(imagen + leidosTotal, cantidad);

    if (leidos == 0) {
      Serial.println("ERROR: lectura interrumpida");
      free(imagen);
      imagen = nullptr;
      return;
    }
    leidosTotal += leidos;
  }

  // Recortar relleno que puede aparecer despues del marcador final JPEG.
  for (uint32_t i = 2; i < leidosTotal; i++) {
    if (imagen[i - 1] == 0xFF && imagen[i] == 0xD9) {
      imagenLen = i + 1;
      break;
    }
  }

  if (imagenLen == 0 || imagen[0] != 0xFF || imagen[1] != 0xD8) {
    Serial.println("ERROR: JPEG incompleto");
    free(imagen);
    imagen = nullptr;
    return;
  }
  Serial.printf("JPEG real: %lu bytes\n", (unsigned long)imagenLen);

  // Solucion de integracion validada: soltar SPI de la camara.
  digitalWrite(CAM_CS, HIGH);
  SPI.end();
  delay(200);
  Serial.println("SPI de camara liberado");

  configurarRadio();
  delay(500);

  uint8_t inicioPkt[5] = {
    PKT_START,
    (uint8_t)(imagenLen >> 24),
    (uint8_t)(imagenLen >> 16),
    (uint8_t)(imagenLen >> 8),
    (uint8_t)imagenLen
  };
  Serial.println("Iniciando transferencia...");

  if (!enviarSeguro(inicioPkt, 5, 0xFFFF)) {
    Serial.println("ERROR: receptor no confirma inicio");
    return;
  }

  uint8_t paquete[4 + CHUNK_SIZE];
  uint16_t secuencia = 0;
  for (uint32_t offset = 0; offset < imagenLen; offset += CHUNK_SIZE) {
    uint8_t cantidad = (uint8_t)min(
      (uint32_t)CHUNK_SIZE, imagenLen - offset
    );

    paquete[0] = PKT_DATA;
    paquete[1] = secuencia >> 8;
    paquete[2] = secuencia & 0xFF;
    paquete[3] = cantidad;
    memcpy(paquete + 4, imagen + offset, cantidad);

    if (!enviarSeguro(paquete, cantidad + 4, secuencia)) {
      Serial.printf("ERROR en paquete %u\n", secuencia);
      return;
    }
    Serial.printf("Paquete %u confirmado (%lu/%lu bytes)\n",
                  secuencia,
                  (unsigned long)(offset + cantidad),
                  (unsigned long)imagenLen);
    secuencia++;
  }

  uint8_t finPkt[1] = {PKT_END};
  if (enviarSeguro(finPkt, 1, 0xFFFE)) {
    Serial.println("FOTOGRAFIA ENVIADA COMPLETAMENTE");
  } else {
    Serial.println("ERROR: fin no confirmado");
  }

  free(imagen);
  imagen = nullptr;
}

void loop() {
  Radio.IrqProcess();
}
