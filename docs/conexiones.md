# Conexiones: ArduCam Mega 5MP-AF → Heltec V3

| Señal ArduCam SPI | GPIO Heltec WiFi LoRa 32 V3 |
| --- | ---: |
| SCK | 6 |
| MISO | 5 |
| MOSI | 4 |
| CS | 7 |

Conectar además alimentación y GND según especificaciones de los módulos; verificar el voltaje permitido de la cámara y el cableado antes de energizar.

El firmware inicializa SPI con `SPI.begin(6, 5, 4, 7)`. Durante las pruebas se corrigió la función `arducamSpiBegin()` de la biblioteca Arducam Mega 3.0.0 para evitar `return SPI.begin();` dentro de una función `void`.

Placa de desarrollo: **Heltec WiFi LoRa 32(V3)**. Paquete de placas usado tras resolver un fallo del compilador: **esp32 by Espressif Systems 3.2.1**.

**Fase LoRa futura:** frecuencia de trabajo prevista: **915 MHz**, con otro Heltec V3 como receptor. Antes de transmitir fotos se necesita implementar fragmentación, índice de paquetes, reintentos y verificación de integridad.
