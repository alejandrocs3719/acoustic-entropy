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


#define _POSIX_C_SOURCE 200809L


#include "recolector.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define CHANNELS 1
#define HEADER_SIZE 44
#define FILENAME_SIZE 64
#define AUDIO_PATH_TEMPLATE "datos/audio_%d.wav"

int comparar_nombres(const void *a, const void *b) {
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcmp(fa, fb);
}

int mezclar_archivos_wav_xor(char **filenames, int num_files, uint32_t *buffer, int num_muestras) {
    qsort(filenames, num_files, sizeof(char *), comparar_nombres); // nos aseguramos que los miren en el mismo orden para la reproducibilidad.
    
    for (int i = 0; i < num_files; i++) {
        printf("Usando WAV: %s\n", filenames[i]);
    }


    // Inicializamos buffer con ceros
    for (int i = 0; i < num_muestras; i++) {
        buffer[i] = 0;
    }

    for (int i = 0; i < num_files; i++) {
        FILE *f = fopen(filenames[i], "rb"); // read binary
        if (!f) {
            fprintf(stderr, "No se pudo abrir: %s\n", filenames[i]);
            return -1;
        }

        fseek(f, 44, SEEK_SET); // Movemos el puntero para saltarnos la cabecera de los .wav

        for (int j = 0; j < num_muestras; j++) {
            uint32_t muestra = 0;

            for (int k = 0; k < 4; k++) { // Cada muestra pedida va a ser de 4 bytes como uint32
                int byte = fgetc(f); // Leemos el byte (fgetc avanza puntero solo)
                if (byte == EOF) {
                    fprintf(stderr, "EOF prematuro en %s\n", filenames[i]);
                    fclose(f);
                    return -1;
                }
                muestra |= ((uint32_t)byte << (8 * k)); // Left shift << del número de bytes que corresponde. Los añadimos con un or bit a bit
            }

            buffer[j] ^= muestra; // XOR con lo anterior
        }

        fclose(f);
    }

    return 0;
}


int grabar_audio(char **filenames, int num_muestras) {
    int bytes_total = num_muestras * sizeof(uint32_t); // 2 bytes por muestra por el numero de muestras me da los bytes totales
    int bytes_por_segundo = SAMPLE_RATE * CHANNELS * (BITS_PER_SAMPLE / 8); // como tengo 2 bytes por muestra multiplicando por la frecuencia de muestreo tengo bytes por segundo.
    int segundos = (bytes_total / bytes_por_segundo) + 1; // sumamos uno por si trunca los decimales

    // Grabamos múltiples audios
    for (int i = 0; i < NUM_AUDIO_GRABACIONES; i++) {
        filenames[i] = malloc(FILENAME_SIZE); // Reservamos 64 bytes
        snprintf(filenames[i], FILENAME_SIZE, AUDIO_PATH_TEMPLATE, i); // Montamos el string con el filename que toca

        char comando[256];
        snprintf(comando, sizeof(comando),
                 "arecord -D plughw:1,0 -f S32_LE -r %d -c1 -d %d -q %s",
                 SAMPLE_RATE, segundos, filenames[i]);

        int status = system(comando); // Ejecutamos el comando de grabación
        if (status != 0) {
            fprintf(stderr, "Error ejecutando arecord para %s\n", filenames[i]);
            return -1;
        }
    }

    return 0; // Ya no mezclamos aquí
}