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


#ifndef WHITENING_H
#define WHITENING_H

#include <stdint.h>
#include <stdlib.h>

int aplicar_whitening(const uint32_t *input, size_t num_muestras, uint8_t **output, size_t *output_len, const char *metodos);
int metodo_whitening_valido(const char *metodos);

#endif
