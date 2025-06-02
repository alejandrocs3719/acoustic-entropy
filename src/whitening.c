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


#define _POSIX_C_SOURCE 200809L  // Necesario para strdup en algunos entornos

#include "whitening.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


int metodo_whitening_valido(const char *metodos) {
    char copia[128];
    strncpy(copia, metodos, sizeof(copia));
    copia[sizeof(copia) - 1] = '\0';

    char *token = strtok(copia, ",");
    while (token) {
        if (strcmp(token, "sha256") != 0 &&
            strcmp(token, "blake2b") != 0 &&
            strcmp(token, "ninguno") != 0 &&
            strcmp(token, "xor") != 0) {
            return 0; // método no válido
        }
        token = strtok(NULL, ",");
    }
    return 1;
}




// Función auxiliar XOR valor fijo + rotación simple
void xor_rotate_whitening(uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        data[i] = ((data[i] ^ 0x5A) << 1) | ((data[i] ^ 0x5A) >> 7); // Aplicamos el XOR con la sal de entropia, luego rotación circular hacia la izquierda.
    }
}

int aplicar_whitening(const uint32_t *input, size_t num_muestras, uint8_t **output, size_t *output_len, const char *metodos) {
    size_t input_len_bytes = num_muestras * sizeof(uint32_t);
    uint8_t *intermedio = (uint8_t *)malloc(input_len_bytes);
    if (!intermedio) return -1;
    memcpy(intermedio, input, input_len_bytes);
    size_t len_actual = input_len_bytes;

    char *metodos_copia = strdup(metodos);
    char *token = strtok(metodos_copia, ",");

    while (token) { // Mientras nos sigan quedando tokens
        if (strcmp(token, "sha256") == 0) {
            // Creacion de array nuevo de resultado
            size_t block_size = 32; // 32 bytes de entrada al programa
            size_t num_blocks = len_actual / block_size; // Solo número de bloques completos
            if (num_blocks == 0) return -1;

            size_t total_out = num_blocks * SHA256_DIGEST_LENGTH;
            uint8_t *nuevo = malloc(total_out);
            if (!nuevo) return -1;

            for (size_t i = 0; i < num_blocks; i++) {
                SHA256(intermedio + i * block_size, block_size, nuevo + i * SHA256_DIGEST_LENGTH); // SHA256(puntero de inicio bloque i, tamaño del bloque a procesar, puntero output);
            }

            free(intermedio);
            intermedio = nuevo;
            len_actual = total_out;

        } else if (strcmp(token, "blake2b") == 0) {
            size_t block_size = 64;
            size_t hash_len = 64;
            size_t num_blocks = len_actual / block_size; // Solo número de bloques completos
            if (num_blocks == 0) return -1;

            size_t total_out = num_blocks * hash_len;
            uint8_t *nuevo = malloc(total_out);
            if (!nuevo) return -1;

            for (size_t i = 0; i < num_blocks; i++) { // Hasheamos cada bloque, concatenamos en nuevo
                EVP_MD_CTX *ctx = EVP_MD_CTX_new(); // nuevo contexto de hashing
                EVP_DigestInit_ex(ctx, EVP_blake2b512(), NULL); // usamos blake2b-512
                EVP_DigestUpdate(ctx, intermedio + i * block_size, block_size); // añadimos al contexto el bloque de entrada correspondiente
                EVP_DigestFinal_ex(ctx, nuevo + i * hash_len, NULL); // aplicamos el hash y lo ponemos donde queremos en el contexto de salida.
                EVP_MD_CTX_free(ctx);
            }

            free(intermedio);
            intermedio = nuevo;
            len_actual = total_out;

        } else if (strcmp(token, "xor") == 0) {
            xor_rotate_whitening(intermedio, len_actual);
        }

        token = strtok(NULL, ","); // Siguiente token
    }

    free(metodos_copia);
    *output = intermedio;
    *output_len = len_actual;
    return 0;
}
