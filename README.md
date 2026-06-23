# Image Processing - Komputasi Pararel Terdistribusi

## Universitas Muhammadiyah Tangerang
- **Mata Kuliah:** Komputasi Pararel Terdistribusi
- **Dosen:** Dyas Yudi Priyanggodo
- **Semester:** 6 / Kelas C1
- **Tahun Akademik:** 2025-2026

---

## Anggota Kelompok
| No | Nama | NIM |
|----|------|-----|
| 1  | (Muhammad Kifah Ibadurrahman) | (23552010180) |
| 2  | (Ridho Rachmadillah Alshani) | (23552010162) |
| 3  | (Dhimas Chandra Andhika) | (23552010195) |
| 4  | (Djildhan Sulviramadhan) | (23552010140) |

---

## Deskripsi Project
Project ini mengimplementasikan **Image Processing** (Gaussian Blur + Sobel Edge Detection) dalam 3 versi:
1. **Serial** - baseline menggunakan C murni
2. **Paralel** - menggunakan OpenMP (CPU) + CUDA (GPU)
3. **Terdistribusi** - menggunakan MPI di 3 node Azure Cloud

---

## Struktur Folder
image_processing_project/

├── serial/

│   ├── main.c

│   ├── gaussian_blur.c

│   ├── sobel_filter.c

│   ├── stb_image.h

│   └── stb_image_write.h

├── parallel/

│   ├── openmp_blur.c

│   └── cuda_sobel.cu

├── distributed/

│   ├── mpi_master.c

│   ├── mpi_worker.c

│   └── deploy.sh

├── benchmark/

│   ├── run_benchmark.sh

│   └── results.csv

├── images/

│   ├── input.jpg

│   └── output/

│       ├── output_serial.jpg

│       ├── output_openmp.jpg

│       ├── output_cuda.jpg

│       └── output_distributed.jpg

└── README.md

---

## Teknologi yang Digunakan
| Teknologi | Kegunaan |
|-----------|----------|
| C/C++ | Bahasa pemrograman utama |
| OpenMP | Paralelisasi CPU multi-core |
| CUDA | Paralelisasi GPU (Google Colab T4) |
| MPI (MPICH) | Komputasi terdistribusi |
| Azure for Students | Cloud platform (3 VM node) |
| STB Image | Library baca/tulis gambar |
| GitHub | Version control & repository |

---

## Cara Menjalankan

### 1. Versi Serial
```bash
# Compile
gcc -o serial/output_serial serial/main.c serial/gaussian_blur.c serial/sobel_filter.c -lm -I serial/

# Jalankan
cd serial
./output_serial
```

### 2. Versi OpenMP
```bash
# Compile
gcc -o parallel/output_openmp.exe parallel/openmp_blur.c -lm -fopenmp -I serial/

# Jalankan
cd parallel
./output_openmp.exe
```

### 3. Versi CUDA
```bash
# Di Google Colab
!nvcc -o cuda_sobel cuda_sobel.cu -lm
!./cuda_sobel
```
## Catatan Implementasi CUDA

Implementasi versi CUDA (`parallel/cuda_sobel.cu`) dikembangkan dan 
dieksekusi menggunakan **Google Colaboratory (Google Colab)** karena 
perangkat pengembangan tidak memiliki GPU NVIDIA secara lokal.

**Spesifikasi GPU yang digunakan:**
- GPU: NVIDIA Tesla T4
- VRAM: 15 GB
- CUDA Version: 13.0
- Driver Version: 580.82.07

**Alasan penggunaan Google Colab:**
Google Colab menyediakan akses GPU NVIDIA secara gratis, sehingga 
memungkinkan pengembangan dan pengujian kernel CUDA custom tanpa 
memerlukan hardware GPU secara fisik. Pendekatan ini umum digunakan 
dalam lingkungan akademik dan penelitian.

**File Google Colab tersedia di:**
[Link Google Drive / Colab Notebook]


### 4. Versi Distributed (MPI)
```bash
# Compile
mpicc -o distributed/mpi_master distributed/mpi_master.c -lm
mpicc -o distributed/mpi_worker distributed/mpi_worker.c -lm

# Jalankan di 3 node
mpirun -np 3 --hostfile hostfile ./mpi_master
```

---

## Hasil Benchmark

| Versi | Waktu (detik) | Speedup | Keterangan |
|-------|--------------|---------|------------|
| Serial | 0.1910 | 1.00x | Baseline |
| OpenMP | 0.0520 | 3.67x | 8 thread CPU |
| CUDA | 0.1141 | 1.67x | GPU Tesla T4 |
| Distributed | TBD | TBD | 3 node Azure |

### Analisis
- **OpenMP** memberikan speedup terbaik (3.67x) untuk ukuran gambar 728x410
- **CUDA** speedup lebih rendah karena gambar kecil (overhead transfer CPU→GPU)
- Untuk gambar berukuran besar (4K), CUDA akan jauh lebih cepat
- **Distributed** cocok untuk pemrosesan gambar dalam jumlah banyak

---

## Penggunaan AI Assistant
Project ini dikerjakan dengan bantuan **Claude AI (Anthropic)** untuk:
- Pembuatan struktur project
- Penulisan kode serial, OpenMP, CUDA, dan MPI
- Debugging dan troubleshooting
- Penjelasan konsep paralelisme

---

## Catatan Penting
- Matikan VM Azure setelah selesai untuk menghindari biaya tak terduga
- CUDA dijalankan di Google Colab (GPU Tesla T4 gratis)
- Library STB Image digunakan untuk membaca dan menulis file gambar