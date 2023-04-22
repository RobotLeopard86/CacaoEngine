#!/bin/bash

echo "Setting up Citrus Engine dev environment..."

echo "Fetching Git changes..."
git submodule update --recursive
git pull --recursive

echo "Setting up boost..."
mkdir ./tmp
wget https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.bz2 ./tmp/boost.tar.bz2
tar --bzip2 -xf ./tmp/boost.tar.bz2
cwd=$(pwd)
cd ./tmp/boost
./bootstrap.sh --with-libraries=all --prefix="$cwd/libs/boost"
./b2 install
cd $cwd

echo "Cleaning up..."
rm ./tmp

echo "Done. Thanks for contributing to Citrus Engine. Read CONTRIBUTING.md in the root directory for more info."