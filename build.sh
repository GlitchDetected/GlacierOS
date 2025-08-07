#!/bin/bash

# sudo port selfupdate
# chmod +x build.sh
# ./build.sh
set -e

PREFIX="$HOME/compiled"
TARGET=i686-elf
PATH="$PREFIX/bin:$PATH"
mkdir -p "$PREFIX"

install_deps() {
    if ! port installed "$1" | grep -q "active"; then
        echo "[*] Installing $1..."
        sudo port install "$1"
    else
        echo "[*] $1 already installed."
    fi
}

missing_file() {
    local url="$1"
    local file="${url##*/}"
    if [ ! -f "$file" ]; then
        echo "[*] Downloading $file..."
        wget "$url"
    else
        echo "[*] $file already exists, skipping download."
    fi
}

if_binary_exists() {
    ls "$PREFIX/bin"/i686-elf-* >/dev/null 2>&1
}

in_correct_path() {
    command -v i686-elf-gcc >/dev/null 2>&1
}

echo "[*] Checking and installing dependencies if missing..."
install_deps gmp
install_deps mpfr
install_deps libmpc
install_deps texinfo
install_deps wget
install_deps bison
install_deps flex
install_deps gcc13
install_deps isl
install_deps libiconv

mkdir -p ~/cross-compilers
cd ~/cross-compilers

if if_binary_exists && in_correct_path; then
    echo "[*] Binutils and GCC already installed and in PATH, skipping build."
    exit 0
elif if_binary_exists && ! in_correct_path; then
    echo "[*] Cross-compiler binaries found but NOT in PATH."
    echo "[*] Please add $PREFIX/bin to your PATH before running this script."
    exit 1
fi

    echo "[*] Building binutils..."
    missing_file https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz
    tar -xf binutils-2.41.tar.xz
    mkdir -p build-binutils && cd build-binutils
    ../binutils-2.41/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
    make -j"$(sysctl -n hw.ncpu)"
    make install
    cd ..

    echo "[*] Building GCC..."
    missing_file https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz
    tar -xf gcc-13.2.0.tar.xz
    mkdir build-gcc && cd build-gcc
    ../gcc-13.2.0/configure \
      --target=$TARGET \
      --prefix="$PREFIX" \
      --disable-nls \
      --enable-languages=c \
      --without-headers \
      --with-libiconv-prefix=/opt/local
    make all-gcc -j$(sysctl -n hw.ncpu)
    make all-target-libgcc -j$(sysctl -n hw.ncpu)
    make install-gcc
    make install-target-libgcc
    cd ..

echo "Cross compiler build completed"
echo
echo "Binaries installed in: $PREFIX/bin"
echo
echo "To use the cross-compiler, add this to your shell config:"
echo "    export PATH=\"$PREFIX/bin:\$PATH\""
echo
echo "Then verify with:"
echo "    i686-elf-gcc --version"