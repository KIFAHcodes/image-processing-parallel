#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../serial/stb_image.h"
#include "../serial/stb_image_write.h"

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Hanya master (rank 0) yang load gambar
    int width = 0, height = 0, channels = 0;
    unsigned char* img = NULL;
    unsigned char* result = NULL;

    if (rank == 0) {
        printf("=== Master Node (rank 0) ===\n");
        printf("Total node: %d\n", size);

        // Load gambar
        img = stbi_load("../images/input.jpg", &width, &height, &channels, 0);
        if (img == NULL) {
            printf("Error: Gagal load gambar!\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        printf("Gambar berhasil di-load!\n");
        printf("Ukuran: %d x %d, Channel: %d\n", width, height, channels);

        result = (unsigned char*)malloc(width * height * channels);
    }

    // Broadcast info gambar ke semua worker
    MPI_Bcast(&width,    1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height,   1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Hitung pembagian baris per node
    int rows_per_node = height / size;
    int extra_rows    = height % size;

    // Hitung sendcounts dan displacements
    int* sendcounts = (int*)malloc(size * sizeof(int));
    int* displs     = (int*)malloc(size * sizeof(int));

    int offset = 0;
    for (int i = 0; i < size; i++) {
        int rows = rows_per_node + (i < extra_rows ? 1 : 0);
        sendcounts[i] = rows * width * channels;
        displs[i]     = offset;
        offset       += sendcounts[i];
    }

    // Alokasi buffer lokal tiap node
    int local_rows = rows_per_node + (rank < extra_rows ? 1 : 0);
    unsigned char* local_input  = (unsigned char*)malloc(local_rows * width * channels);
    unsigned char* local_output = (unsigned char*)malloc(local_rows * width * channels);

    // Scatter: bagikan potongan gambar ke tiap node
    MPI_Scatterv(img, sendcounts, displs, MPI_UNSIGNED_CHAR,
                 local_input, local_rows * width * channels,
                 MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    printf("Node %d memproses %d baris\n", rank, local_rows);

    // ===== MULAI TIMER =====
    double start = MPI_Wtime();

    // Proses Sobel Filter di tiap node
    int Gx[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    int Gy[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};

    for (int y = 0; y < local_rows; y++) {
        for (int x = 0; x < width; x++) {
            float sumX = 0.0f, sumY = 0.0f;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int ny = y + ky;
                    int nx = x + kx;

                    if (ny < 0) ny = 0;
                    if (ny >= local_rows) ny = local_rows - 1;
                    if (nx < 0) nx = 0;
                    if (nx >= width) nx = width - 1;

                    float pixel = local_input[(ny * width + nx) * channels];
                    sumX += pixel * Gx[ky + 1][kx + 1];
                    sumY += pixel * Gy[ky + 1][kx + 1];
                }
            }

            float magnitude = sqrtf(sumX * sumX + sumY * sumY);
            if (magnitude > 255.0f) magnitude = 255.0f;
            if (magnitude < 0.0f)   magnitude = 0.0f;

            for (int c = 0; c < channels; c++) {
                local_output[(y * width + x) * channels + c] = (unsigned char)magnitude;
            }
        }
    }

    // ===== STOP TIMER =====
    double end = MPI_Wtime();
    printf("Node %d selesai dalam %.4f detik\n", rank, end - start);

    // Gather: kumpulkan hasil dari semua node ke master
    MPI_Gatherv(local_output, local_rows * width * channels, MPI_UNSIGNED_CHAR,
                result, sendcounts, displs, MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    // Master simpan hasil
    if (rank == 0) {
        double total_time = end - start;
        printf("\n=== Hasil Distributed ===\n");
        printf("Waktu eksekusi distributed: %.4f detik\n", total_time);
        printf("Speedup vs Serial: %.2fx\n", 0.1910 / total_time);

        stbi_write_jpg("../images/output/output_distributed.jpg",
                       width, height, channels, result, 100);
        printf("Output disimpan ke images/output/output_distributed.jpg\n");

        stbi_image_free(img);
        free(result);
    }

    free(local_input);
    free(local_output);
    free(sendcounts);
    free(displs);

    MPI_Finalize();
    return 0;
}