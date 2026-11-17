# Firmware de transmisión JPEG completa

**Estado:** versión funcional reportada en hardware el 16 de noviembre de 2026.

- [Emisor](emisor_lora/emisor_lora.ino): captura VGA JPEG, lectura en RAM, SPI.end(), paquetes de 64 bytes y ACK/reintentos.
- [Receptor](receptor_lora/receptor_lora.ino): recepción, ACK, secuencias y datos `LV_*` al puerto serial.
- [Python](../../python/recibir_imagen.py): reconstrucción automática en PC.

**Importante:** antes de compilar, instalar las dependencias y considerar el ajuste local a Arducam_Mega descrito en el README raíz. Los pines y frecuencia corresponden al montaje experimental de dos Heltec V3.

**Guía, resultados y limitaciones:** [Transmisión de imágenes completas](../../docs/transmision_completa.md).

La versión de 32 bytes se conserva en [prueba_lora_jpeg](../prueba_lora_jpeg/).
