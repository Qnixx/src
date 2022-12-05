#!/usr/bin/env bash
################################################################
# A script to build a GNU GCC and Binutils cross-compiler
# Based on https://wiki.osdev.org/GCC_Cross-Compiler
#
# Copyright (c) 2022, the Native authors.
# SPDX-License-Identifier: BSD-2-Clause
################################################################

################################################################
# Configuration
################################################################

# Exit if there is an error
set -e

# Target to use
TARGET=x86_64-elf

# Versions to build
# Always use the latest working version (test before updating)
BUT_VER=2.39
GCC_VER=12.2.0

# Tar file extension to use
# Always use the one with the smallest file size (check when updating version)
BUT_EXT=xz
GCC_EXT=xz

# Multicore builds
# Currrently automatic using nproc
CORES=$(($(nproc) + 1))
LOAD=$(nproc)

PREFIX="$(pwd)/cross"
export PATH="$PREFIX/bin:$PATH"

echo "Building $TARGET Binutils $BUT_VER and GCC $GCC_VER..."
echo "Cores: $CORES, load: $LOAD"

################################################################
# Source Tarballs
################################################################

BUT_TARBALL=binutils-$BUT_VER.tar.$BUT_EXT
GCC_TARBALL=gcc-$GCC_VER.tar.$GCC_EXT

mkdir -p buildcc
cd buildcc

# Download tarballs
echo "Downloading Binutils tarball..."
if [ ! -f $BUT_TARBALL ]; then
    wget https://ftp.gnu.org/gnu/binutils/$BUT_TARBALL
fi

echo "Downloading GCC tarball..."
if [ ! -f $GCC_TARBALL ]; then
    wget https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/$GCC_TARBALL
fi

# Unzip tarballs
printf "%s" "Unzipping Binutils tarball"
tar -xf $BUT_TARBALL --checkpoint=.400
echo "" # Newline :~)
printf "%s" "Unzipping GCC tarball"
tar -xf $GCC_TARBALL --checkpoint=.400

################################################################
# Building
################################################################

echo "Removing old build directories..."
rm -rf buildcc-gcc build-binutils

# Build binutils
mkdir buildcc-binutils
cd buildcc-binutils
echo "Configuring Binutils..."
../binutils-$BUT_VER/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
echo "Building Binutils..."
make -j$CORES -l$LOAD
echo "Installing Binutils..."
make install -j$CORES -l$LOAD
cd ..

# Build gcc
cd gcc-$GCC_VER
echo "Downloading prerequisites for GCC..."
contrib/download_prerequisites
cd ..
mkdir buildcc-gcc
cd buildcc-gcc
echo "Configuring GCC..."
../gcc-$GCC_VER/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
echo "Building all-gcc..."
make all-gcc -j$CORES -l$LOAD
echo "Building all-target-libgcc..."
make all-target-libgcc -j$CORES -l$LOAD
echo "Installing GCC..."
make install-gcc -j$CORES -l$LOAD
echo "Installing target-libgcc..."
make install-target-libgcc -j$CORES -l$LOAD
cd ../..

echo "Removing build directory..."
rm -rf buildcc

echo "Build complete, binaries are in $PREFIX/bin"

################################################################
# Basic Testing (just prints info for now)
################################################################

echo "Testing GCC..."
$TARGET-gcc -v

echo "Testing LD..."
$TARGET-ld -v

echo "Done!"
