#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void sobel_filter(unsigned char* input, unsigned char* output, int width, int height, int channels) {
    printf("Menjalankan Sobel Filter (Serial)...\n");

    // Kernel Sobel arah X (deteksi tepi horizontal)
    int Gx[3][3] = {
        {-1,  0,  1},
        {-2,  0,  2},
        {-1,  0,  1}
    };

    // Kernel Sobel arah Y (deteksi tepi vertikal)
    int Gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            float sumX = 0.0f;
            float sumY = 0.0f;

            // Konvolusi dengan kernel Sobel
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {

                    // Clamp agar tidak keluar batas gambar
                    int ny = y + ky;
                    int nx = x + kx;

                    if (ny < 0) ny = 0;
                    if (ny >= height) ny = height - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= width) nx = width - 1;

                    // Ambil nilai grayscale dari channel pertama (R)
                    float pixel = input[(ny * width + nx) * channels];

                    sumX += pixel * Gx[ky + 1][kx + 1];
                    sumY += pixel * Gy[ky + 1][kx + 1];
                }
            }

            // Hitung magnitude gradient
            float magnitude = sqrtf(sumX * sumX + sumY * sumY);

            // Clamp nilai 0-255
            if (magnitude > 255.0f) magnitude = 255.0f;
            if (magnitude < 0.0f)   magnitude = 0.0f;

            // Simpan ke semua channel (R, G, B sama semua → grayscale)
            for (int c = 0; c < channels; c++) {
                output[(y * width + x) * channels + c] = (unsigned char)magnitude;
            }
        }
    }

    printf("Sobel Filter selesai!\n");
}