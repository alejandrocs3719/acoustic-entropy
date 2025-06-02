# AcousticEntropy - A Linux tool to inject audio-based randomness into the entropy pool.
# Copyright (C) 2025  alejandrocs3719

# This program is free software: you can redistribute it and/or modify  
# it under the terms of the GNU General Public License as published by  
# the Free Software Foundation, either version 3 of the License, or  
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,  
# but WITHOUT ANY WARRANTY; without even the implied warranty of  
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License  
# along with this program.  If not, see <https://www.gnu.org/licenses/>.                    


import os
import subprocess
import pandas as pd
from pathlib import Path
import re
import glob

# Configuración general
base_dir = Path("./pruebas_previas")
datos_dir = Path("./datos")
output_excel = Path("analisis_resultados_entropia.xlsx")

metodos_whitening = ["xor", "sha256", "blake2b", "sha256,xor", "blake2b,xor","xor,sha256","xor,blake2b", "ninguno"]
columnas = [
    "Carpeta de prueba", "Micrófono", "Método Whitening", "Entropía por muestra (Shannon)",
    "Entropía (ent)", "Chi Square (ent)", "Reducción compresión óptima % (ent)", "Media aritmética (ent)",
    "Monte Carlo (ent)", "Error Monte Carlo (ent)", "Correlación (ent)",
    "FIPS éxitos (rngtest)", "FIPS fallos (rngtest)",
    "Monobit (rngtest)", "Poker (rngtest)", "Runs (rngtest)", "Long run (rngtest)", "Continuous run (rngtest)",
    "Dieharder PASSED", "Dieharder FAILED", "Dieharder WEAK"
]

resultados = []

# Recorremos todas las carpetas de pruebas
for carpeta in sorted(base_dir.iterdir()):
    if not carpeta.is_dir():
        continue

    mic = "malmic" if "malmic" in carpeta.name else "promic" if "promic" in carpeta.name else "desconocido"

    for metodo in metodos_whitening:
        # Expandimos los .wav de la carpeta
        wav_files = sorted(glob.glob(str(carpeta / "*.wav")))
        if not wav_files:
            continue

        # Ejecutamos el programa principal con todos los .wav
        proc = subprocess.run(
            ["sudo", "./tfg_entropy", "-e", *wav_files, "-w", metodo],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        # Buscamos entropía de Shannon en salida
        entropia_shannon = None
        shannon_match = re.search(r"Entrop[ií]a estimada:\s+([\d.]+)\s+bits por muestra", proc.stdout)
        if shannon_match:
            entropia_shannon = float(shannon_match.group(1))
            print(f"Entropía por muestra encontrada: {entropia_shannon}") # Como el programa tiene mucho tiempo de ejecución, dejamos esto a modo de feedback visual por consola.
        else:
            print("No se encontró entropía por muestra en stdout.")



        # Localizamos y movemos el archivo generado
        metodo_archivo = metodo.replace(",", "-")
        archivo_generado = datos_dir / f"datos_{metodo_archivo}.bin"
        archivo_destino = carpeta / f"datos_{metodo_archivo}.bin"

        if not archivo_generado.exists():
            continue  # algo falló en la ejecución

        archivo_generado.rename(archivo_destino)

        # Ejecutamos 'ent'
        proc_ent = subprocess.run(["ent", str(archivo_destino)], capture_output=True, text=True)
        ent_out = proc_ent.stdout
        ent_out_clean = " ".join(ent_out.splitlines())  # Fusionar para facilitar parsing multilínea

        # Inicializamos variables
        ent_entropy = chi_square = compression = mean = montecarlo_pi = montecarlo_error = corr = None

        # Extraemos valores usando expresiones regulares
        match_entropy = re.search(r"Entropy = ([\d.]+)", ent_out)
        match_chi = re.search(r"Chi square.*?is\s+([\d.]+)", ent_out_clean)
        match_compression = re.search(r"reduce the size.*?by\s+(\d+)\s+percent", ent_out_clean)
        match_mean = re.search(r"Arithmetic mean value of data bytes is ([\d.]+)", ent_out)
        match_montecarlo = re.search(r"Monte Carlo value for Pi is ([\d.]+) \(error ([\d.]+)", ent_out)
        match_corr = re.search(r"Serial correlation coefficient is ([\d\-.]+)", ent_out)

        if match_entropy:
            ent_entropy = float(match_entropy.group(1))
        if match_chi:
            chi_square = float(match_chi.group(1))
        if match_compression:
            compression = int(match_compression.group(1))
        if match_mean:
            mean = float(match_mean.group(1))
        if match_montecarlo:
            montecarlo_pi = float(match_montecarlo.group(1))
            montecarlo_error = float(match_montecarlo.group(2))
        if match_corr:
            corr = float(match_corr.group(1))



        # rng test

        # Ejecutamos 'cat archivo | rngtest' como shell command
        proc_rng = subprocess.run(
            f"cat '{archivo_destino}' | rngtest",
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        # Capturamos salida desde stderr (donde rngtest imprime todo)
        rng_out = proc_rng.stderr


        # Inicializamos valores por si no aparecen
        fips_ok = fips_fail = monobit = poker = runs = longrun = cont_run = None

        # Parseo línea a línea
        for line in rng_out.splitlines():
            if "successes:" in line:
                match = re.search(r"successes:\s+(\d+)", line)
                if match:
                    fips_ok = int(match.group(1))
            elif "failures:" in line:
                match = re.search(r"failures:\s+(\d+)", line)
                if match:
                    fips_fail = int(match.group(1))
            elif "Monobit" in line:
                monobit = int(re.search(r"Monobit: (\d+)", line).group(1))
            elif "Poker" in line:
                poker = int(re.search(r"Poker: (\d+)", line).group(1))
            elif "Runs" in line and "Continuous run" not in line:
                runs = int(re.search(r"Runs: (\d+)", line).group(1))
            elif "Long run" in line:
                longrun = int(re.search(r"Long run: (\d+)", line).group(1))
            elif "Continuous run" in line:
                cont_run = int(re.search(r"Continuous run: (\d+)", line).group(1))




        # Ejecutamos 'dieharder' optimizado
        proc_die = subprocess.run([
            "dieharder", "-d", "0,1,2,3,10,100", "-t", "10", "-g", "201", "-f", str(archivo_destino)
        ], capture_output=True, text=True)

        die_out = proc_die.stdout

        passed = len(re.findall(r"\s+PASSED", die_out))
        weak = len(re.findall(r"\s+WEAK", die_out))
        failed = len(re.findall(r"\s+FAILED", die_out))

        resultados.append([
            carpeta.name, mic, metodo, entropia_shannon,
            ent_entropy, chi_square, compression, mean, montecarlo_pi, montecarlo_error, corr,
            fips_ok, fips_fail, monobit, poker, runs, longrun, cont_run,
            passed, failed, weak
        ])


# Guardamos en Excel
df = pd.DataFrame(resultados, columns=columnas)
df.to_excel(output_excel, index=False)
print(f"[OK] Análisis completado. Resultados guardados en: {output_excel}")
