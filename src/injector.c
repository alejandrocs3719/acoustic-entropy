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



#include "injector.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/random.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int inyectar_entropia(uint32_t *datos, int num_muestras, int entropy_bits) {
    int fd = open("/dev/random", O_WRONLY);
    if (fd < 0) {
        perror("No se pudo abrir /dev/random");
        return -1;
    }

    int data_size = num_muestras * sizeof(uint32_t);

    struct rand_pool_info *pool = malloc(sizeof(struct rand_pool_info) + data_size);
    if (!pool) {
        perror("malloc");
        close(fd);
        return -1;
    }

    pool->entropy_count = entropy_bits; // bits de entropía real estimada
    pool->buf_size = data_size;
    memcpy(pool->buf, datos, data_size);

    int res = ioctl(fd, RNDADDENTROPY, pool);
    if (res < 0) {
        perror("ioctl RNDADDENTROPY falló");
    } else {
        printf("Entropía inyectada correctamente (%d bits)\n", entropy_bits);
    }

    free(pool);
    close(fd);
    return res;
}
