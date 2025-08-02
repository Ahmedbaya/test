# Makefile for MOKP DLL and test programs

CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC
LDFLAGS = -lm

# Source files
DLL_SOURCES = mokp_dll.c mokp_core.c indicators.c
DLL_HEADERS = mokp_dll.h mokp_core.h indicators.h Common.h IBMOLS.h
TEST_SOURCES = test_mokp_dll.c
ORIGINAL_SOURCES = main.c indicators.c

# Object files
DLL_OBJECTS = $(DLL_SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

# Target names
DLL_TARGET = libmokp.so
STATIC_TARGET = libmokp.a
TEST_TARGET = test_mokp_dll
ORIGINAL_TARGET = original_main

# Platform-specific settings
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    DLL_TARGET = libmokp.so
    DLL_FLAGS = -shared
endif
ifeq ($(UNAME_S),Darwin)
    DLL_TARGET = libmokp.dylib
    DLL_FLAGS = -dynamiclib
endif
ifdef OS
    # Windows (assume MinGW)
    DLL_TARGET = mokp.dll
    DLL_FLAGS = -shared -Wl,--out-implib,libmokp.a
    CFLAGS += -DMOKP_DLL_EXPORTS
endif

.PHONY: all clean dll static test original install

# Default target
all: dll static test

# Build shared library (DLL)
dll: $(DLL_TARGET)

$(DLL_TARGET): $(DLL_OBJECTS)
	$(CC) $(DLL_FLAGS) -o $@ $^ $(LDFLAGS)

# Build static library
static: $(STATIC_TARGET)

$(STATIC_TARGET): $(DLL_OBJECTS)
	ar rcs $@ $^

# Build test program
test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJECTS) $(DLL_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Build original main (for comparison)
original: $(ORIGINAL_TARGET)

$(ORIGINAL_TARGET): main.o indicators.o
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile source files to object files
%.o: %.c $(DLL_HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Special rule for main.o (original main)
main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f *.o $(DLL_TARGET) $(STATIC_TARGET) $(TEST_TARGET) $(ORIGINAL_TARGET)
	rm -f *.dll *.dylib *.so *.a

# Install library (Linux/macOS)
install: $(DLL_TARGET) $(STATIC_TARGET)
	sudo cp $(DLL_TARGET) /usr/local/lib/
	sudo cp $(STATIC_TARGET) /usr/local/lib/
	sudo cp mokp_dll.h /usr/local/include/
	sudo ldconfig 2>/dev/null || true

# Run tests
run-test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Show library information
info:
	@echo "Platform: $(UNAME_S)"
	@echo "DLL Target: $(DLL_TARGET)"
	@echo "Compiler: $(CC)"
	@echo "Flags: $(CFLAGS)"
	@echo "Sources: $(DLL_SOURCES)"

# Help
help:
	@echo "Available targets:"
	@echo "  all       - Build DLL, static library, and test program"
	@echo "  dll       - Build shared library (DLL)"
	@echo "  static    - Build static library"
	@echo "  test      - Build test program"
	@echo "  original  - Build original main program"
	@echo "  clean     - Remove build artifacts"
	@echo "  install   - Install library system-wide (Linux/macOS)"
	@echo "  run-test  - Build and run test program"
	@echo "  info      - Show build configuration"
	@echo "  help      - Show this help message"