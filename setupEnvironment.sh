#!/bin/bash

echo "Setting up Citrus Engine dev environment..."

echo "Fetching Git changes..."
git submodule update --recursive
git pull

echo "Setting up boost..."
mkdir ./tmp
wget -O ./tmp/boost.tar.bz2 https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.bz2
tar --bzip2 -xf ./tmp/boost.tar.bz2 -C ./tmp
cwd=$(pwd)
mv ./tmp/boost_1_82_0 ./tmp/boost
cd ./tmp/boost
./bootstrap.sh --with-libraries=test,thread,system --prefix="$cwd/libs/boost"
./b2 install
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