#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void process_image(
    unsigned char* input,
    unsigned char* output,
    int width, int local_rows, int channels)
{
    // Gaussian Blur 5x5
    float kernel[5][5] = {
        {1/273.0f,  4/273.0f,  7/273.0f,  4/273.0f, 1/273.0f},
        {4/273.0f, 16/273.0f, 26/273.0f, 16/273.0f, 4/273.0f},
        {7/273.0f, 26/273.0f, 41/273.0f, 26/273.0f, 7/273.0f},
        {4/273.0f, 16/273.0f, 26/273.0f, 16/273.0f, 4/273.0f},
        {1/273.0f,  4/273.0f,  7/273.0f,  4/273.0f, 1/273.0f}
    };

    unsigned char* blurred = (unsigned char*)malloc(
        local_rows * width * channels);

    // Step 1: Gaussian Blur
    int radius = 2;
    for (int y = 0; y < local_rows; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < channels; c++) {
                float sum = 0.0f;
                for (int ky = -radius; ky <= radius; ky++) {
                    for (int kx = -radius; kx <= radius; kx++) {
                        int ny = y + ky;
                        int nx = x + kx;

                        if (ny < 0) ny = 0;
                        if (ny >= local_rows) ny = local_rows - 1;
                        if (nx < 0) nx = 0;
                        if (nx >= width) nx = width - 1;

                        sum += input[(ny * width + nx) * channels + c]
                               * kernel[ky + radius][kx + radius];
                    }
                }
                blurred[(y * width + x) * channels + c] = (unsigned char)sum;
            }
        }
    }

    // Step 2: Sobel Filter
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

                    float pixel = blurred[(ny * width + nx) * channels];
                    sumX += pixel * Gx[ky + 1][kx + 1];
                    sumY += pixel * Gy[ky + 1][kx + 1];
                }
            }

            float magnitude = sqrtf(sumX * sumX + sumY * sumY);
            if (magnitude > 255.0f) magnitude = 255.0f;
            if (magnitude < 0.0f)   magnitude = 0.0f;

            for (int c = 0; c < channels; c++) {
                output[(y * width + x) * channels + c] =
                    (unsigned char)magnitude;
            }
        }
    }

    free(blurred);
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Terima info gambar dari master
    int width = 0, height = 0, channels = 0;
    MPI_Bcast(&width,    1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height,   1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Hitung baris yang diterima node ini
    int rows_per_node = height / size;
    int extra_rows    = height % size;
    int local_rows    = rows_per_node + (rank < extra_rows ? 1 : 0);

    // Hitung sendcounts dan displacements
    int* sendcounts = (int*)malloc(size * sizeof(int));
    int* displs     = (int*)malloc(size * sizeof(int));

    int offset = 0;
    for (int i = 0; i < size; i++) {
        int rows    = rows_per_node + (i < extra_rows ? 1 : 0);
        sendcounts[i] = rows * width * channels;
        displs[i]     = offset;
        offset       += sendcounts[i];
    }

    // Alokasi buffer lokal
    unsigned char* local_input  = (unsigned char*)malloc(
        local_rows * width * channels);
    unsigned char* local_output = (unsigned char*)malloc(
        local_rows * width * channels);

    // Terima potongan gambar dari master
    MPI_Scatterv(NULL, sendcounts, displs, MPI_UNSIGNED_CHAR,
                 local_input, local_rows * width * channels,
                 MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    printf("Worker node %d memproses %d baris\n", rank, local_rows);

    // Proses gambar
    double start = MPI_Wtime();
    process_image(local_input, local_output, width, local_rows, channels);
    double end = MPI_Wtime();

    printf("Worker node %d selesai dalam %.4f detik\n", rank, end - start);

    // Kirim hasil balik ke master
    MPI_Gatherv(local_output, local_rows * width * channels, MPI_UNSIGNED_CHAR,
                NULL, sendcounts, displs, MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    free(local_input);
    free(local_output);
    free(sendcounts);
    free(displs);

    MPI_Finalize();
    return 0;
}