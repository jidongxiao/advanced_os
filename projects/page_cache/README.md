# Programming Assignment: Proving the Physical-Memory Cost of `read()`

## Learning Objectives

After completing this assignment, you should be able to:

* Explain how buffered `read()` moves file data between the **page cache** and a process's **user-space buffer**.
* Explain why this copy can result in the **same data occupying multiple physical memory pages**.
* Distinguish between a **user-space virtual address**, a `struct page`, a **PFN**, and a physical address.
* Use Linux kernel memory-management APIs to locate and inspect physical pages associated with a process and the page cache.
* Explain how file-backed `mmap()` can avoid the additional copy required by a conventional buffered `read()`.

## Background

**Why does `read()` potentially waste physical memory?**

When a process reads a regular file using `read()`, Linux normally brings the file data into the **page cache** and then copies that data into the process's **user-space buffer**.

This means that, for the same file data, physical memory can contain **two copies**:

```text
                 File
                   |
                   v
            +-------------+
            |  Page Cache |
            | Physical    |
            | Page A      |
            +-------------+
                   |
                   | copy
                   |   ← CPU must perform this copy
                   v
            +-------------+
            | User Buffer |
            | Physical    |
            | Page B      |
            +-------------+
```

Page A and Page B contain the same data, but they can be **different physical pages**.

This has two distinct costs:

1. Runtime performance cost: With read(), the kernel must copy the file data from the page-cache page into the user-space buffer. This consumes CPU time and memory-bandwidth and can also affect CPU caches.
2. Physical memory cost: After the copy, the same data may exist in two different physical pages—one in the page cache and one backing the user-space buffer. The duplicate data therefore consumes additional physical memory.

By contrast, with file-backed mmap(), the process can map the file's page-cache pages directly into its address space. This can avoid the additional copy and therefore avoids the corresponding CPU/memory-bandwidth cost and the extra physical page needed for the user-space copy.

### Your Goal

In this assignment, you will write a **Linux kernel module that provides direct physical-memory evidence of this duplication**.

Your module must prove that:

> **After a process reads a file using `read()`, the same file data can exist simultaneously in two different physical pages: one page belonging to the file's page cache and another page belonging to the process's user-space buffer.**

You will prove this by finding both physical pages, comparing their PFNs, and comparing their contents.

---

## What You Are Given

The following files are provided:

* `readFile.c` — the user-space test program
* `Makefile` — the build script
* `data.bin` — a single-page (4096-byte) file generated automatically by the provided Makefile

You are **not allowed to modify `read_copy.c` or the provided `Makefile`**.

The provided Makefile creates data.bin using:

dd if=/dev/urandom of=data.bin bs=4096 count=1

Therefore, data.bin contains exactly 4096 bytes (one memory page) of randomly generated data.

The provided user-space program:

1. Opens `data.bin`.
2. Allocates a page-aligned user-space buffer.
3. Calls:

```c
read(fd, buffer, 4096);
```

4. Prints its PID and the virtual address of the user-space buffer.
5. Waits for you to run your kernel module.

For example:

```text
PID: 12345
Read 4096 bytes
User buffer virtual address: 0x5d48d17b0000
First 16 bytes: ...
Press Enter to exit...
```

Your task is to implement **only the kernel module**:

```text
pagecacheTest.c
```

The provided Makefile already builds both the user-space program and the kernel module.

---

## Task: Write the Kernel Module

Create:

```text
pagecacheTest.c
```

Your kernel module must create:

```text
/proc/pagecache_test
```

The module should accept:

```bash
echo "<PID> <USER_VIRTUAL_ADDRESS> <FILE_NAME>" > /proc/pagecache_test
```

For example:

```bash
echo "12345 5d48d17b0000 data.bin" > /proc/pagecache_test
```

### Your module must: Compare the Physical Pages

Your kernel module must directly inspect the two physical pages:

* the physical page backing the user-space buffer, and
* the page-cache page corresponding to offset 0 of `data.bin`.

The module must:

