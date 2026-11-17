# Prueba LoRa: primeros 32 bytes JPEG

Esta carpeta guarda una **reconstrucción documentada de los sketches experimentales** utilizados en la prueba observada entre el Heltec emisor (COM9) y el receptor (COM11) el 16–17 de noviembre de 2026. La prueba física fue exitosa, pero los archivos de este repositorio no se han vuelto a compilar después de publicarlos.

## Cómo repetirla

1. Conectar la ArduCam Mega 5MP-AF al emisor: SCK=6, MISO=5, MOSI=4, CS=7, además de alimentación y tierra adecuados.
2. Conectar **antenas LoRa a ambos equipos** antes de transmitir.
3. Compilar y cargar `receptor_lora.ino` al segundo Heltec V3; monitor serial 115200, esperar `Esperando paquete JPEG...`.
4. Compilar y cargar `emisor_lora.ino` al Heltec con cámara, también a 115200.
5. Reiniciar el emisor cuando el receptor ya esté escuchando.
6. Comparar los primeros 32 bytes en hexadecimal del emisor y receptor.

Bibliotecas usadas en las pruebas: `Arducam_Mega 3.0.0`, `Heltec ESP32 Dev-Boards 2.1.6`, paquete ESP32 3.2.1. En el entorno local se corrigió previamente la rutina `arducamSpiBegin` de ArduCam para el ESP32-S3 (ver README principal).

## Resultado

- Imagen capturada: 10 280 bytes
- Fragmento enviado/recibido: 32 bytes
- Marcador JPEG inicial: `FFD8`
- RSSI observado: −15 dBm (prueba de proximidad)
- Confirmaciones: `Paquete transmitido por LoRa` y `=== PAQUETE RECIBIDO ===`

La liberación de SPI (`digitalWrite(CAM_CS, HIGH); SPI.end();`) antes de `Mcu.begin(...)` fue decisiva en esta configuración experimental.

## Alcance y limitaciones

**Este experimento no transfiere la imagen completa.** Solo toma la fotografía, lee sus primeros 32 bytes, libera el SPI de la cámara y envía un paquete LoRa. Todavía hacen falta fragmentación, control de orden, integridad, manejo de pérdidas y reconstrucción desde la recepción.

Detalles y evidencia textual: [Bitácora](../../docs/avances.md).
