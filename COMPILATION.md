# MOKP DLL Compilation Instructions

## Quick Start

```bash
# Linux/macOS
make                    # Build shared library
make test              # Build and compile test
make run-test          # Run tests

# Windows (MinGW)
make dll               # Build Windows DLL

# All platforms
make clean             # Clean build files
```

## Platform-Specific Instructions

### Linux

#### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential gcc make

# CentOS/RHEL/Fedora
sudo yum install gcc make
# or
sudo dnf install gcc make

# Arch Linux
sudo pacman -S gcc make
```

#### Building
```bash
# Standard shared library (.so)
make

# Static library (.a) 
make static

# Debug build with symbols
make debug

# Install system-wide
sudo make install
```

#### Usage
```bash
# Set library path for testing
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
./test_mokp

# Or install system-wide
sudo make install
./test_mokp
```

### macOS

#### Prerequisites
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Or using Homebrew
brew install gcc make
```

#### Building
```bash
# Same as Linux
make                   # Builds libmokp.dylib
make test
make run-test
```

#### Usage
```bash
# Set library path
export DYLD_LIBRARY_PATH=.:$DYLD_LIBRARY_PATH
./test_mokp
```

### Windows

#### Option 1: MinGW-w64 (Recommended)

##### Prerequisites
```bash
# Install MinGW-w64
# Download from: https://www.mingw-w64.org/downloads/

# Or using MSYS2
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-make
```

##### Building
```bash
# Build DLL
make dll

# This creates:
# - mokp.dll (main library)
# - libmokp.dll.a (import library)
```

##### Usage
```bash
# Test the DLL
./test_mokp.exe

# Or from Command Prompt
test_mokp.exe
```

#### Option 2: Visual Studio

##### Prerequisites
- Visual Studio 2019 or later
- C/C++ development tools

##### Manual Project Setup
1. Create new C++ Console Application
2. Add source files:
   - `main.c`
   - `indicators.c` 
   - `mokp_dll.c`
3. Add header files:
   - `Common.h`
   - `IBMOLS.h`
   - `indicators.h`
   - `mokp_dll.h`
4. Project Properties:
   - Configuration Type: Dynamic Library (.dll)
   - C/C++ → Preprocessor → Definitions: Add `MOKP_DLL_EXPORTS`
   - Linker → Input → Additional Dependencies: Add `kernel32.lib`

##### Building
```
Build → Build Solution (Ctrl+Shift+B)
```

This creates:
- `mokp.dll`
- `mokp.lib` (import library)

#### Option 3: CMake (Cross-platform)

Create `CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.10)
project(MOKP)

set(CMAKE_C_STANDARD 99)

# Source files
set(SOURCES
    main.c
    indicators.c
    mokp_dll.c
)

set(HEADERS
    Common.h
    IBMOLS.h
    indicators.h
    mokp_dll.h
)

# Shared library
add_library(mokp SHARED ${SOURCES} ${HEADERS})

# Link math library on Unix
if(UNIX)
    target_link_libraries(mokp m)
endif()

# Export symbols on Windows
if(WIN32)
    target_compile_definitions(mokp PRIVATE MOKP_DLL_EXPORTS)
endif()

# Test executable
add_executable(test_mokp test_mokp_dll.c)
target_link_libraries(test_mokp mokp)
```

Build:
```bash
mkdir build
cd build
cmake ..
make              # Linux/macOS
# or
cmake --build .   # All platforms
```

## Advanced Compilation Options

### Optimization Levels
```bash
# Debug build
make CFLAGS="-g -O0 -DDEBUG"

# Release build  
make CFLAGS="-O3 -DNDEBUG"

# Size optimization
make CFLAGS="-Os"

# Maximum warnings
make CFLAGS="-Wall -Wextra -Wpedantic"
```

### Cross-Compilation

#### Linux to Windows
```bash
# Install MinGW cross-compiler
sudo apt-get install gcc-mingw-w64

# Cross-compile
CC=x86_64-w64-mingw32-gcc make dll
```

#### macOS to Windows/Linux
```bash
# Using Homebrew
brew install mingw-w64

# Cross-compile for Windows
CC=x86_64-w64-mingw32-gcc make dll
```

### Custom Build Variables
```bash
# Custom compiler
make CC=clang

# Custom flags
make CFLAGS="-O3 -march=native"

# Custom library name
make TARGET_LIB=libmokp_custom.so

# Verbose build
make V=1
```

