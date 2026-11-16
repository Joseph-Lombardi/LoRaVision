"""Reconstruye la ultima captura JPEG completa del volcado serial ArduCam.

Uso:
    python reconstruir.py
    python reconstruir.py ruta/a/captura.txt

Guarda foto1.jpg, foto2.jpg, etc., sin sobrescribir resultados previos.
"""
from pathlib import Path
import re
import sys

def extraer_ultima_captura(texto: str) -> bytes:
    # Solo aceptamos bloques completos; evitamos capturas parciales.
    bloques = re.findall(r"INICIO_JPEG\s*(.*?)\s*FIN_JPEG", texto, flags=re.DOTALL)
    if not bloques:
        raise ValueError("No se encontro una captura completa INICIO_JPEG / FIN_JPEG")

    # El contenido esperado son digitos hexadecimales y espacios.
    hexadecimal = re.sub(r"\s+", "", bloques[-1])
    if not re.fullmatch(r"[0-9a-fA-F]+", hexadecimal or ""):
        raise ValueError("La ultima captura contiene caracteres no hexadecimales")

    datos = bytes.fromhex(hexadecimal)
    inicio = datos.find(b"\xff\xd8")
    if inicio < 0:
        raise ValueError("Falta el inicio JPEG FFD8")

    fin = datos.find(b"\xff\xd9", inicio + 2)
    if fin < 0:
        raise ValueError("Falta el final JPEG FFD9")

    imagen = datos[inicio:fin + 2]
    print(f"Bytes recibidos: {len(datos)}")
    print(f"Bytes del JPEG: {len(imagen)}")
    print(f"Bytes descartados: {len(datos) - len(imagen)}")
    return imagen

def siguiente_nombre(carpeta: Path) -> Path:
    numero = 1
    while (carpeta / f"foto{numero}.jpg").exists():
        numero += 1
    return carpeta / f"foto{numero}.jpg"

def main() -> None:
    archivo = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("captura1.txt")
    texto = archivo.read_text(encoding="utf-8", errors="replace")
    imagen = extraer_ultima_captura(texto)
    salida = siguiente_nombre(Path.cwd())
    # 'xb' protege el archivo ante una eventual creacion simultanea.
    with salida.open("xb") as f:
        f.write(imagen)
    print(f"Fotografia guardada: {salida}")

if __name__ == "__main__":
    main()
