#!/bin/bash
# Download tinygltf header-only library

TINYGLTF_URL="https://raw.githubusercontent.com/syoyo/tinygltf/release/tiny_gltf.h"
DEST_DIR="include/pylmesh/external"

mkdir -p "$DEST_DIR"
cd "$DEST_DIR"

if [ ! -f "tiny_gltf.h" ]; then
    echo "Downloading tinygltf..."
    curl -L -o tiny_gltf.h "$TINYGLTF_URL"
    echo "Downloaded tiny_gltf.h"
else
    echo "tiny_gltf.h already exists"
fi
