#!/bin/sh
set -e

CXX=${CXX:-g++}
CXXFLAGS="-std=c++17 -I metro"

mkdir -p bin

for src in tests/*.cpp; do
  name=$(basename "$src" .cpp)
  echo "[BUILD] $name"
  $CXX $CXXFLAGS "$src" -o "bin/$name"
done