1. Obtain the `struct page *` for each page.
2. Obtain and print the **actual PFN** of each page.
3. Obtain and print the **actual physical address** of each page.
4. Temporarily map each page into the kernel address space.
5. Print the **first 16 bytes of each physical page in hexadecimal**.
6. Compare the **entire `PAGE_SIZE` bytes** of the two pages using `memcmp()`.
7. Print the actual return value of `memcmp()`.

For example:

```text
USER BUFFER:
  PFN              = 1585426
  physical address = 0x183112000
  first 16 bytes   = 7a 31 9f 04 2c 81 ...

PAGE CACHE:
  PFN              = 1703623
  physical address = 0x19fec7000
  first 16 bytes   = 7a 31 9f 04 2c 81 ...

memcmp(PAGE_SIZE) = 0
```

The values printed for the PFNs, physical addresses, and page contents must be **obtained from the pages at runtime**. Do not hard-code any of these values.

Because `data.bin` is generated from `/dev/urandom`, the actual bytes will be different each time the experiment is run. Therefore, the output must reflect the bytes actually stored in the two physical pages.

A correct implementation should show that:

* the two PFNs are different, and
* the bytes read from the two pages match, with `memcmp(PAGE_SIZE)` returning `0`.

This provides direct evidence that the same 4096-byte file data exists in two different physical pages.

---

## Expected Experiment

Build and load your module:

```bash
make
sudo insmod pagecacheTest.ko
```

Run the provided program:

```bash
./readFile
```

While it is waiting, use the PID and virtual address it printed:

```bash
echo "<PID> <USER_VIRTUAL_ADDRESS> data.bin" > /proc/pagecache_test
```

Then inspect the kernel output:

```bash
sudo dmesg | tail -n 64
[103180.679078] ========================================
[103180.679309] Page Cache vs User Buffer Experiment
[103180.679584] ========================================
[103180.679873] PID:              9440
[103180.680299] User virtual addr: 0x55555555a000
[103180.680501] File:             data.bin

[103180.680839] USER BUFFER:
[103180.680986]   virtual address = 0x55555555a000
[103180.681180]   PFN              = 1576212
[103180.681353]   physical address = 0x180d14000
[103180.681564]   first 16 bytes   =
[103180.681564]  a9
[103180.681713]  69
[103180.681794]  1a
[103180.681906]  33
[103180.681995]  4f
[103180.682100]  77
[103180.682184]  a2
[103180.682262]  fb
[103180.682347]  d9
[103180.682431]  a3
[103180.682516]  5e
[103180.682599]  68
[103180.682689]  33
[103180.682770]  2a
[103180.682878]  5d
[103180.682974]  1b


[103180.683467] PAGE CACHE:
[103180.683660]   file offset      = 0
[103180.683888]   PFN              = 1785074
[103180.684153]   physical address = 0x1b3cf2000
[103180.684437]   first 16 bytes   =
[103180.684437]  a9
[103180.684645]  69
[103180.684774]  1a
[103180.684915]  33
[103180.685042]  4f
[103180.685165]  77
[103180.685286]  a2
[103180.685411]  fb
[103180.685536]  d9
[103180.685662]  a3
[103180.685783]  5e
[103180.685924]  68
[103180.686048]  33
[103180.686180]  2a
[103180.686313]  5d
[103180.686444]  1b


[103180.686935] memcmp(PAGE_SIZE) = 0
[103180.687152] ========================================
```

When finished:

```bash
sudo rmmod pagecacheTest
```

Automated Test

An automated test script, run.sh, is also provided. It runs the entire experiment with a single command, including:

Removing a previously loaded pagecacheTest module.
Building the experiment with make.
Loading the kernel module.
Running readFile.
Obtaining the process PID and user-space virtual address automatically.
Running the kernel-module test.
Displaying the kernel output.
Cleaning up the readFile process and kernel module.

To run the automated test:

chmod +x run.sh
./run.sh

You do not need to open multiple terminals or manually copy the PID and virtual address.

A successful automated run should look similar to:

