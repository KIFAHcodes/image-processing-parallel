#!/bin/bash

# ============================================
# Script Deploy Image Processing - Azure VMs
# ============================================

# IP Address dari 3 VM Azure (isi setelah VM dibuat)
MASTER_IP="<IP_MASTER>"
WORKER1_IP="<IP_WORKER1>"
WORKER2_IP="<IP_WORKER2>"

USERNAME="azureuser"
PROJECT_DIR="/home/azureuser/image_processing"
KEY_FILE="~/.ssh/azure_key.pem"

echo "======================================"
echo "  DEPLOY IMAGE PROCESSING - 3 NODE"
echo "======================================"

# ============================================
# STEP 1 - Install dependencies di semua node
# ============================================
echo ""
echo "[Step 1] Install dependencies..."

for IP in $MASTER_IP $WORKER1_IP $WORKER2_IP; do
    echo "Install di node: $IP"
    ssh -i $KEY_FILE $USERNAME@$IP "
        sudo apt-get update -y &&
        sudo apt-get install -y gcc make mpich &&
        echo 'Install selesai di $IP'
    "
done

# ============================================
# STEP 2 - Upload file ke semua node
# ============================================
echo ""
echo "[Step 2] Upload file ke semua node..."

for IP in $MASTER_IP $WORKER1_IP $WORKER2_IP; do
    echo "Upload ke node: $IP"
    ssh -i $KEY_FILE $USERNAME@$IP "mkdir -p $PROJECT_DIR/images/output"

    # Upload kode
    scp -i $KEY_FILE distributed/mpi_master.c  $USERNAME@$IP:$PROJECT_DIR/
    scp -i $KEY_FILE distributed/mpi_worker.c  $USERNAME@$IP:$PROJECT_DIR/
    scp -i $KEY_FILE serial/stb_image.h         $USERNAME@$IP:$PROJECT_DIR/
    scp -i $KEY_FILE serial/stb_image_write.h   $USERNAME@$IP:$PROJECT_DIR/

    # Upload gambar input
    scp -i $KEY_FILE images/input.jpg           $USERNAME@$IP:$PROJECT_DIR/images/
done

# ============================================
# STEP 3 - Compile di master node
# ============================================
echo ""
echo "[Step 3] Compile program di master node..."

ssh -i $KEY_FILE $USERNAME@$MASTER_IP "
    cd $PROJECT_DIR &&
    mpicc -o mpi_master mpi_master.c -lm &&
    mpicc -o mpi_worker mpi_worker.c -lm &&
    echo 'Compile berhasil!'
"

# ============================================
# STEP 4 - Buat hostfile MPI
# ============================================
echo ""
echo "[Step 4] Buat hostfile MPI..."

ssh -i $KEY_FILE $USERNAME@$MASTER_IP "
    cat > $PROJECT_DIR/hostfile << EOF
$MASTER_IP slots=1
$WORKER1_IP slots=1
$WORKER2_IP slots=1
EOF
    echo 'Hostfile berhasil dibuat!'
    cat $PROJECT_DIR/hostfile
"

# ============================================
# STEP 5 - Jalankan program MPI
# ============================================
echo ""
echo "[Step 5] Jalankan program MPI di 3 node..."

ssh -i $KEY_FILE $USERNAME@$MASTER_IP "
    cd $PROJECT_DIR &&
    mpirun -np 3 --hostfile hostfile ./mpi_master
"

# ============================================
# STEP 6 - Download hasil output
# ============================================
echo ""
echo "[Step 6] Download hasil output..."

scp -i $KEY_FILE \
    $USERNAME@$MASTER_IP:$PROJECT_DIR/images/output/output_distributed.jpg \
    images/output/output_distributed.jpg

echo ""
echo "======================================"
echo "  DEPLOY SELESAI!"
echo "======================================"
echo "Output tersimpan di: images/output/output_distributed.jpg"