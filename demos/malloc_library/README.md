# LD_PRELOAD malloc/free Wrapper Demo

This demo shows how Linux `LD_PRELOAD` can intercept calls to
`malloc()` and `free()`.

## Files

- `test_malloc.c` — simple program that calls `malloc()` and `free()`.
- `libmalloc_free_wrap.c` — shared library that wraps `malloc()` and `free()`.
- `Makefile` — builds the test program and shared library.

## Build

Run:

```bash
make
```

## Run

Run the test program with the wrapper preloaded:

```bash
LD_PRELOAD=./libmalloc_free_wrap.so ./test_malloc
```

The wrapper intercepts calls to `malloc()` and `free()` and prints
a message for each intercepted call.

## Expected Output

```text
[malloc wrapper] malloc called
[malloc wrapper] malloc called
[malloc wrapper] malloc called
Allocated memory
[free wrapper] free called
[free wrapper] free called
Freed memory
```
