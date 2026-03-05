# Building pylmesh from Source

## Dependencies

### Build Dependencies
- CMake >= 3.15
- C++17 compatible compiler
- Python >= 3.8 (for Python bindings)
- pybind11 (for Python bindings)

### Runtime Dependencies
None! All dependencies are header-only and bundled.

### Optional Dependencies (Auto-downloaded)
For GLTF/GLB support (enabled by default):
- tinygltf (header-only)
- nlohmann/json (header-only)
- stb_image (header-only)
- stb_image_write (header-only)

These are automatically downloaded during CMake configuration.

## Installation

### Python Package

#### Basic Installation
```bash
pip install .
```

#### With Trimesh Support
```bash
pip install .[trimesh]
```

#### With Development Tools
```bash
pip install .[dev]
```

#### Install All Optional Dependencies
```bash
pip install .[trimesh,dev]
```

#### Install Build Dependencies First
```bash
pip install -r requirements-build.txt
pip install .
```

#### Build Without GLTF Support
```bash
pip install . --config-settings=cmake.args="-DENABLE_GLTF=OFF"
```

#### Build Without Draco Compression
```bash
pip install . --config-settings=cmake.args="-DENABLE_DRACO=OFF"
```

### C++ Library

#### With GLTF Support (default)
```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

#### Without GLTF Support
```bash
mkdir build && cd build
cmake -DENABLE_GLTF=OFF ..
make
sudo make install
```

## Build Options

### CMake Options
- `BUILD_EXAMPLES` - Build example programs (default: ON)
- `BUILD_TESTS` - Build unit tests (default: ON)
- `BUILD_PYTHON` - Build Python bindings (default: ON)
- `ENABLE_GLTF` - Enable GLTF/GLB support (default: ON)

### Example
```bash
cmake -DBUILD_EXAMPLES=OFF -DENABLE_GLTF=ON ..
```

## Troubleshooting

### Dependencies Not Downloading
If automatic download fails, manually download to `include/pylmesh/external/`:
```bash
cd include/pylmesh/external
curl -L -O https://raw.githubusercontent.com/syoyo/tinygltf/release/tiny_gltf.h
curl -L -O https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
curl -L -O https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
curl -L -O https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
```

### Build Fails
Try clean build:
```bash
rm -rf build dist *.egg-info
pip install . --no-cache-dir
```
