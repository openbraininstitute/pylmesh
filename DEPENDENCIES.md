# Dependencies

## Overview

pylmesh uses header-only libraries for all external dependencies, making it easy to build and distribute.

## Automatic Dependency Management

All dependencies are **automatically downloaded** during CMake configuration. No manual installation required!

### Downloaded Dependencies

When `ENABLE_GLTF=ON` (default), CMake automatically downloads:

1. **tinygltf** (274KB)
   - URL: https://github.com/syoyo/tinygltf
   - Purpose: GLTF/GLB file format support
   - License: MIT

2. **nlohmann/json** (945KB)
   - URL: https://github.com/nlohmann/json
   - Purpose: JSON parsing (required by tinygltf)
   - License: MIT

3. **stb_image** (277KB)
   - URL: https://github.com/nothings/stb
   - Purpose: Image loading (required by tinygltf)
   - License: Public Domain

4. **stb_image_write** (70KB)
   - URL: https://github.com/nothings/stb
   - Purpose: Image writing (required by tinygltf)
   - License: Public Domain

Total size: ~1.6MB (header-only, no linking required)

## Build Requirements

### Python Package
```bash
pip install scikit-build-core pybind11
pip install .
```

### C++ Library
```bash
cmake >= 3.15
C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
```

## Disabling GLTF Support

To build without GLTF/GLB support (reduces dependencies):

### Python
```bash
pip install . --config-settings=cmake.args="-DENABLE_GLTF=OFF"
```

### C++
```bash
cmake -DENABLE_GLTF=OFF ..
make
```

This will:
- Skip downloading tinygltf, json, stb headers
- Disable GLTF/GLB import/export
- Reduce package size by ~1.6MB
- Still support OBJ, STL, PLY, OFF formats

## Manual Dependency Installation

If automatic download fails (firewall, no internet), manually download to `include/pylmesh/external/`:

```bash
cd include/pylmesh/external

# Download tinygltf
curl -L -O https://raw.githubusercontent.com/syoyo/tinygltf/release/tiny_gltf.h

# Download nlohmann json
curl -L -O https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp

# Download stb_image
curl -L -O https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

# Download stb_image_write
curl -L -O https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
```

Then build normally:
```bash
pip install .
```

## Runtime Dependencies

**None!** All dependencies are header-only and compiled into the library.

## License Compatibility

All dependencies use permissive licenses (MIT, Public Domain) compatible with commercial use.
