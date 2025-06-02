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


#ifndef RECOLECTOR_H
#define RECOLECTOR_H
#define NUM_AUDIO_GRABACIONES 5

#include <stdint.h>

int grabar_audio(char **filenames, int num_muestras);
int mezclar_archivos_wav_xor(char **filenames, int num_files, uint32_t *buffer, int num_muestras);

#endif