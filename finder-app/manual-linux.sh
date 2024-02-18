#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
SYSROOT_DIR=$(realpath $(${CROSS_COMPILE}gcc --print-sysroot))

echo "Using ${SYSROOT_DIR} as sysroot directory"

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # Build the kernel image
    # git apply /home/chien/Desktop/Coursera/Linux_Embedded_System/assignments-3-and-later-NguyenChien215/e33a814e772cdc36436c8c188d8c42d019fda639.patch
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper # "Deep clean" - the kernel buildtree - removing the .config file with any existing configurations
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig # Configure for our "virt" arm dev board we will simulate in QEMU
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all # Build a kernel image for booting with QEMU
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules # Build any kernel modules
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs # Build the device tree
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# Create necessary base directories
mkdir rootfs && cd rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}

    # Configure busybox
    make distclean 
    make defconfig
else
    cd busybox

fi

# Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "Shared library"

# Add library dependencies to rootfs
cp ${SYSROOT_DIR}/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib
cp ${SYSROOT_DIR}/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64
cp ${SYSROOT_DIR}/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64
cp ${SYSROOT_DIR}/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64

# Make device nodes
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/console c 5 1

# Clean and build the writer utility
cd "$FINDER_APP_DIR"
make clean
make CROSS_COMPILE=${CROSS_COMPILE} writer

# Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp -r ./* ${OUTDIR}/rootfs/home

# Chown the root directory
sudo chown -R root:root ${OUTDIR}/rootfs

# Create initramfs.cpio.gz
cd "${OUTDIR}/rootfs"
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ../
gzip -f initramfs.cpio