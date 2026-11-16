# LoRaVision 📷📡

Proyecto experimental de captura y futura transmisión de imágenes mediante **ArduCam Mega 5MP-AF**, **Heltec WiFi LoRa 32 V3 (ESP32-S3)** y **Python**.

## Estado del proyecto (16/11/2026)

- ✅ Captura de fotografías JPEG desde ArduCam Mega mediante SPI.
- ✅ Exportación hexadecimal del JPEG por monitor serial a 115200 baudios.
- ✅ Reconstrucción en Python desde `INICIO_JPEG` / `FIN_JPEG`, ignorando relleno posterior a `FFD9`.
- ✅ Balance de blancos automático y ajustes de imagen aplicados en el firmware de captura.
- ✅ Guardado de archivos `foto1.jpg`, `foto2.jpg`, etc., sin sobrescribir.
- ✅ Pruebas independientes de mensajes entre módulos Heltec mediante LoRa.
- 🚧 **Pendiente:** unir captura con fragmentación/transmisión LoRa, recepción íntegra y reconstrucción automática sin pegar texto.

> Importante: la transmisión *completa* de fotografías por LoRa todavía no está validada. El firmware de este repositorio contiene el módulo de captura por USB/serial; no afirma transmisión de imágenes ya implementada.

## Estructura

```text
firmware/
  captura_arducam/
    captura_arducam.ino
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
2. Transmitir paquetes desde el Heltec emisor por LoRa (915 MHz).
3. Reconstruir la imagen con integridad/completitud en el receptor.
4. Entregar los datos a Python para generar fotos automáticamente.
5. Medir tasa efectiva, tiempos, pérdida de paquetes y fiabilidad.

## Licencia

Proyecto personal en desarrollo. Dependencias y bibliotecas externas conservan sus respectivas licencias.
