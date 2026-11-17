#include <Arduino.h>
#include <SPI.h>
#include <Arducam_Mega.h>
#include "LoRaWan_APP.h"

// LoRaVision: prueba experimental de los primeros 32 bytes de un JPEG.
// Placa: Heltec WiFi LoRa 32 V3 (ESP32-S3).
// Camara SPI: SCK 6, MISO 5, MOSI 4, CS 7.

#define CAM_SCK 6
#define CAM_MISO 5
#define CAM_MOSI 4
#define CAM_CS 7

#define RF_FREQUENCY 915000000
#define TX_POWER 14
#define LORA_BW 0
#define LORA_SF 7
#define LORA_CR 1
#define PACKET_SIZE 32

Arducam_Mega cam(CAM_CS);
static RadioEvents_t RadioEvents;
uint8_t paquete[PACKET_SIZE];
bool enviado = false;
bool radioLista = false;

void OnTxDone() {
  Serial.println("Paquete transmitido por LoRa");
  Radio.Sleep();
}

void OnTxTimeout() {
  Serial.println("ERROR: tiempo de envio agotado");
  Radio.Sleep();
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("=== LoRaVision: Emisor ===");

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
  Serial.print("Tamano JPEG: ");
  Serial.println(total);
  if (total < PACKET_SIZE) {
    Serial.println("ERROR: imagen demasiado pequena");
    return;
  }

  uint8_t leidos = cam.readBuff(paquete, PACKET_SIZE);
  if (leidos != PACKET_SIZE) {
    Serial.println("ERROR: lectura de JPEG");
    return;
  }

  Serial.println("Primeros bytes capturados:");
  for (int i = 0; i < PACKET_SIZE; i++) {
    if (paquete[i] < 16) Serial.print("0");
    Serial.print(paquete[i], HEX);
  }
  Serial.println();

  // Cambio clave: liberar SPI de la camara antes de inicializar SX1262.
  digitalWrite(CAM_CS, HIGH);
  SPI.end();
  delay(200);

  Serial.println("SPI de camara liberado");
  Serial.println("Inicializando radio LoRa...");

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(
    MODEM_LORA, TX_POWER, 0, LORA_BW, LORA_SF, LORA_CR,
    8, false, true, 0, 0, false, 3000
  );

  radioLista = true;
}

void loop() {
  if (!radioLista) {
    delay(1000);
    return;
  }

  if (!enviado) {
    enviado = true;
    Serial.println("Enviando primeros 32 bytes...");
    Radio.Send(paquete, PACKET_SIZE);
  }

  Radio.IrqProcess();
}
