#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../serial/stb_image.h"
#include "../serial/stb_image_write.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <omp.h>

// Kernel Gaussian 5x5
static float gaussian_kernel[5][5] = {
    {1/273.0f,  4/273.0f,  7/273.0f,  4/273.0f, 1/273.0f},
    {4/273.0f, 16/273.0f, 26/273.0f, 16/273.0f, 4/273.0f},
    {7/273.0f, 26/273.0f, 41/273.0f, 26/273.0f, 7/273.0f},
    {4/273.0f, 16/273.0f, 26/273.0f, 16/273.0f, 4/273.0f},
    {1/273.0f,  4/273.0f,  7/273.0f,  4/273.0f, 1/273.0f}
};

// Sobel Filter dengan OpenMP
void sobel_filter_omp(unsigned char* input, unsigned char* output, int width, int height, int channels) {
    int Gx[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    int Gy[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};

    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float sumX = 0.0f, sumY = 0.0f;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    if (ny < 0) ny = 0;
                    if (ny >= height) ny = height - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= width) nx = width - 1;

                    float pixel = input[(ny * width + nx) * channels];
                    sumX += pixel * Gx[ky + 1][kx + 1];
                    sumY += pixel * Gy[ky + 1][kx + 1];
                }
            }

            float magnitude = sqrtf(sumX * sumX + sumY * sumY);
            if (magnitude > 255.0f) magnitude = 255.0f;
            if (magnitude < 0.0f)   magnitude = 0.0f;

            for (int c = 0; c < channels; c++) {
                output[(y * width + x) * channels + c] = (unsigned char)magnitude;
            }
        }
    }
}

// Gaussian Blur dengan OpenMP
void gaussian_blur_omp(unsigned char* input, unsigned char* output, int width, int height, int channels) {
    int radius = 2;

    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < channels; c++) {
                float sum = 0.0f;

                for (int ky = -radius; ky <= radius; ky++) {
                    for (int kx = -radius; kx <= radius; kx++) {
                        int ny = y + ky;
                        int nx = x + kx;

                        if (ny < 0) ny = 0;
                        if (ny >= height) ny = height - 1;
                        if (nx < 0) nx = 0;
                        if (nx >= width) nx = width - 1;

                        float pixel = input[(ny * width + nx) * channels + c];
                        sum += pixel * gaussian_kernel[ky + radius][kx + radius];
                    }
                }

                output[(y * width + x) * channels + c] = (unsigned char)sum;
            }
        }
    }
}

int main() {
    int width, height, channels;
    unsigned char* img = stbi_load("../images/input.jpg", &width, &height, &channels, 0);

    if (img == NULL) {
        printf("Error: Gagal load gambar!\n");
        return 1;
    }

    printf("Gambar berhasil di-load!\n");
    printf("Ukuran: %d x %d, Channel: %d\n", width, height, channels);
    printf("Jumlah thread OpenMP: %d\n", omp_get_max_threads());

    unsigned char* blurred = (unsigned char*)malloc(width * height * channels);
    unsigned char* edges   = (unsigned char*)malloc(width * height * channels);

    // ===== MULAI TIMER =====
    double start = omp_get_wtime();

    // Step 1: Gaussian Blur (OpenMP)
    printf("Menjalankan Gaussian Blur (OpenMP)...\n");
    gaussian_blur_omp(img, blurred, width, height, channels);
    printf("Gaussian Blur selesai!\n");

    // Step 2: Sobel Filter (OpenMP)
    printf("Menjalankan Sobel Filter (OpenMP)...\n");
    sobel_filter_omp(blurred, edges, width, height, channels);
    printf("Sobel Filter selesai!\n");

    // ===== STOP TIMER =====
    double end = omp_get_wtime();
    double time_taken = end - start;

    printf("Waktu eksekusi OpenMP: %.4f detik\n", time_taken);
    printf("Speedup vs Serial: %.2fx\n", 0.1910 / time_taken);

    // Simpan hasil
    stbi_write_jpg("../images/output/output_openmp.jpg", width, height, channels, edges, 100);
    printf("Output disimpan ke images/output/output_openmp.jpg\n");

    stbi_image_free(img);
    free(blurred);
    free(edges);

    return 0;
}