```bash
$ ./run.sh 
========================================
Cleaning up previous module
========================================
Removing already-loaded pagecacheTest...

========================================
Building the experiment
========================================
make: Nothing to be done for 'all'.

========================================
Loading kernel module
========================================

========================================
Running readFile
========================================
readFile process started: PID 10885
PID: 10885
Read 4096 bytes
User buffer virtual address: 0x55555555b000
First 16 bytes: a9 69 1a 33 4f 77 a2 fb d9 a3 5e 68 33 2a 5d 1b
Waiting for kernel-module test...

========================================
Verifying readFile
========================================
    PID    PPID S COMMAND
  10885   10872 S readFile

readFile PID:     10885
User buffer:      0x55555555b000

========================================
Running kernel-module test
========================================

========================================
Kernel output
========================================
[103972.562152]   first 16 bytes   =
[103972.562153]  a9
[103972.562388]  69
[103972.562525]  1a
[103972.562648]  33
[103972.562794]  4f
[103972.562954]  77
[103972.563082]  a2
[103972.563211]  fb
[103972.563338]  d9
[103972.563461]  a3
[103972.563587]  5e
[103972.563712]  68
[103972.563852]  33
[103972.563979]  2a
[103972.564105]  5d
[103972.564233]  1b


[103972.564699] memcmp(PAGE_SIZE) = 0
[103972.564940] ========================================
[104046.497824] pagecache_test: unloaded
[104046.514687] pagecache_test: loaded
[104046.515154] Use /proc/pagecache_test to run the experiment

[104046.537350] ========================================
[104046.537587] Page Cache vs User Buffer Experiment
[104046.537925] ========================================
[104046.538249] PID:              10885
[104046.538479] User virtual addr: 0x55555555b000
[104046.538813] File:             data.bin

[104046.539168] USER BUFFER:
[104046.539351]   virtual address = 0x55555555b000
[104046.539651]   PFN              = 1571701
[104046.539940]   physical address = 0x17fb75000
[104046.540224]   first 16 bytes   =
[104046.540225]  a9
[104046.540432]  69
[104046.540559]  1a
[104046.540680]  33
[104046.540825]  4f
[104046.540948]  77
[104046.541075]  a2
[104046.541205]  fb
[104046.541315]  d9
[104046.541442]  a3
[104046.541570]  5e
[104046.541696]  68
[104046.541854]  33
[104046.541974]  2a
[104046.542101]  5d
[104046.542229]  1b


[104046.542682] PAGE CACHE:
[104046.542883]   file offset      = 0
[104046.543114]   PFN              = 1785074
[104046.543383]   physical address = 0x1b3cf2000
[104046.543663]   first 16 bytes   =
[104046.543664]  a9
[104046.543913]  69
[104046.544033]  1a
[104046.544152]  33
[104046.544283]  4f
[104046.544405]  77
[104046.544532]  a2
[104046.544653]  fb
[104046.544803]  d9
[104046.544922]  a3
[104046.545051]  5e
[104046.545177]  68
[104046.545302]  33
[104046.545424]  2a
[104046.545551]  5d
[104046.545673]  1b


[104046.546175] memcmp(PAGE_SIZE) = 0
[104046.546401] ========================================

========================================
Experiment complete
========================================

========================================
Cleaning up
========================================
Stopping readFile (PID 10885)...
Removing pagecacheTest...
```

The exact PFNs, physical addresses, timestamps, and file contents will vary between runs. However, a correct implementation must demonstrate the following:

* The **two physical addresses must be different**, showing that the user buffer and page cache are backed by **different physical pages**.
* The **contents of the two physical pages must be identical**, with the same bytes in the user-buffer page and the page-cache page.
* Therefore, `memcmp(PAGE_SIZE)` should return **`0`**.

These observations provide direct evidence that the same 4096-byte file data exists in **two different physical pages**.

## Submission

Submit the following file:

1. `pagecacheTest.c` — your completed kernel module

The following files are provided and **must not be modified**. Do not submit their two files**:

* `readFile.c`
* `Makefile`

2. a README file including your test results.

---

## The Big Idea

This assignment is not simply about finding PFNs.

You are using the kernel to **prove a performance and memory-management cost of ordinary file I/O**:

> **`read()` can require the kernel to copy file data from the page cache into a separate user-space buffer, resulting in the same data occupying two different physical pages.**

The goal is for you to be able to look at the two PFNs and say:

**“These are two different physical pages, but they contain the same file data.”**
