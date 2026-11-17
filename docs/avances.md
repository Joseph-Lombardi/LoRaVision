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

## 2026-11-16/17 — Primer paquete de JPEG real recibido por LoRa

**Objetivo:** Integrar ArduCam Mega 5MP-AF con un Heltec WiFi LoRa 32 V3 emisor y transmitir los **primeros 32 bytes** de un JPEG a un segundo Heltec V3 receptor.

### Montaje y configuración

- Emisor: Heltec V3 con ArduCam SPI, identificado como **COM9** en el monitor serial.
- Receptor: segundo Heltec V3, identificado como **COM11**.
- Frecuencia de radio: **915 MHz**; modo LoRa punto a punto, 32 bytes por paquete de prueba.
- Configuración empleada: BW = 0, SF = 7, CR = 1, preámbulo = 8; monitor serial **115200 baudios**.
- Se mantuvo la configuración de color/auto white balance de la cámara.

### Problema de integración y corrección

**Síntoma inicial:** El emisor imprimía «Enviando primeros 32 bytes...» pero no «Paquete transmitido por LoRa»; el receptor permanecía esperando. La cámara sí tomaba fotografías, pero la radio no completaba el envío.

**Cambio aplicado:** después de capturar y leer los primeros 32 bytes desde ArduCam, se liberó el SPI configurado para la cámara **antes de iniciar la radio Heltec**, usando:

```cpp
digitalWrite(CAM_CS, HIGH);
SPI.end();
delay(200);
Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);
```

Esto es compatible con la hipótesis de interferencia de configuración SPI entre cámara y radio; no se realizó una medición independiente del bus para demostrar la causa física exacta.

### Evidencia observada en monitores seriales

**Emisor (COM9):**

```text
=== LoRaVision: Emisor ===
Capturando fotografia...
Tamano JPEG: 10280
Primeros bytes capturados:
FFD8FFE000104A46494660000101000000000000FFDB00430004030304030304
SPI de camara liberado
Inicializando radio LoRa...
Enviando primeros 32 bytes...
Paquete transmitido por LoRa
```

**Receptor (COM11):**

```text
=== LoRaVision: Receptor ===
Esperando paquete JPEG...

=== PAQUETE RECIBIDO ===
Bytes: 32
RSSI: -15
HEX: FFD8FFE000104A46494660000101000000000000FFDB00430004030304030304
========================
```

Los **32 bytes del receptor coinciden** con los del emisor y empiezan por `FFD8` (inicio de JPEG). El valor RSSI −15 dBm procede de una prueba de corta distancia; no representa cobertura ni alcance máximo.

**Conclusión:** se demostró la recepción inalámbrica de un **fragmento inicial real de JPEG** entre dos Heltec V3. No se ha transmitido una fotografía completa ni verificado la reconstrucción desde el receptor.

**Código experimental:** `firmware/prueba_lora_jpeg/emisor_lora.ino` y `receptor_lora.ino`; documentan la lógica de la prueba. Antes de repetir, comprobar la compatibilidad con la versión instalada de las bibliotecas y conservar el firmware de captura serial que ya funciona.

### Incidencia de compilación

Durante las pruebas hubo fallos intermitentes de ejecución `cc1plus.exe` / `Access is denied` con Arduino IDE y CLI; el mismo compilador funcionó directamente en PowerShell. Se cambió el proveedor antivirus principal de Reason Cybersecurity a Microsoft Defender. Posteriormente se obtuvo compilación y carga, **sin demostrar que el cambio de antivirus fuese la causa de la solución**.

## 2026-11-16 — Primer JPEG completo transmitido y reconstruido automáticamente

**Resultado reportado por el desarrollador:** la fotografía se transmite por LoRa entre dos Heltec V3 y se reconstruye directamente en Python, guardándose en un archivo JPEG. Ya no es necesario copiar el volcado hexadecimal manualmente.

**Arquitectura:** ArduCam Mega 5MP-AF → Heltec V3 emisor (captura a RAM, liberación SPI) → LoRa P2P 915 MHz, bloques de 64 bytes con secuencia, ACK y reintentos → Heltec V3 receptor → USB serial → Python + PySerial → `fotoN.jpg`.

Se publicó una versión separada del firmware de emisor, receptor y el script de Python en el repositorio. El código de la prueba previa de 32 bytes se mantuvo.

**Nivel de evidencia:** existe confirmación directa del resultado funcional por parte del desarrollador, pero no se entregaron en esta etapa archivos JPEG resultantes, logs completos, tiempos medidos ni tasas de pérdida. La transferencia completa funciona en las condiciones ensayadas; faltan pruebas de desempeño y robustez.

**Documentos:** [Transferencia completa](transmision_completa.md), [incidencia del compilador y antivirus](problemas_compilacion_antivirus.md).

## Próximo hito

Repetir la transferencia a distancias controladas, medir latencia y reintentos, comparar integridad de origen/destino y mejorar recuperación de transmisiones interrumpidas.
