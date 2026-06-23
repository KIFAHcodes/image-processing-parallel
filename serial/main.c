#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Deklarasi fungsi dari file lain
void gaussian_blur(unsigned char* input, unsigned char* output, int width, int height, int channels);
void sobel_filter(unsigned char* input, unsigned char* output, int width, int height, int channels);

int main() {
    // Load gambar
    int width, height, channels;
    unsigned char* img = stbi_load("../images/input.jpg", &width, &height, &channels, 0);
    
    if (img == NULL) {
        printf("Error: Gagal load gambar!\n");
        return 1;
    }
    
    printf("Gambar berhasil di-load!\n");
    printf("Ukuran: %d x %d, Channel: %d\n", width, height, channels);
    
    // Alokasi memori untuk output
    unsigned char* blurred = (unsigned char*)malloc(width * height * channels);
    unsigned char* edges   = (unsigned char*)malloc(width * height * channels);
    
    // ===== MULAI TIMER =====
    clock_t start = clock();
    
    // Step 1: Gaussian Blur
    gaussian_blur(img, blurred, width, height, channels);
    
    // Step 2: Sobel Filter
    sobel_filter(blurred, edges, width, height, channels);
    
    // ===== STOP TIMER =====
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Waktu eksekusi serial: %.4f detik\n", time_taken);
    
    // Simpan hasil
    stbi_write_jpg("../images/output/output_serial.jpg", width, height, channels, edges, 100);
    printf("Output disimpan ke images/output/output_serial.jpg\n");
    
    // Bebaskan memori
    stbi_image_free(img);
    free(blurred);
    free(edges);
    
    return 0;
}