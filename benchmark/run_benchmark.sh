#!/bin/bash

# ============================================
# Script Benchmarking Image Processing
# ============================================

echo "======================================"
echo "  BENCHMARKING IMAGE PROCESSING"
echo "======================================"

# Buat folder output kalau belum ada
mkdir -p ../images/output

# File hasil benchmark
RESULT_FILE="results.csv"

# Buat header CSV
echo "Versi,Waktu(detik),Speedup,Ukuran Gambar" > $RESULT_FILE

# ============================================
# BENCHMARK 1 - Serial
# ============================================
echo ""
echo "[1/3] Menjalankan versi Serial..."

cd ../serial
SERIAL_TIME=$(./output_serial | grep "Waktu eksekusi serial" | awk '{print $4}')
cd ../benchmark

echo "Serial,$SERIAL_TIME,1.0,728x410" >> $RESULT_FILE
echo "Serial selesai! Waktu: $SERIAL_TIME detik"

# ============================================
# BENCHMARK 2 - OpenMP
# ============================================
echo ""
echo "[2/3] Menjalankan versi OpenMP..."

cd ../parallel
OPENMP_TIME=$(./output_openmp.exe | grep "Waktu eksekusi OpenMP" | awk '{print $4}')
cd ../benchmark

OPENMP_SPEEDUP=$(echo "scale=2; $SERIAL_TIME / $OPENMP_TIME" | bc)
echo "OpenMP,$OPENMP_TIME,$OPENMP_SPEEDUP,728x410" >> $RESULT_FILE
echo "OpenMP selesai! Waktu: $OPENMP_TIME detik | Speedup: ${OPENMP_SPEEDUP}x"

# ============================================
# BENCHMARK 3 - CUDA (dari hasil Colab)
# ============================================
echo ""
echo "[3/3] Memasukkan hasil CUDA (dari Google Colab)..."

CUDA_TIME="0.1141"
CUDA_SPEEDUP=$(echo "scale=2; $SERIAL_TIME / $CUDA_TIME" | bc)
echo "CUDA,$CUDA_TIME,$CUDA_SPEEDUP,728x410" >> $RESULT_FILE
echo "CUDA selesai! Waktu: $CUDA_TIME detik | Speedup: ${CUDA_SPEEDUP}x"

# ============================================
# HASIL AKHIR
# ============================================
echo ""
echo "======================================"
echo "  HASIL BENCHMARK"
echo "======================================"
echo ""
echo "Versi      | Waktu (detik) | Speedup"
echo "-----------|---------------|--------"
echo "Serial     | $SERIAL_TIME        | 1.00x"
echo "OpenMP     | $OPENMP_TIME        | ${OPENMP_SPEEDUP}x"
echo "CUDA       | $CUDA_TIME        | ${CUDA_SPEEDUP}x"
echo "Distributed| (dari Azure)  | TBD"
echo ""
echo "Hasil tersimpan di: benchmark/results.csv"
echo "======================================"