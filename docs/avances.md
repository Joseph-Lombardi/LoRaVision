# Bitácora experimental

## 2026-11-16 — Captura y reconstrucción serial

**Objetivo de la prueba:** Obtener una fotografía JPEG con la ArduCam Mega 5MP-AF, imprimir sus bytes en hexadecimal desde el Heltec V3 y reconstruirlos con Python.

**Resultado observado:**
- Captura VGA (640 × 480) y salida delimitada por `INICIO_JPEG` y `FIN_JPEG`.
- Una captura reportó **34856 bytes** por el puerto serial.
- El volcado contenía el marcador JPEG `FFD9` seguido de relleno con `00`; Python recorta esos bytes.
- Se observó inicialmente una dominante verde/amarilla. Se aplicaron ajustes de balance de blancos en el firmware y se comprobó que una imagen antigua estaba siendo reconstruida por error al pegar nuevas capturas tras el mismo archivo.
- El script actual elige la **última captura completa** de un texto y guarda `foto1.jpg`, `foto2.jpg`, etc., sin sobrescribir archivos.

**Problemas resueltos durante el desarrollo:**
- Error `cc1plus.exe: CreateProcess: No such file or directory` al compilar con el entorno anterior; las compilaciones avanzaron tras utilizar el paquete ESP32 **3.2.1**.
- Error de biblioteca `ArducamSpi.cpp`: `return SPI.begin();` en una función `void`; cambio local a `SPI.begin(6, 5, 4, 7);`.

**Estado de integración:** La captura y reconstrucción por serial funcionan. El envío **completo de imágenes por LoRa no se ha validado todavía**; se habían realizado pruebas independientes de comunicación LoRa.

## Próximo hito

Transmitir el JPEG por bloques desde el Heltec emisor hacia otro Heltec, gestionar pérdidas y reconstruir automáticamente en la PC.
