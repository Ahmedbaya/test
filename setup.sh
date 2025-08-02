#!/bin/bash
# Setup and build script for MOKP DLL

set -e  # Exit on any error

echo "MOKP DLL Setup and Build Script"
echo "==============================="
echo

# Detect platform
OS="$(uname -s)"
case "${OS}" in
    Linux*)     PLATFORM=Linux;;
    Darwin*)    PLATFORM=macOS;;
    CYGWIN*)    PLATFORM=Windows;;
    MINGW*)     PLATFORM=Windows;;
    *)          PLATFORM="Unknown:${OS}"
esac

echo "Detected platform: ${PLATFORM}"
echo

# Check for required tools
echo "Checking requirements..."

if ! command -v gcc &> /dev/null; then
    echo "ERROR: GCC compiler not found. Please install GCC."
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo "ERROR: Make not found. Please install Make."
    exit 1
fi

echo "✓ GCC found: $(gcc --version | head -n1)"
echo "✓ Make found: $(make --version | head -n1)"
echo

# Check for required files
echo "Checking required files..."
REQUIRED_FILES=(
    "mokp_dll.h"
    "mokp_dll.c"
    "mokp_core.h"
    "mokp_core.c"
    "indicators.h"
    "indicators.c"
    "Common.h"
    "IBMOLS.h"
    "Makefile"
    "250.2.txt"
    "Weights_2obj_FQ200.txt"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [[ ! -f "$file" ]]; then
        echo "ERROR: Required file '$file' not found."
        exit 1
    fi
done

echo "✓ All required files found."
echo

# Build the project
echo "Building MOKP DLL..."
make clean
make all

echo
echo "Build completed successfully!"
echo

# List created files
echo "Created files:"
if [[ "$PLATFORM" == "Linux" ]] || [[ "$PLATFORM" == "macOS" ]]; then
    ls -la libmokp.so libmokp.a test_mokp_dll 2>/dev/null || true
elif [[ "$PLATFORM" == "Windows" ]]; then
    ls -la mokp.dll libmokp.a test_mokp_dll.exe 2>/dev/null || true
fi

echo
echo "Testing the DLL..."
if [[ "$PLATFORM" == "Windows" ]]; then
    ./test_mokp_dll.exe
else
    ./test_mokp_dll
fi

echo
echo "✓ DLL test completed successfully!"
echo

# Test Python integration if Python is available
if command -v python3 &> /dev/null; then
    echo "Testing Python integration..."
    python3 python_example_simple.py
    echo "✓ Python integration test completed!"
else
    echo "Note: Python3 not found. Skipping Python integration test."
    echo "To test Python integration later, run: python3 python_example_simple.py"
fi

echo
echo "Setup and build completed successfully!"
echo
echo "Next steps:"
echo "1. Use the DLL in your C/C++ projects by linking against libmokp.a"
echo "2. Use the shared library in Python with the provided wrapper"
echo "3. See README_DLL.md for detailed documentation"
echo "4. Modify problem files and weight files as needed"
echo
echo "Example usage:"
echo "  C/C++: Link with -L. -lmokp"
echo "  Python: Use python_example_simple.py as a starting point"