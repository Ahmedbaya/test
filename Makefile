# Makefile for Multi-Objective Knapsack DLL
# Supports Windows (DLL), Linux (SO), and macOS (DYLIB)

# Compiler settings
CC = gcc
CFLAGS = -std=c99 -O2 -Wall -Wextra -fPIC
LDFLAGS = -shared -lm

# Source files
CORE_SOURCES = knapsack_core.c
DLL_SOURCES = knapsack_dll.c
HEADERS = knapsack_dll.h knapsack_core.h Common.h

# Object files
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)
DLL_OBJECTS = $(DLL_SOURCES:.c=.o)
ALL_OBJECTS = $(CORE_OBJECTS) $(DLL_OBJECTS)

# Platform detection and settings
UNAME_S := $(shell uname -s)

# Set defaults for Linux
TARGET = libknapsack.so
CFLAGS += -DLINUX

# Override for macOS
ifeq ($(UNAME_S),Darwin)
    TARGET = libknapsack.dylib
    CFLAGS += -DMACOS
    LDFLAGS = -dynamiclib -lm
endif

# Override for Windows (when cross-compiling or using mingw)
ifeq ($(CC),x86_64-w64-mingw32-gcc)
    TARGET = knapsack.dll
    CFLAGS += -DWINDOWS -DKNAPSACK_DLL_EXPORTS
    LDFLAGS = -shared -Wl,--out-implib,libknapsack.a -lm
endif

# Default target
all: $(TARGET)

# Build the shared library/DLL
$(TARGET): $(ALL_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "Built $(TARGET) successfully"

# Compile source files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Test compilation (static executable for testing)
test_static: test_dll.c $(CORE_SOURCES) $(DLL_SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -DTEST_MODE -o test_static test_dll.c $(CORE_SOURCES) $(DLL_SOURCES) -lm
	@echo "Built test_static successfully"

# Test the DLL with C test program
test_dll: $(TARGET) test_dll.c
	$(CC) $(CFLAGS) -o test_dll test_dll.c -L. -lknapsack -lm
	@echo "Built test_dll successfully"

# Clean build files
clean:
	rm -f $(ALL_OBJECTS) $(TARGET) libknapsack.a test_static test_dll
	@echo "Cleaned build files"

# Install (copy to system library directory) - Linux/macOS only
install: $(TARGET)
ifeq ($(UNAME_S),Linux)
	sudo cp $(TARGET) /usr/local/lib/
	sudo ldconfig
	@echo "Installed $(TARGET) to /usr/local/lib/"
endif
ifeq ($(UNAME_S),Darwin)
	sudo cp $(TARGET) /usr/local/lib/
	@echo "Installed $(TARGET) to /usr/local/lib/"
endif

# Uninstall - Linux/macOS only
uninstall:
ifeq ($(UNAME_S),Linux)
	sudo rm -f /usr/local/lib/$(TARGET)
	sudo ldconfig
	@echo "Uninstalled $(TARGET)"
endif
ifeq ($(UNAME_S),Darwin)
	sudo rm -f /usr/local/lib/$(TARGET)
	@echo "Uninstalled $(TARGET)"
endif

# Build for Windows from Linux (cross-compilation)
windows:
	$(MAKE) TARGET=knapsack.dll CC=x86_64-w64-mingw32-gcc \
		CFLAGS="$(CFLAGS) -DWINDOWS -DKNAPSACK_DLL_EXPORTS" \
		LDFLAGS="-shared -Wl,--out-implib,libknapsack.a -lm"

# Build for multiple platforms
all_platforms: clean
	$(MAKE) linux
	$(MAKE) clean
	$(MAKE) macos  
	$(MAKE) clean
	$(MAKE) windows

linux:
	$(MAKE) TARGET=libknapsack.so CFLAGS="$(CFLAGS) -DLINUX"

macos:
	$(MAKE) TARGET=libknapsack.dylib CFLAGS="$(CFLAGS) -DMACOS" \
		LDFLAGS="-dynamiclib -lm"

# Python testing (requires Python)
test_python: $(TARGET)
	python3 -c "import sys; sys.path.append('.'); import knapsack_python; print('Python interface test passed')"

# Create example data file for testing
create_test_data:
	@echo "Creating test data file..."
	@echo "2 5" > test_problem.txt
	@echo "10.0" >> test_problem.txt
	@echo "1:" >> test_problem.txt
	@echo "2" >> test_problem.txt
	@echo "3" >> test_problem.txt
	@echo "2:" >> test_problem.txt
	@echo "3" >> test_problem.txt
	@echo "4" >> test_problem.txt
	@echo "3:" >> test_problem.txt
	@echo "4" >> test_problem.txt
	@echo "5" >> test_problem.txt
	@echo "4:" >> test_problem.txt
	@echo "5" >> test_problem.txt
	@echo "6" >> test_problem.txt
	@echo "5:" >> test_problem.txt
	@echo "1" >> test_problem.txt
	@echo "2" >> test_problem.txt
	@echo "15.0" >> test_problem.txt
	@echo "1:" >> test_problem.txt
	@echo "1" >> test_problem.txt
	@echo "5" >> test_problem.txt
	@echo "2:" >> test_problem.txt
	@echo "2" >> test_problem.txt
	@echo "6" >> test_problem.txt
	@echo "3:" >> test_problem.txt
	@echo "3" >> test_problem.txt
	@echo "7" >> test_problem.txt
	@echo "4:" >> test_problem.txt
	@echo "4" >> test_problem.txt
	@echo "8" >> test_problem.txt
	@echo "5:" >> test_problem.txt
	@echo "2" >> test_problem.txt
	@echo "4" >> test_problem.txt
	@echo "Created test_problem.txt"

# Run all tests
test: $(TARGET) test_static
	@echo "Running static test..."
	./test_static
	@echo "Testing Python interface..."
	python3 example_usage.py

# Help target
help:
	@echo "Multi-Objective Knapsack DLL Build System"
	@echo "=========================================="
	@echo ""
	@echo "Targets:"
	@echo "  all            - Build for current platform"
	@echo "  clean          - Remove build files"
	@echo "  install        - Install library to system (Linux/macOS)"
	@echo "  uninstall      - Remove library from system (Linux/macOS)"
	@echo "  test_static    - Build static test executable"
	@echo "  test_dll       - Build DLL test executable"
	@echo "  test_python    - Test Python interface"
	@echo "  test           - Run all tests"
	@echo "  windows        - Cross-compile for Windows"
	@echo "  linux          - Build for Linux"
	@echo "  macos          - Build for macOS"
	@echo "  all_platforms  - Build for all platforms"
	@echo "  create_test_data - Create sample test data"
	@echo "  help           - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  CC             - Compiler (default: gcc)"
	@echo "  CFLAGS         - Compiler flags"
	@echo "  LDFLAGS        - Linker flags"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build for current platform"
	@echo "  make windows            # Cross-compile for Windows"
	@echo "  make CC=clang           # Use clang compiler"
	@echo "  make CFLAGS='-O3 -g'    # Custom compiler flags"

.PHONY: all clean install uninstall test_static test_dll test_python test windows linux macos all_platforms create_test_data help