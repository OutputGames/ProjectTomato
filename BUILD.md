# Building Tomato Engine

This document provides detailed instructions for building the Tomato Engine on various platforms.

## Prerequisites

### All Platforms
- CMake 3.15 or higher
- C++20 compatible compiler
- Git (for cloning with submodules)

### Windows
- Visual Studio 2019 or later (recommended)
- Or MinGW-w64 with GCC 10+

### Linux
- GCC 10+ or Clang 11+
- Development packages: `libgl1-mesa-dev`, `libx11-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxi-dev`

### macOS
- Xcode 12+ with command line tools
- macOS 10.15+ (Catalina or later)

## Quick Build (CMake)

```bash
# Clone with submodules
git clone --recursive https://github.com/OutputGames/ProjectTomato.git
cd ProjectTomato

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .

# Run examples
./bin/BasicExample  # Linux/macOS
# or
bin\BasicExample.exe  # Windows
```

## CMake Build Options

### Basic Options

```bash
# Build with examples (default: ON)
cmake -DTOMATO_BUILD_EXAMPLES=ON ..

# Build shared library instead of static (default: OFF)
cmake -DTOMATO_BUILD_SHARED=ON ..

# Specify build type
cmake -DCMAKE_BUILD_TYPE=Release ..  # Release, Debug, RelWithDebInfo, MinSizeRel
```

### Platform-Specific

#### Windows - Visual Studio
```bash
# Generate VS 2022 solution
cmake -G "Visual Studio 17 2022" ..

# Build from command line
cmake --build . --config Release

# Or open generated .sln file in Visual Studio
```

#### Windows - MinGW
```bash
cmake -G "MinGW Makefiles" ..
cmake --build .
```

#### Linux - Make
```bash
cmake -G "Unix Makefiles" ..
cmake --build . -j$(nproc)  # Use all CPU cores
```

#### Linux - Ninja (faster builds)
```bash
cmake -G "Ninja" ..
cmake --build .
```

#### macOS - Xcode
```bash
cmake -G "Xcode" ..
# Open generated .xcodeproj or build with:
cmake --build .
```

## Building with Premake5 (Alternative)

Premake5 is also supported, primarily for Windows Visual Studio users:

```bash
# Download premake5.exe from https://premake.github.io/

# Generate Visual Studio 2022 solution
premake5 vs2022

# Or for other IDEs
premake5 vs2019     # Visual Studio 2019
premake5 gmake2     # GNU Make

# Open generated solution in your IDE and build
```

## Troubleshooting

### CMake can't find dependencies
```bash
# Make sure submodules are initialized
git submodule update --init --recursive
```

### Build fails with "C++20 not supported"
```bash
# Specify a newer compiler
cmake -DCMAKE_CXX_COMPILER=g++-11 ..  # Linux
cmake -DCMAKE_CXX_COMPILER=clang++ ..  # macOS
```

### Missing OpenGL on Linux
```bash
# Ubuntu/Debian
sudo apt install libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev

# Fedora/RHEL
sudo dnf install mesa-libGL-devel libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel

# Arch
sudo pacman -S mesa libx11 libxrandr libxinerama libxcursor libxi
```

### Slow build times
```bash
# Use Ninja for faster builds
cmake -G "Ninja" ..

# Or use parallel builds with Make
cmake --build . -j$(nproc)  # Linux
cmake --build . -j%NUMBER_OF_PROCESSORS%  # Windows
```

## Advanced Build Configurations

### Debug Build with Symbols
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### Release Build with Debug Info
```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build .
```

### Minimum Size Release
```bash
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
cmake --build .
```

### Custom Install Location
```bash
cmake --install . --prefix /custom/install/path
```

## Building Only the Library (No Examples)

```bash
cmake -DTOMATO_BUILD_EXAMPLES=OFF ..
cmake --build .
```

## Integrating Tomato Engine in Your Project

### Using CMake

Add to your project's CMakeLists.txt:

```cmake
# Add Tomato Engine as subdirectory
add_subdirectory(path/to/TomatoEngine)

# Link against your executable
add_executable(MyGame main.cpp)
target_link_libraries(MyGame PRIVATE TomatoEngine)
```

### Using Installed Library

After installing with `cmake --install`:

```cmake
find_package(TomatoEngine REQUIRED)
target_link_libraries(MyGame PRIVATE TomatoEngine)
```

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
        
    steps:
    - uses: actions/checkout@v2
      with:
        submodules: recursive
        
    - name: Configure
      run: cmake -B build
      
    - name: Build
      run: cmake --build build --config Release
      
    - name: Test
      run: build/bin/BasicExample
```

## Verified Build Configurations

The following configurations are regularly tested:

- ✅ Windows 10/11 + Visual Studio 2022 + CMake 3.25
- ✅ Ubuntu 22.04 + GCC 11 + CMake 3.22
- ✅ macOS 13 + Xcode 14 + CMake 3.25
- ✅ Windows 10 + MinGW GCC 12 + CMake 3.25

## Getting Help

- Check [examples/](examples/) for working code
- See [README.md](README.md) for architecture overview
- Open an issue on GitHub for build problems
- Include your OS, compiler version, and CMake version when reporting issues
