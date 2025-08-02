# Makefile for MOKP DLL
CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC -std=c99
LDFLAGS = -shared
TARGET_LIB = libmokp.so
TARGET_DLL = mokp.dll
STATIC_LIB = libmokp.a

# Source files
SOURCES = main.c indicators.c mokp_dll.c
HEADERS = Common.h IBMOLS.h indicators.h mokp_dll.h
OBJECTS = $(SOURCES:.c=.o)

# Test files
TEST_SOURCES = test_mokp_dll.c
TEST_TARGET = test_mokp

# Default target
all: $(TARGET_LIB)

# Build shared library (Linux)
$(TARGET_LIB): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ -lm

# Build DLL (Windows with MinGW)
dll: CFLAGS += -DMOKP_DLL_EXPORTS
dll: LDFLAGS = -shared -Wl,--out-implib,libmokp.dll.a
dll: $(TARGET_DLL)

$(TARGET_DLL): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ -lm

# Build static library
static: $(STATIC_LIB)

$(STATIC_LIB): $(OBJECTS)
	ar rcs $@ $^

# Object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Test compilation
test: $(TARGET_LIB) $(TEST_TARGET)

$(TEST_TARGET): $(TEST_SOURCES) $(TARGET_LIB)
	$(CC) $(CFLAGS) -o $@ $< -L. -lmokp -lm

# Clean
clean:
	rm -f $(OBJECTS) $(TARGET_LIB) $(TARGET_DLL) $(STATIC_LIB) libmokp.dll.a $(TEST_TARGET)

# Install (Linux)
install: $(TARGET_LIB)
	cp $(TARGET_LIB) /usr/local/lib/
	cp mokp_dll.h /usr/local/include/
	ldconfig

# Uninstall (Linux)
uninstall:
	rm -f /usr/local/lib/$(TARGET_LIB)
	rm -f /usr/local/include/mokp_dll.h
	ldconfig

# Run tests
run-test: test
	./$(TEST_TARGET)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET_LIB)

# Documentation
docs:
	@echo "MOKP DLL Compilation Instructions:"
	@echo ""
	@echo "Linux:"
	@echo "  make                    # Build shared library (.so)"
	@echo "  make static             # Build static library (.a)"
	@echo "  make test               # Build and compile test program"
	@echo "  make run-test           # Run the test program"
	@echo "  make install            # Install library system-wide"
	@echo ""
	@echo "Windows (MinGW):"
	@echo "  make dll                # Build Windows DLL"
	@echo ""
	@echo "Cross-platform:"
	@echo "  make clean              # Clean build files"
	@echo "  make debug              # Build with debug symbols"
	@echo ""
	@echo "Required files:"
	@echo "  - Problem files: 250.2.txt, 250.3.txt, etc."
	@echo "  - Weight files: Weights_2obj_FQ200.txt, etc."

.PHONY: all dll static clean install uninstall test run-test debug docs