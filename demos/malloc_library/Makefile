# Compiler
CC = gcc
# Here PIC stands for Position Independent Code.
# Required when building shared libraries (.so).
# Ensures that the compiled code can be loaded at any memory address.
# Important for LD_PRELOAD libraries, because the OS can load our .so anywhere in memory.
CFLAGS = -O2 -fPIC
# Links against libdl, the dynamic linking library.
# Needed for functions like dlsym() and dlopen().
# Without this, our wrapper won't compile because dlsym would be undefined.
LDFLAGS = -ldl

# Files
WRAPPER_SRC = libmalloc_free_wrap.c
WRAPPER_LIB = libmalloc_free_wrap.so
TEST_SRC = test_malloc.c
TEST_BIN = test_malloc

# Default target
all: $(WRAPPER_LIB) $(TEST_BIN)

# Build the shared wrapper library
# -shared tells GCC to produce a shared library (.so) instead of a normal executable.
# Shared libraries are dynamically loaded at runtime, which is exactly what we want for LD_PRELOAD.
$(WRAPPER_LIB): $(WRAPPER_SRC)
	$(CC) $(CFLAGS) -shared $^ -o $@ $(LDFLAGS)

# Build the test program
$(TEST_BIN): $(TEST_SRC)
	$(CC) $(TEST_SRC) -o $@

# Run the test with LD_PRELOAD
run: all
	LD_PRELOAD=./$(WRAPPER_LIB) ./$(TEST_BIN)

# Clean build files
clean:
	rm -f $(WRAPPER_LIB) $(TEST_BIN)
