"""Reconstruccion automatica de fotografias JPEG recibidas por LoRa.

Requiere: python -m pip install pyserial
El receptor Heltec debe estar conectado a PUERTO y su monitor serial
Arduino debe estar cerrado para dejar libre el puerto COM.
"""
import serial
from pathlib import Path

PUERTO = "COM11"
BAUDIOS = 115200

imagen = bytearray()
tamano_esperado = None
secuencia_esperada = 0


def siguiente_foto():
    numero = 1
    while Path(f"foto{numero}.jpg").exists():
        numero += 1
    return Path(f"foto{numero}.jpg")


print("=== LoRaVision: Recepcion automatica ===")
print(f"Escuchando {PUERTO}...")

with serial.Serial(PUERTO, BAUDIOS, timeout=1) as conexion:
    while True:
        linea = conexion.readline().decode(
            "utf-8", errors="ignore"
        ).strip()

        if not linea:
            continue

        if linea.startswith("LV_BEGIN,"):
            try:
                tamano_esperado = int(linea.split(",")[1])
            except ValueError:
                continue

            imagen = bytearray()
            secuencia_esperada = 0
            print(f"\nNueva imagen: {tamano_esperado} bytes")

        elif linea.startswith("LV_DATA,"):
            if tamano_esperado is None:
                continue

            try:
                _, numero, hexadecimal = linea.split(",", 2)
                numero = int(numero)

                if numero != secuencia_esperada:
                    print("Paquete fuera de orden:", numero)
                    continue

                datos = bytes.fromhex(hexadecimal)
            except ValueError:
                continue

            imagen.extend(datos)
            secuencia_esperada += 1
            print(
                f"\rRecibidos: {len(imagen)}/{tamano_esperado}",
                end="", flush=True
            )

        elif linea.startswith("LV_END,"):
            if tamano_esperado is None:
                continue

            if (
                len(imagen) == tamano_esperado
                and imagen.startswith(b"\xFF\xD8")
                and imagen.endswith(b"\xFF\xD9")
            ):
                destino = siguiente_foto()
                destino.write_bytes(imagen)
                print(f"\nFotografia guardada: {destino}")
            else:
                print("\nERROR: fotografia incompleta o invalida")

            tamano_esperado = None
            imagen = bytearray()
