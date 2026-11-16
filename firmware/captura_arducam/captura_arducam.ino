#include <Arduino.h>
#include <SPI.h>
#include <Arducam_Mega.h>

// ArduCam Mega SPI -> Heltec WiFi LoRa 32 V3 (ESP32-S3)
#define CAM_SCK  6
#define CAM_MISO 5
#define CAM_MOSI 4
#define CAM_CS   7

Arducam_Mega cam(CAM_CS);

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("Iniciando ArduCam...");
  SPI.begin(CAM_SCK, CAM_MISO, CAM_MOSI, CAM_CS);

  CamStatus estado = cam.begin();
  if (estado != CAM_ERR_SUCCESS) {
    Serial.println("ERROR: inicializacion de camara");
    return;
  }

  // Balance de blancos y parametros neutros de imagen.
  // Esta configuracion corrigio la dominante de color observada.
  cam.setAutoWhiteBalance(1);
  cam.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_MODE_DEFAULT);
  cam.setColorEffect(CAM_COLOR_FX_NONE);
  cam.setSaturation(CAM_STAURATION_LEVEL_DEFAULT);
  cam.setContrast(CAM_CONTRAST_LEVEL_DEFAULT);
  cam.setBrightness(CAM_BRIGHTNESS_LEVEL_DEFAULT);
  cam.setAutoExposure(1);

  delay(3000); // Estabilizacion del sensor

  Serial.println("Capturando fotografia...");
  estado = cam.takePicture(CAM_IMAGE_MODE_VGA, CAM_IMAGE_PIX_FMT_JPG);
  if (estado != CAM_ERR_SUCCESS) {
    Serial.println("ERROR: captura de fotografia");
    return;
  }

  uint32_t total = cam.getTotalLength();
  Serial.print("Tamano: ");
  Serial.println(total);

  if (total == 0 || total > 100000) {
    Serial.println("ERROR: tamano invalido");
    return;
  }

  uint8_t buffer[128];
  uint32_t enviados = 0;

  Serial.println("INICIO_JPEG");
  while (enviados < total) {
    uint8_t cantidad = (uint8_t)min((uint32_t)sizeof(buffer), total - enviados);
    uint8_t leidos = cam.readBuff(buffer, cantidad);
    if (leidos == 0) {
      Serial.println("ERROR: lectura interrumpida");
      break;
    }

    for (uint8_t i = 0; i < leidos; i++) {
      if (buffer[i] < 16) Serial.print("0");
      Serial.print(buffer[i], HEX);
    }
    Serial.println();
    enviados += leidos;
  }

  Serial.println("FIN_JPEG");
  Serial.print("Bytes leidos: ");
  Serial.println(enviados);
  Serial.println("Captura finalizada");
}

void loop() {
  // Una captura por reinicio.
}
