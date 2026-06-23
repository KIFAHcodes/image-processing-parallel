#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Kernel Gaussian 5x5
static float gaussian_kernel[5][5] = {
    {1/273.0f,  4/273.0f,  7/273.0f,  4/273.0f, 1/273.0f},
    {4/273.0f, 16/273.0f, 26/273.0f, 16/273.0f, 4/273.0f},
    {7/273.0f, 26/273.0f, 41/273.0f, 26/273.0f, 7/273.0f},
    {4/273.0f, 16/273.0f, 26/273.0f, 16/273.0f, 4/273.0f},
    {1/273.0f,  4/273.0f,  7/273.0f,  4/273.0f, 1/273.0f}
};

void gaussian_blur(unsigned char* input, unsigned char* output, int width, int height, int channels) {
    printf("Menjalankan Gaussian Blur (Serial)...\n");

    int radius = 2; // radius kernel 5x5

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < channels; c++) {

                float sum = 0.0f;

                // Konvolusi dengan kernel gaussian
                for (int ky = -radius; ky <= radius; ky++) {
                    for (int kx = -radius; kx <= radius; kx++) {

                        // Pastikan tidak keluar batas gambar (clamp)
                        int ny = y + ky;
                        int nx = x + kx;

                        if (ny < 0) ny = 0;
                        if (ny >= height) ny = height - 1;
                        if (nx < 0) nx = 0;
                        if (nx >= width) nx = width - 1;

                        // Ambil nilai pixel
                        float pixel = input[(ny * width + nx) * channels + c];

                        // Kalikan dengan kernel
                        sum += pixel * gaussian_kernel[ky + radius][kx + radius];
                    }
                }

                // Simpan hasil ke output
                output[(y * width + x) * channels + c] = (unsigned char)sum;
            }
        }
    }

    printf("Gaussian Blur selesai!\n");
}