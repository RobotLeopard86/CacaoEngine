#!/bin/bash

if ! [ -f "$(which wget)" ]
then
    echo "This script requires wget!"
fi

echo "Setting up boost..."
mkdir ./tmp
echo "Downloading boost archive..."
wget -q --show-progress -O ./tmp/boost.tar.bz2 https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.bz2 && printf '\e[A\e[K'
echo "Unpacking boost archive..."
tar --bzip2 -xf ./tmp/boost.tar.bz2 -C ./tmp
cwd=$(pwd)
mv ./tmp/boost_1_82_0 ./tmp/boost
cd ./tmp/boost
echo "Setting up boost build environment..."
./bootstrap.sh --without-icu --without-libraries=python --prefix="$cwd/libs/boost"
echo "Installing boost library to Citrus Engine libraries directory..."
./b2 install -j14
cd $cwd

echo "The default compiler is Zig. Checking for presence of Zig on PATH..."
if ! [ -f "$(which zig)" ]
then
    echo "Zig is either not installed or is not on your PATH."
    read -p "Open the Zig download page in your browser? [y/n]" -n 1 -r
    if [[ $REPLY =~ ^[Yy]$ ]]
	then
		xdg-open "https://https://ziglang.org/download/"
	fi
else
    echo "Zig is installed."
fi

echo "Cleaning up..."
rm -rf ./tmp

echo "Done. Thanks for contributing to Citrus Engine. Read CONTRIBUTING.md in the root directory for more info."
