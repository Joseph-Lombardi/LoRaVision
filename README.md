# LoRaVision 📷📡

Proyecto experimental de captura, transmisión inalámbrica y reconstrucción automática de imágenes mediante **ArduCam Mega 5MP-AF**, **Heltec WiFi LoRa 32 V3 (ESP32-S3)** y **Python**.

## Estado del proyecto (16–17/11/2026)

- ✅ Captura de fotografías JPEG desde ArduCam Mega mediante SPI.
- ✅ Exportación hexadecimal del JPEG por monitor serial a 115200 baudios.
- ✅ Reconstrucción en Python desde `INICIO_JPEG` / `FIN_JPEG`, ignorando relleno posterior a `FFD9`.
- ✅ Balance de blancos automático y ajustes de imagen aplicados en el firmware de captura.
- ✅ Guardado de archivos `foto1.jpg`, `foto2.jpg`, etc., sin sobrescribir.
- ✅ Prueba de integración: **32 bytes iniciales de un JPEG real transmitidos y recibidos por LoRa a 915 MHz** entre dos Heltec V3.
- ✅ Confirmación de transmisión por `OnTxDone()` y recepción de payload idéntico, con RSSI **−15 dBm** en la prueba de proximidad.
- ✅ **Transmisión de un JPEG completo y reconstrucción automática en Python**, comprobadas en una ejecución de extremo a extremo reportada por el desarrollador (16/11/2026).
- ✅ Fragmentación en bloques de 64 bytes, ACK y reintentos por paquete; guardado automático `fotoN.jpg` en PC.
- 🚧 **Pendiente:** mediciones de rendimiento/alcance, pruebas sistemáticas de pérdida de paquetes e integridad bit a bit; robustez ante transferencias interrumpidas.

 > **Nivel de evidencia:** el desarrollador confirmó que la imagen completa se transmitió y reconstruyó correctamente. La validación cuantitativa y la repetibilidad en condiciones variables están pendientes. Se preserva el prototipo previo de 32 bytes.

## Estructura

```text
firmware/
  captura_arducam/
    captura_arducam.ino
  prueba_lora_jpeg/
    emisor_lora.ino
    receptor_lora.ino
    README.md
  transmision_jpeg/
    emisor_lora/emisor_lora.ino
    receptor_lora/receptor_lora.ino
    README.md
python/
  reconstruir.py
  recibir_imagen.py
docs/
  conexiones.md
  avances.md
  transmision_completa.md
  problemas_compilacion_antivirus.md
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

## Transmisión LoRa y reconstrucción automática (versión funcional)

1. Cargar [`receptor_lora.ino`](firmware/transmision_jpeg/receptor_lora/receptor_lora.ino) en el receptor Heltec V3 y [`emisor_lora.ino`](firmware/transmision_jpeg/emisor_lora/emisor_lora.ino) en el emisor con ArduCam; conectar antenas LoRa.
2. Cerrar el monitor serial del receptor y comprobar su puerto COM.
3. Instalar PySerial: `python -m pip install pyserial` (si `pip` no existe como comando directo, invocarlo así mediante Python).
4. Ajustar `PUERTO` en [`python/recibir_imagen.py`](python/recibir_imagen.py) y ejecutar `python recibir_imagen.py`.
5. Con Python escuchando, reiniciar el emisor; las fotografías se guardan como `fotoN.jpg`.

Detalles técnicos y limitaciones: [transmisión completa](docs/transmision_completa.md). Incidencia del compilador y posible interferencia antivirus: [reporte](docs/problemas_compilacion_antivirus.md).

## Reconstruir una fotografía

Desde la carpeta `python`, poner `captura1.txt` (puede incluir todo el registro serial) y ejecutar:

```powershell
python reconstruir.py
```

Se utiliza la **última captura completa** del texto y se guarda como `fotoN.jpg` con el primer número libre. Si la captura anterior permanece en el texto, el script igualmente identifica la última captura completa. Los archivos `captura1.txt` y `foto*.jpg` se excluyen de Git mediante `.gitignore` por defecto.

## Próximas etapas

1. Medir duración, transferencia efectiva, tasa de reintentos y pérdida de paquetes.
2. Comparar hashes de fotografías de origen y destino y añadir verificación de integridad extremo a extremo.
3. Implementar recuperación tras interrupciones de sesión, validación a distintas distancias y reintentos robustos.
4. Automatizar captura periódica, gestión de energía e integración con aplicación de monitoreo.

## Licencia

Proyecto personal en desarrollo. Dependencias y bibliotecas externas conservan sus respectivas licencias.
