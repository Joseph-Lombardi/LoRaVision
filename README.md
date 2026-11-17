# LoRaVision 📷📡

Proyecto experimental de captura y transmisión inalámbrica progresiva de imágenes mediante **ArduCam Mega 5MP-AF**, **Heltec WiFi LoRa 32 V3 (ESP32-S3)** y **Python**.

## Estado del proyecto (16–17/11/2026)

- ✅ Captura de fotografías JPEG desde ArduCam Mega mediante SPI.
- ✅ Exportación hexadecimal del JPEG por monitor serial a 115200 baudios.
- ✅ Reconstrucción en Python desde `INICIO_JPEG` / `FIN_JPEG`, ignorando relleno posterior a `FFD9`.
- ✅ Balance de blancos automático y ajustes de imagen aplicados en el firmware de captura.
- ✅ Guardado de archivos `foto1.jpg`, `foto2.jpg`, etc., sin sobrescribir.
- ✅ Prueba de integración: **32 bytes iniciales de un JPEG real transmitidos y recibidos por LoRa a 915 MHz** entre dos Heltec V3.
- ✅ Confirmación de transmisión por `OnTxDone()` y recepción de payload idéntico, con RSSI **−15 dBm** en la prueba de proximidad.
- 🚧 **Pendiente:** enviar el JPEG completo en fragmentos, validar integridad, gestionar pérdidas y reconstruir automáticamente sin pegar texto.

> Importante: la transmisión *completa* de fotografías por LoRa todavía no está validada. Se ha validado **solo un paquete de 32 bytes reales del JPEG** por LoRa, no una fotografía completa. La versión de prueba de emisor/receptor se conserva bajo `firmware/prueba_lora_jpeg/`.

## Estructura

```text
firmware/
  captura_arducam/
    captura_arducam.ino
  prueba_lora_jpeg/
    emisor_lora.ino
    receptor_lora.ino
    README.md
python/
  reconstruir.py
docs/
  conexiones.md
  avances.md
.gitignore
```

## Hardware

- Placa Heltec WiFi LoRa 32 **V3**, ESP32-S3.
- Cámara **ArduCam Mega 5MP-AF** con interfaz SPI.
- Cable USB para leer el puerto serial y PC con Python 3.
- Segundo Heltec V3 para la fase LoRa.

## Compilación Arduino

1. Instalar **ESP32 by Espressif Systems 3.2.1**, seleccionar **Heltec WiFi LoRa 32(V3)**.
2. Instalar `Arducam_Mega` (se trabajó con 3.0.0).
3. Abrir `firmware/captura_arducam/captura_arducam.ino` y cargarlo.
4. Monitor serial a **115200**; copiar la salida desde `INICIO_JPEG` hasta `FIN_JPEG` en `captura1.txt`.

**Nota de compatibilidad:** en la instalación usada se corrigió localmente `ArducamSpi.cpp`, reemplazando `return SPI.begin();` dentro de `void arducamSpiBegin()` por `SPI.begin(6, 5, 4, 7);`. No se incluye una copia modificada de la biblioteca externa.

## Reconstruir una fotografía

Desde la carpeta `python`, poner `captura1.txt` (puede incluir todo el registro serial) y ejecutar:

```powershell
python reconstruir.py
```

Se utiliza la **última captura completa** del texto y se guarda como `fotoN.jpg` con el primer número libre. Si la captura anterior permanece en el texto, el script igualmente identifica la última captura completa. Los archivos `captura1.txt` y `foto*.jpg` se excluyen de Git mediante `.gitignore` por defecto.

## Próximas etapas

1. Fragmentar JPEG en paquetes numerados.
2. Extender la prueba **ya validada de 32 bytes** a transmisión de múltiples paquetes LoRa (915 MHz).
3. Reconstruir la imagen con integridad/completitud en el receptor.
4. Entregar los datos a Python para generar fotos automáticamente.
5. Medir tasa efectiva, tiempos, pérdida de paquetes y fiabilidad.

## Licencia

Proyecto personal en desarrollo. Dependencias y bibliotecas externas conservan sus respectivas licencias.
