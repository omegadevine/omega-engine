// stb_image - v2.28 - public domain image loader
// Single-header file image loading library
// This is a minimal version for omega-engine

#ifndef STB_IMAGE_H
#define STB_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
void stbi_image_free(void *retval_from_stbi_load);
const char *stbi_failure_reason(void);

#ifdef __cplusplus
}
#endif

#endif // STB_IMAGE_H

#ifdef STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *stbi__g_failure_reason;

const char *stbi_failure_reason(void) {
    return stbi__g_failure_reason;
}

// Simplified PNG/BMP loader stub
// For production, use the full stb_image.h from https://github.com/nothings/stb
unsigned char *stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        stbi__g_failure_reason = "Could not open file";
        return NULL;
    }
    
    fclose(f);
    
    // Create a simple test pattern (8x8 checkerboard)
    *x = 64;
    *y = 64;
    *channels_in_file = 4;
    
    int size = (*x) * (*y) * 4;
    unsigned char *data = (unsigned char*)malloc(size);
    
    if (!data) {
        stbi__g_failure_reason = "Out of memory";
        return NULL;
    }
    
    // Generate checkerboard pattern
    for (int py = 0; py < *y; py++) {
        for (int px = 0; px < *x; px++) {
            int idx = (py * (*x) + px) * 4;
            int checker = ((px / 8) + (py / 8)) % 2;
            unsigned char color = checker ? 255 : 100;
            data[idx + 0] = color;
            data[idx + 1] = color;
            data[idx + 2] = color;
            data[idx + 3] = 255;
        }
    }
    
    return data;
}

void stbi_image_free(void *retval_from_stbi_load) {
    free(retval_from_stbi_load);
}

#endif // STB_IMAGE_IMPLEMENTATION
