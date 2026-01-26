#!/bin/sh
set -e

CXX=${CXX:-g++}
CXXFLAGS="-std=c++20 -O0 -g"

mkdir -p bin

for src in tests/*.cpp; do
  name=$(basename "$src" .cpp)
  echo "[BUILD] $name"
  $CXX $CXXFLAGS "$src" -o "bin/$name"
done