## Linking Instructions

### Static Linking
```bash
# C/C++
gcc -o myprogram myprogram.c -L. -lmokp -lm -static

# Or with full path
gcc -o myprogram myprogram.c libmokp.a -lm
```

### Dynamic Linking
```bash
# Linux
gcc -o myprogram myprogram.c -L. -lmokp -lm
LD_LIBRARY_PATH=. ./myprogram

# Windows
gcc -o myprogram.exe myprogram.c -L. -lmokp
# Copy mokp.dll to same directory as myprogram.exe
```

### Python Integration
```python
# Automatic loading (library in same directory)
from mokp_python import MOKP

# Manual path specification
mokp = MOKP(lib_path="./libmokp.so")  # Linux
mokp = MOKP(lib_path="./mokp.dll")    # Windows
```

## Performance Optimization

### Compiler Optimizations
```bash
# Fast math (may reduce precision)
make CFLAGS="-O3 -ffast-math"

# Link-time optimization
make CFLAGS="-O3 -flto"

# Profile-guided optimization
make CFLAGS="-O3 -fprofile-generate"
./test_mokp  # Run with representative data
make clean
make CFLAGS="-O3 -fprofile-use"
```

### Memory Optimization
```bash
# Reduce memory usage
make CFLAGS="-Os -DMOKP_ARCHIVE_SIZE=10000"

# Stack protection
make CFLAGS="-fstack-protector-strong"
```

## Troubleshooting Build Issues

### Common Problems

#### 1. Missing Math Library
```bash
# Error: undefined reference to 'exp', 'log', etc.
# Solution: Add -lm flag
make LDFLAGS="-lm"
```

#### 2. Position Independent Code
```bash
# Error: requires position independent code
# Solution: Add -fPIC flag
make CFLAGS="-fPIC"
```

#### 3. Symbol Visibility
```bash
# Error: symbols not exported
# Linux: Ensure __attribute__((visibility("default")))
# Windows: Ensure __declspec(dllexport)
```

#### 4. Missing Headers
```bash
# Error: No such file or directory
# Solution: Check include paths
make CFLAGS="-I/path/to/headers"
```

### Debug Build Issues
```bash
# Enable all warnings
make CFLAGS="-Wall -Wextra -Wpedantic -Werror"

# Memory debugging
make CFLAGS="-fsanitize=address -g"

# Thread safety (if needed)
make CFLAGS="-pthread"
```

### Verification

#### Test Library Loading
```bash
# Linux
ldd libmokp.so
nm -D libmokp.so | grep mokp_

# Windows
dumpbin /exports mokp.dll

# macOS  
otool -L libmokp.dylib
nm -gU libmokp.dylib | grep mokp_
```

#### Runtime Testing
```bash
# Basic functionality
./test_mokp

# Memory check (Linux)
valgrind --leak-check=full ./test_mokp

# Performance profiling
time ./test_mokp
```

## Distribution

### Creating Release Package
```bash
# Build all variants
make clean
make                    # Shared library
make static            # Static library
make dll              # Windows DLL (if cross-compiling)

# Package files
tar -czf mokp-dll-v1.0.tar.gz \
    libmokp.so libmokp.a mokp.dll \
    mokp_dll.h mokp_python.py \
    README_DLL.md COMPILATION.md \
    test_mokp_dll.c Makefile
```

### Binary Compatibility
- **Linux**: glibc version compatibility
- **Windows**: MSVC runtime requirements  
- **macOS**: Minimum OS version targets

Use static linking or bundle dependencies for maximum compatibility.

## Integration Examples

### CMake Integration
```cmake
# Find library
find_library(MOKP_LIBRARY
    NAMES mokp libmokp
    PATHS /usr/local/lib
)

# Link to your target
target_link_libraries(your_target ${MOKP_LIBRARY})
```

### pkg-config Support
Create `mokp.pc`:
```
prefix=/usr/local
libdir=${prefix}/lib
includedir=${prefix}/include

Name: MOKP
Description: Multi-Objective Knapsack Problem Library
Version: 1.0
Libs: -L${libdir} -lmokp -lm
Cflags: -I${includedir}
```

Use:
```bash
gcc $(pkg-config --cflags --libs mokp) -o program program.c
```