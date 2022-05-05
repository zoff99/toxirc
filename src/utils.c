#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void hex_string_to_bin(const char *hex_string, uint8_t *output) {
    size_t len = strlen(hex_string) / 2;
    size_t i = len;
    if (!output) {
        return;
    }

    const char *pos = hex_string;

    for (i = 0; i < len; ++i, pos += 2) {
        sscanf(pos, "%2hhx", &output[i]);
    }
}

off_t get_file_size(char *file) {
    off_t size = 0;

    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        return size;
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fclose(fp);

    return size;
}

void to_hex(char *out, uint8_t *in, int size) {
    while (size--) {
        if (*in >> 4 < 0xA) {
            *out++ = '0' + (*in >> 4);
        } else {
            *out++ = 'A' + (*in >> 4) - 0xA;
        }

        if ((*in & 0xf) < 0xA) {
            *out++ = '0' + (*in & 0xF);
        } else {
            *out++ = 'A' + (*in & 0xF) - 0xA;
        }
        in++;
    }
}
