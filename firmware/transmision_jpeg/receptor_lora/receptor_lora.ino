#include <Arduino.h>
#include "LoRaWan_APP.h"

// LoRaVision - receptor de paquetes JPEG con confirmacion ACK
// Heltec WiFi LoRa 32 V3. Envia lineas LV_* al PC por USB serial.

#define RF_FREQUENCY 915000000
#define TX_POWER 14
#define LORA_BW 0
#define LORA_SF 7
#define LORA_CR 1

#define PKT_START 0xA1
#define PKT_DATA  0xA2
#define PKT_END   0xA3
#define PKT_ACK   0xB1

static RadioEvents_t RadioEvents;

bool escuchar = true;
bool ackPendiente = false;
uint16_t secuenciaACK = 0;

bool imagenActiva = false;
bool imagenTerminada = false;
uint32_t longitudEsperada = 0;
uint32_t bytesRecibidos = 0;
uint16_t siguienteSecuencia = 0;

void OnTxDone() {
  Radio.Sleep();
  escuchar = true;
}

void OnTxTimeout() {
  Radio.Sleep();
  escuchar = true;
}

void OnRxDone(uint8_t *payload, uint16_t size,
              int16_t rssi, int8_t snr) {
  Radio.Sleep();

  if (size == 0) {
    escuchar = true;
    return;
  }

  uint8_t tipo = payload[0];

  if (tipo == PKT_START && size == 5) {
    uint32_t nuevaLongitud =
      ((uint32_t)payload[1] << 24) |
      ((uint32_t)payload[2] << 16) |
      ((uint32_t)payload[3] << 8) |
      payload[4];

    if (!imagenActiva || imagenTerminada) {
      if (nuevaLongitud == 0 || nuevaLongitud > 70000) {
        escuchar = true;
        return;
      }

      longitudEsperada = nuevaLongitud;
      bytesRecibidos = 0;
      siguienteSecuencia = 0;
      imagenActiva = true;
      imagenTerminada = false;

      Serial.printf("LV_BEGIN,%lu\n",
                    (unsigned long)longitudEsperada);
    }
    secuenciaACK = 0xFFFF;
    ackPendiente = true;
    return;
  }

  if (tipo == PKT_DATA && size >= 5 && imagenActiva) {
    uint16_t secuencia =
      ((uint16_t)payload[1] << 8) | payload[2];
    uint8_t cantidad = payload[3];

    if (cantidad == 0 || cantidad > 64 ||
        size != cantidad + 4) {
      escuchar = true;
      return;
    }

    if (secuencia == siguienteSecuencia &&
        bytesRecibidos + cantidad <= longitudEsperada) {
      Serial.printf("LV_DATA,%u,", secuencia);

      for (uint8_t i = 0; i < cantidad; i++) {
        if (payload[i + 4] < 16) Serial.print("0");
        Serial.print(payload[i + 4], HEX);
      }
      Serial.println();

      bytesRecibidos += cantidad;
      siguienteSecuencia++;

      secuenciaACK = secuencia;
      ackPendiente = true;
      return;
    }

    // Un ACK perdido produce retransmision del mismo paquete.
    // Se confirma el duplicado pero no se imprime nuevamente.
    if (secuencia < siguienteSecuencia) {
      secuenciaACK = secuencia;
      ackPendiente = true;
      return;
    }

    escuchar = true;
    return;
  }

  if (tipo == PKT_END && size == 1 && imagenActiva) {
    if (bytesRecibidos == longitudEsperada) {
      if (!imagenTerminada) {
        Serial.printf("LV_END,%lu\n",
                      (unsigned long)bytesRecibidos);
        imagenTerminada = true;
      }

      secuenciaACK = 0xFFFE;
      ackPendiente = true;
      return;
    }
  }
  escuchar = true;
}

void OnRxTimeout() { escuchar = true; }
void OnRxError() { escuchar = true; }

void setup() {
  Serial.begin(115200);
  delay(1500);

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

  Serial.println("=== LoRaVision: Receptor completo ===");
  Serial.println("Esperando fotografia...");
}

void loop() {
  Radio.IrqProcess();

  if (ackPendiente) {
    ackPendiente = false;
    uint8_t ack[3] = {
      PKT_ACK,
      (uint8_t)(secuenciaACK >> 8),
      (uint8_t)secuenciaACK
    };

    escuchar = false;
    Radio.Send(ack, sizeof(ack));
  }

  if (escuchar) {
    escuchar = false;
    Radio.Rx(0);
  }
  delay(1);
}
