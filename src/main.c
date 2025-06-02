// AcousticEntropy - A Linux tool to inject audio-based randomness into the entropy pool.
// Copyright (C) 2025  alejandrocs3719

// This program is free software: you can redistribute it and/or modify  
// it under the terms of the GNU General Public License as published by  
// the Free Software Foundation, either version 3 of the License, or  
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,  
// but WITHOUT ANY WARRANTY; without even the implied warranty of  
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License  
// along with this program.  If not, see <https://www.gnu.org/licenses/>.                    


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <glob.h>
#include "recolector.h"
#include "shannon.h"
#include "injector.h"
#include "whitening.h"

#define NUM_MUESTRAS 400000
#define MAX_FILES 100

int main(int argc, char *argv[]) {
    // Parametros por defecto.
    const char *whitening_methods = "blake2b"; // default
    int usar_wavs_existentes = 0;
    char *wav_files[MAX_FILES];
    int wav_count = 0;

    // Procesado de los argumentos del programa
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            whitening_methods = argv[++i];
        } else if (strcmp(argv[i], "-e") == 0) { // Si encontramos la -e
            usar_wavs_existentes = 1;
            i++; // saltamos el "-e"
            while (i < argc && argv[i][0] != '-') { // Vamos contando argumentos a ver que hay, evitamos leer flags si vienen después
                if (wav_count >= MAX_FILES) break;
                wav_files[wav_count++] = strdup(argv[i]);
                i++;
            }
            i--; // volvemos uno atrás para que el for no se lo salte
        }

    }

 
    // Módulo recolector
    uint32_t buffer[NUM_MUESTRAS];
    // ------ Caso de usar ficheros existentes
    if (usar_wavs_existentes) {
        if (wav_count == 0) {
            // Sin argumentos adicionales, usar datos/*.wav
            glob_t glob_result;
            if (glob("datos/*.wav", 0, NULL, &glob_result) == 0) {
                for (size_t j = 0; j < glob_result.gl_pathc && wav_count < MAX_FILES; j++) {
                    wav_files[wav_count++] = strdup(glob_result.gl_pathv[j]);
                }
                globfree(&glob_result);
            }
        }

        // Mezclamos los archivos WAV con XOR
        if (mezclar_archivos_wav_xor(wav_files, wav_count, buffer, NUM_MUESTRAS) != 0) {
            fprintf(stderr, "Error al mezclar los archivos WAV existentes.\n");
            return 1;
        }

        printf("Recolectadas y mezcladas %d muestras desde archivos WAV existentes.\n", NUM_MUESTRAS);

    } else {
        printf("Recolectando %d muestras de tiempo...\n", NUM_MUESTRAS);

        wav_count = NUM_AUDIO_GRABACIONES;

        // Grabamos múltiples audios usando el módulo recolector
        if (grabar_audio(wav_files, NUM_MUESTRAS) != 0) {
            fprintf(stderr, "Error al grabar audios.\n");
            return 1;
        }

        // Mezclamos los audios grabados con XOR
        if (mezclar_archivos_wav_xor(wav_files, wav_count, buffer, NUM_MUESTRAS) != 0) {
            fprintf(stderr, "Error al mezclar grabaciones.\n");
            return 1;
        }

        printf("Recolectadas y mezcladas %d muestras desde grabación directa.\n", NUM_MUESTRAS);
    }


    // Módulo whitening
    uint8_t *blanco = NULL;
    size_t blanco_len = 0;

    if (!metodo_whitening_valido(whitening_methods)) {
        fprintf(stderr, "Método(s) de whitening inválido(s): %s\n", whitening_methods);
        return 1;    
    }

    printf("Aplicando whitening %s por bloques...\n", whitening_methods);
    if (aplicar_whitening(buffer, NUM_MUESTRAS, &blanco, &blanco_len, whitening_methods) != 0) {
        fprintf(stderr, "Error al aplicar whitening.\n");
        return 1;
    }

    // Preparamos nombre de archivo según el método de whitening
    char filename_white_parcial[128];
    strncpy(filename_white_parcial, whitening_methods, sizeof(filename_white_parcial));
    filename_white_parcial[sizeof(filename_white_parcial) - 1] = '\0'; // Por si acaso el string midiera más siempre tener el string acabado
    for (int i = 0; filename_white_parcial[i]; i++) {
        if (filename_white_parcial[i] == ',') filename_white_parcial[i] = '-'; // reemplazamos comas por guiones que es más elegante
    }

    char filename_white[256];
    snprintf(filename_white, sizeof(filename_white), "datos/datos_%s.bin", filename_white_parcial);

    FILE *f_white = fopen(filename_white, "wb");
    if (f_white) {
        fwrite(blanco, 1, blanco_len, f_white);
        fclose(f_white);
        printf("Datos blanqueados guardados en %s\n", filename_white);
    }





    // Módulos de estimación y de inyección
    size_t num_blanco_muestras = blanco_len / sizeof(uint32_t);
    double entropia_por_muestra = calcular_entropia_shannon((uint32_t *)blanco, num_blanco_muestras);
    double entropia_total = entropia_por_muestra * num_blanco_muestras;

    printf("Entropía estimada: %.4f bits por muestra x %zu = %.0f bits totales\n",
           entropia_por_muestra, num_blanco_muestras, entropia_total);

    printf("Inyectando entropía al sistema...\n");
    if (inyectar_entropia((uint32_t *)blanco, num_blanco_muestras, (int)entropia_total) != 0) {
        fprintf(stderr, "Error al inyectar entropía.\n");
    } else {
        printf("Inyección completada.\n");
    }

    free(blanco);
    for (int i = 0; i < wav_count; i++) free(wav_files[i]);
    return 0;
}
