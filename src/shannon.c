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


#include "shannon.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HISTOGRAM_SIZE ((uint64_t)1 << 32)  // 2^32 posibles valores

double calcular_entropia_shannon(uint32_t *buffer, int num_muestras) {
    // Reservamos dinámicamente el histograma para no agotar la pila
    uint32_t *histograma = calloc(HISTOGRAM_SIZE, sizeof(uint32_t));
    if (!histograma) {
        perror("No se pudo asignar memoria para el histograma de 32 bits");
        return -1;
    }

    // Llenamos el histograma contando las ocurrencias exactas de cada valor de 32 bits
    for (int i = 0; i < num_muestras; i++) {
        histograma[buffer[i]]++;
    }

    // Calculamos la entropía
    double entropia = 0.0;
    for (uint64_t i = 0; i < HISTOGRAM_SIZE; i++) {
        if (histograma[i] > 0) {
            double p = (double)histograma[i] / num_muestras;
            entropia -= p * log2(p);
        }
    }

    free(histograma);
    return entropia;  // bits por muestra (máx 32)
}
