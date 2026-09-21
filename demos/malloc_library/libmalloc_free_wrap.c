// This tells the C library "enable GNU extensions".
// It unlocks extra functions and constants that are not part of the standard C or POSIX specification.
// RTLD_NEXT is not part of standard POSIX. It only exists when GNU extensions are enabled.
#define _GNU_SOURCE
#include <dlfcn.h>
#include <unistd.h>

// this line means: malloc_t is a type for a pointer to a function that takes a size_t argument and returns a void*.
typedef void *(*malloc_t)(size_t);
typedef void (*free_t)(void *);

// Why do we use static here? Without static, a global variable could be visible to other files that include or link with this file.
// static limits the scope to this source file (.c) only. 
// This prevents name collisions and keeps our wrapper self-contained.
static malloc_t real_malloc = NULL;
static free_t real_free = NULL;

void *malloc(size_t size) {
    if (!real_malloc) {
        // dlsym() is a dynamic linking function provided by libdl 
        // that lets us look up the address of a symbol (function or variable) at runtime.
        // RTLD_NEXT is a special constant we can pass to dlsym(): 
        // It tells dlsym:
        // "Find the next occurrence of this symbol in the dynamic linking chain, after this library."
        // If our wrapper calls malloc() and tries to use dlsym(RTLD_DEFAULT, "malloc"), it might return our own wrapper again -> recursion.
        // RTLD_NEXT skips our library and returns the real malloc from libc (or whatever library provides it).
        // 
        // Also, dlsym() returns a void*, but void* by itself cannot be called as a function.
        // We have to cast it to the correct function pointer type
        real_malloc = (malloc_t)dlsym(RTLD_NEXT, "malloc");
    }
    // we should avoid using printf here, because printf might trigger calls to malloc, which may lead this program to an infinite loop and a crash.
    ssize_t written = write(2, "[malloc wrapper] malloc called\n", 32);
    if (written < 0) { 
	    // Ignore logging failure. 
    }
    return real_malloc ? real_malloc(size) : NULL;
}

void free(void *ptr) {
    if (!real_free) {
        real_free = (free_t)dlsym(RTLD_NEXT, "free");
    }
    ssize_t written = write(2, "[free wrapper] free called\n", 28);
    if (written < 0) { 
	    // Ignore logging failure. 
    }
    if (real_free) {
        real_free(ptr);
    }
}
