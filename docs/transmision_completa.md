# Transmisión completa de fotografías JPEG por LoRa

**Fecha:** 16 de noviembre de 2026 (hora de Perú)  
**Estado:** demostración funcional reportada en hardware por el desarrollador.  
**Evidencia disponible:** confirmación de que se transmitió una imagen y se reconstruyó directamente en Python; no se adjuntó en esta etapa un nuevo registro de tiempos, tamaño de esa imagen, RSSI ni un archivo JPEG resultante.

## Objetivo y resultado

Se integró una **ArduCam Mega 5MP-AF** conectada por SPI a un **Heltec WiFi LoRa 32 V3 (ESP32-S3)** con un segundo Heltec V3 como receptor. El emisor toma una fotografía JPEG, la conserva temporalmente en memoria, libera el SPI de la cámara e inicializa la radio SX1262. Después divide el JPEG en paquetes numerados de hasta **64 bytes de datos**, los envía mediante LoRa a **915 MHz** y espera una confirmación ACK de cada paquete. El receptor imprime los datos recibidos en formato `LV_BEGIN`, `LV_DATA` y `LV_END`; Python escucha el puerto serial y genera `foto1.jpg`, `foto2.jpg`, etc., automáticamente.

**Resultado comunicado por el desarrollador:** la fotografía completa llegó al receptor y se reconstruyó automáticamente en Python. Esto confirma una primera ejecución funcional del flujo de extremo a extremo; todavía no es una validación cuantitativa de fiabilidad, alcance, duración ni porcentaje de pérdida de paquetes.

## Arquitectura

```text
ArduCam Mega 5MP-AF
   | SPI: SCK 6, MISO 5, MOSI 4, CS 7
   v
Heltec V3 EMISOR
   | JPEG en RAM -> liberar SPI de camara -> SX1262
   | Paquetes JPEG de hasta 64 bytes, numerados, ACK/reintentos
   v
       LoRa P2P 915 MHz
   v
Heltec V3 RECEPTOR
   | Validar orden, deduplicar, responder ACK
   | Volcado hexadecimal estructurado por USB serial a 115200
   v
PC + Python + pyserial
   | Comprobar longitud y marcadores JPEG FFD8 / FFD9
   v
fotoN.jpg
```

## Archivos de esta etapa

- [Emisor](../firmware/transmision_jpeg/emisor_lora/emisor_lora.ino)
- [Receptor](../firmware/transmision_jpeg/receptor_lora/receptor_lora.ino)
- [Reconstrucción Python](../python/recibir_imagen.py)
- [Prueba previa de 32 bytes](../firmware/prueba_lora_jpeg/README.md)

La prueba anterior de **32 bytes** se conserva sin reemplazarla. Estos nuevos archivos reproducen los códigos compartidos y utilizados durante la etapa de transferencia completa. No se ha ejecutado una compilación o ensayo independiente desde el repositorio tras su publicación.

## Dependencias y configuración

- Arduino IDE; placa **Heltec WiFi LoRa 32(V3)**.
- Paquete **esp32 by Espressif Systems 3.2.1**.
- Biblioteca **Arducam_Mega 3.0.0** (con ajuste local a `ArducamSpi.cpp` descrito en el README principal).
- Biblioteca **Heltec ESP32 Dev-Boards 2.1.6**.
- **Python 3** y `pyserial`, instalado mediante `python -m pip install pyserial`.
- Frecuencia: **915 MHz**; configuración LoRa **BW = 0, SF = 7, CR = 1**, preámbulo 8, potencia de transmisión configurada a 14 dBm.
- Puerto serial **115200 baudios**. En las pruebas: emisor COM9, receptor COM11. Estos puertos pueden cambiar según el equipo.

## Procedimiento de reproducción

1. Conectar antenas LoRa a ambos Heltec. Conectar la ArduCam al emisor y comprobar alimentación.
2. Compilar/cargar el firmware receptor, luego el emisor.
3. Cerrar el monitor serial de Arduino correspondiente al receptor; Python requiere acceso exclusivo a su puerto COM.
4. En el equipo, ejecutar `python -m pip install pyserial` y luego `python recibir_imagen.py` (ajustar `PUERTO` si cambió).
5. Cuando Python indique `Escuchando COM11...`, reiniciar el emisor para tomar la fotografía y comenzar la transferencia.
6. Verificar que Python guarde un archivo `fotoN.jpg` y que pueda abrirse.

## Detalles técnicos y límites conocidos

- El emisor almacena el JPEG en RAM con `malloc`, hasta **70 000 bytes** según la configuración. Una captura mayor se rechaza.
- Se identifica el final JPEG `FFD9` y se descarta cualquier relleno posterior.
- Protocolo: `A1` comienzo con longitud de 4 bytes; `A2` dato con índice de 2 bytes y tamaño; `A3` fin; `B1` ACK con índice.
- El emisor reintenta hasta cinco veces por paquete y usa un timeout de ACK configurado a 3 segundos.
- El receptor evita escribir de nuevo los duplicados; Python reconstruye bytes a partir de las líneas `LV_DATA`.
- La verificación final en Python comprueba longitud y marcadores `FFD8`/`FFD9`, pero **no aplica checksum/CRC extremo a extremo del JPEG**. El PHY LoRa tiene su propia detección de errores según configuración.
- La aplicación captura una imagen al iniciar el emisor. No hay aún intervalo de captura autónomo ni funcionamiento sumergido probado.
- El receptor actual no define un procedimiento explícito de recuperación total cuando una transferencia es abandonada a mitad de imagen.
- Mantener la misma configuración de LoRa en ambos equipos y considerar restricciones regulatorias locales antes de pruebas en exteriores.

## Próximos ensayos recomendados

Medir tiempo total de transmisión, tamaño de fotografías, cantidad de reintentos, tasa efectiva, pérdida, distancias, consumo y robustez con reinicios. Registrar fotografías originales/recibidas y comparar hashes para evaluar integridad bit a bit. Comprobar recuperación ante pérdida de ACK y transferencia incompleta.

## Problemas del entorno de compilación

Véase [Problemas de compilación y antivirus](problemas_compilacion_antivirus.md): se observaron fallos de ejecución del compilador y un cambio posterior del proveedor antivirus, pero **no se ha demostrado que el antivirus fuera la única causa**.
