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

Your kernel module must create the following proc interface so that we can communicate with your kernel module from the user space:

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
$ make
$ sudo insmod pagecacheTest.ko
```

Run the provided program:

```bash
$ ./readFile 
PID: 11439
Read 4096 bytes
User buffer virtual address: 0x55555555a000
First 16 bytes: 67 b8 1b 04 0b 24 5b eb 00 9e b6 81 5a 7c 06 99
Waiting for kernel-module test...
```

While it is waiting, use the PID and virtual address it printed on a second terminal to communicate with the kernel module via the proc interface:

```bash
$ echo "<PID> <USER_VIRTUAL_ADDRESS> data.bin" > /proc/pagecache_test
```

For the above readFile output, we should run:

```bash
$ echo "11439 55555555a000 data.bin" > /proc/pagecache_test
```

Notice that the 0x is not needed when passing the address to the kernel module.

Then inspect the kernel output:

```bash
$ sudo dmesg | tail -n 63
[104046.546175] memcmp(PAGE_SIZE) = 0
[104046.546401] ========================================
[104046.765692] pagecache_test: unloaded
[104535.607963] audit: type=1400 audit(1791432000.180:199): apparmor="DENIED" operation="capable" class="cap" profile="/usr/sbin/cupsd" pid=11157 comm="cupsd" capability=12  capname="net_admin"
[104535.617734] audit: type=1400 audit(1791432000.189:200): apparmor="DENIED" operation="open" class="file" profile="snap.firmware-updater.firmware-notifier" name="/proc/sys/vm/max_map_count" pid=11021 comm="firmware-notifi" requested_mask="r" denied_mask="r" fsuid=1000 ouid=0
[105386.986155] pagecache_test: loaded
[105386.986630] Use /proc/pagecache_test to run the experiment

[105388.340325] ========================================
[105388.340549] Page Cache vs User Buffer Experiment
[105388.340815] ========================================
[105388.341093] PID:              11439
[105388.341301] User virtual addr: 0x55555555a000
[105388.341557] File:             data.bin

[105388.341868] USER BUFFER:
[105388.342018]   virtual address = 0x55555555a000
[105388.342276]   PFN              = 1829894
[105388.342665]   physical address = 0x1bec06000
[105388.342942]   first 16 bytes   =
[105388.342943]  67
[105388.343153]  b8
[105388.343276]  1b
[105388.343393]  04
[105388.343510]  0b
[105388.343624]  24
[105388.343739]  5b
[105388.343879]  eb
[105388.344006]  00
[105388.344122]  9e
[105388.344238]  b6
[105388.344353]  81
[105388.344469]  5a
[105388.344584]  7c
[105388.344702]  06
[105388.344846]  99


[105388.345305] PAGE CACHE:
[105388.345451]   file offset      = 0
[105388.345655]   PFN              = 1846174
[105388.345884]   physical address = 0x1c2b9e000
[105388.346136]   first 16 bytes   =
[105388.346137]  67
[105388.346323]  b8
[105388.346446]  1b
[105388.346574]  04
[105388.346707]  0b
[105388.346841]  24
[105388.346979]  5b
[105388.347112]  eb
[105388.347236]  00
[105388.347365]  9e
[105388.347488]  b6
[105388.347610]  81
[105388.347727]  5a
[105388.347845]  7c
[105388.347961]  06
[105388.348076]  99


[105388.348515] memcmp(PAGE_SIZE) = 0
[105388.348722] ========================================
```

### Automated Test

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

---

## The Big Idea

This assignment is not simply about finding PFNs.

You are using the kernel to **prove a performance and memory-management cost of ordinary file I/O**:

> **`read()` can require the kernel to copy file data from the page cache into a separate user-space buffer, resulting in the same data occupying two different physical pages.**

The goal is for you to be able to look at the two PFNs and say:

**“These are two different physical pages, but they contain the same file data.”**

## Submission

Submit the following files on Submitty:

1. `pagecacheTest.c` — your completed kernel module.

2. README.txt - Include your test result (when running the run.sh script) in the README. If your program works correctly and produces the right results, you don't need to include anything else in the README. If your program does not work as expected, you can add some explanation on what works and what does not work and reflect on why your program does not work, and what you have tried (to troubleshoot) - such reflection/explanation may help you earn some partial credit.

## Due Date

10/26/2026, 11:59pm.

## APIs

- Passing structured input from user space to a kernel module through a /proc entry provides a simple interface for controlling or configuring kernel functionality. The module's .proc_write handler receives the entire input as a user-space buffer, uses copy_from_user() to safely copy it into kernel memory, and can then use sscanf() to parse multiple arguments such as a PID, virtual address, and filename. Students should also validate the number of successfully parsed arguments before using them. See the [proc input example module](examples/proc) for a complete demonstration.

- Translating a user-space virtual address to its corresponding physical page in another process requires obtaining the process's `mm_struct` and using the kernel's page-management APIs. After obtaining the target process's `mm_struct` with `get_task_mm()`, call `mmap_read_lock()` before accessing the process's page tables, then use `get_user_pages_remote()` to resolve the user-space virtual address to a `struct page`. The `FOLL_GET` flag obtains a reference to the returned page, which must later be released with `put_page()`. Once the `struct page` is obtained, call `page_to_pfn()` to obtain its physical frame number (PFN), which can then be converted to a physical address by shifting it left by `PAGE_SHIFT`. Release the memory-map lock with `mmap_read_unlock()` after the page lookup is complete. See the [get_page example module](examples/get_page) for a complete demonstration.

## Grading Rubric

50 pts
 - Required Files (16 pts)
   - README file is missing. (-8)
   - README file is provided but complete testing results (when running run.sh) are missing. (-5)
   - pagecacheTest.c file is missing. (-8)
   - pagecacheTest.c file is provided but is significantly incomplete. (-5)
 - Correctness (34 pts)
   - Program fails to compile. (-34)
   - Hard code the content of the file (data.bin) in the kernel module and print the hard coded content. (-34)
   - Program fails to demonstrate that the two pages are two different physical pages. (-10)
   - Program fails to demonstrate that the two different physical pages contain identical content. (-10)
   - Program fails to print the first 16 bytes of these 2 pages. (-8)
   - Program prints the first 16 bytes of these 2 pages, but the content are different. (-6)
   - Program demonstrates expected behaviors but causes the kernel to crash. (-10)
