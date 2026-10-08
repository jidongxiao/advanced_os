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

This has two costs:

1. **CPU cost:** the kernel must copy the data from the page-cache page into the user-space buffer.
2. **Memory cost:** the same data occupies two physical pages.

By contrast, with file-backed `mmap()`, the process can map the file's page-cache pages directly into its address space, avoiding this additional user-buffer copy.

### Your Goal

In this assignment, you will write a **Linux kernel module that provides direct physical-memory evidence of this duplication**.

Your module must prove that:

> **After a process reads a file using `read()`, the same file data can exist simultaneously in two different physical pages: one page belonging to the file's page cache and another page belonging to the process's user-space buffer.**

You will prove this by finding both physical pages, comparing their PFNs, and comparing their contents.

---

## What You Are Given

The user-space test program is **already provided**.

You do not need to write or modify it.

The program:

1. Opens `data.bin`.
2. Allocates a page-aligned user-space buffer.
3. Calls:

```c
read(fd, buffer, 4096);
```

4. Prints its PID and the virtual address of the buffer.
5. Waits for you to run your kernel module.

For example:

```text
PID: 12345
Read 4096 bytes
User buffer virtual address: 0x5d48d17b0000
First 16 bytes: ...
Press Enter to exit...
```

Your job is to write the **kernel module only**.

---

## Task: Write the Kernel Module

Create:

```text
pagecache_test.c
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

### Your module must:

### 1. Find the User-Buffer Page

Given the PID and virtual address:

* locate the process;
* obtain its `mm_struct`;
* obtain the `struct page *` corresponding to the user-space virtual address;
* determine its PFN.

Report something similar to:

```text
USER BUFFER:
  virtual address = 0x5d48d17b0000
  PFN              = 1585426
  physical address = 0x183112000
```

### 2. Find the Page-Cache Page

Open the specified file and obtain its `address_space`.

Find the page-cache page corresponding to **file offset 0**.

Report:

```text
PAGE CACHE:
  file offset      = 0
  PFN              = 1703623
  physical address = 0x19fec7000
```

### 3. Compare the Physical Pages

Compare the PFNs.

If they are different, the two pages occupy different physical memory locations.

Then compare the **entire contents of the two pages** (`PAGE_SIZE` bytes).

A successful experiment should produce:

```text
CONTENTS:           SAME

PFNs:               DIFFERENT

RESULT:
  TWO DIFFERENT PHYSICAL PAGES
  CONTAIN THE SAME FILE DATA.
```

This is the key result of the assignment.

It provides direct kernel-level evidence that the `read()` operation has resulted in **duplicate copies of the file data in physical memory**.

---

## Important Restrictions

### Do not use `mmap()`

The experiment is specifically about the behavior of normal buffered `read()`.

### Do not use `/proc/<pid>/maps`

The user-space program gives you the virtual address directly.

### Do not infer physical sharing from virtual addresses

You must obtain the actual `struct page` objects and PFNs.

### Compare the entire pages

Do not compare only the first few bytes.

The module must compare:

```c
PAGE_SIZE
```

bytes.

---

## Questions to Answer

Include brief answers in `README.txt`.

### 1. Why are there potentially two physical pages containing the same file data after `read()`?

### 2. What CPU operation creates the second copy?

### 3. Why does this consume additional physical memory?

### 4. How would accessing the same file through `mmap()` differ from using `read()`?

### 5. Does `read()` always result in two physical copies of the data? Explain why or why not.

---

## Expected Experiment

Build and load your module:

```bash
make
sudo insmod pagecache_test.ko
```

Run the provided program:

```bash
./read_copy
```

While it is waiting, use the PID and virtual address it printed:

```bash
echo "<PID> <USER_VIRTUAL_ADDRESS> data.bin" > /proc/pagecache_test
```

Then inspect the kernel output:

```bash
sudo dmesg | tail -50
```

When finished:

```bash
sudo rmmod pagecache_test
```

## Submission

Submit:

1. `pagecache_test.c` — your kernel module
2. `Makefile` — build script
3. `README.txt` — implementation description, test results, and answers to the questions

The user-space test program is provided and **must not be submitted**.

## The Big Idea

This assignment is not simply about finding PFNs.

You are using the kernel to **prove a performance and memory-management cost of ordinary file I/O**:

> **`read()` can require the kernel to copy file data from the page cache into a separate user-space buffer, resulting in the same data occupying two different physical pages.**

The goal is for you to be able to look at the two PFNs and say:

**“These are two different physical pages, but they contain the same file data.”**
