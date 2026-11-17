#include <Arduino.h>
#include "LoRaWan_APP.h"

// LoRaVision: receptor de prueba para los 32 bytes iniciales de un JPEG.
#define RF_FREQUENCY 915000000
#define LORA_BW 0
#define LORA_SF 7
#define LORA_CR 1

static RadioEvents_t RadioEvents;
bool iniciarRecepcion = true;

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  Serial.println();
  Serial.println("=== PAQUETE RECIBIDO ===");
  Serial.print("Bytes: ");
  Serial.println(size);
  Serial.print("RSSI: ");
  Serial.println(rssi);
  Serial.print("HEX: ");
  for (uint16_t i = 0; i < size; i++) {
    if (payload[i] < 16) Serial.print("0");
    Serial.print(payload[i], HEX);
  }
  Serial.println();
  Serial.println("========================");
  iniciarRecepcion = true;
}

void OnRxTimeout() { iniciarRecepcion = true; }
void OnRxError() {
  Serial.println("Error de recepcion");
  iniciarRecepcion = true;
}

void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.RxDone = OnRxDone;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetRxConfig(
    MODEM_LORA, LORA_BW, LORA_SF, LORA_CR,
    0, 8, 0, false, 0, true, 0, 0, false, true
  );

  Serial.println("=== LoRaVision: Receptor ===");
  Serial.println("Esperando paquete JPEG...");
}

void loop() {
  if (iniciarRecepcion) {
    iniciarRecepcion = false;
    Radio.Rx(0);
  }
  Radio.IrqProcess();
}
