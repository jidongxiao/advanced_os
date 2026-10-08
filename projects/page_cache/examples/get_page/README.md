# Getting the Physical Page

This example demonstrates how a kernel module can translate a **user-space virtual address** into the `struct page` representing the physical page that backs that address.

The example uses:

* `find_get_pid()` and `get_pid_task()` to find a process.
* `get_task_mm()` to obtain the process's memory descriptor.
* `get_user_pages_remote()` to locate the physical page backing a user-space virtual address.
* `page_to_pfn()` to obtain the page frame number (PFN).

## Build

Run:

```bash
make
```

This should create:

```text
get_page.ko
```

## Load the Module

The example contains a PID and virtual address in the source code:

```c
pid_t pid = 12345;
unsigned long user_address = 0x7f1234000000;
```

These values must correspond to a **currently running process** and a valid user-space address in that process.

For example, you can use a process that is waiting:

```bash
sleep 1000 &
```

Find its PID:

```bash
ps
```

Then find a suitable virtual address belonging to that process. For example, you can inspect its memory mappings with:

```bash
cat /proc/<PID>/maps
```

Update `pid` and `user_address` in `get_page.c` accordingly, then rebuild:

```bash
make
```

## Run the Example

Load the module:

```bash
sudo insmod get_page.ko
```

View the kernel output:

```bash
sudo dmesg | tail
```

You should see output similar to:

```text
PID: 12345
Virtual address: 0x7f1234000000
PFN: 123456
Physical address: 0x1e240000
```

The exact values will vary between processes and runs.

## What the Module Does

The important part of the example is:

```c
mmap_read_lock(mm);

ret = get_user_pages_remote(mm,
                            user_address,
                            1,
                            FOLL_GET,
                            &page,
                            NULL);

mmap_read_unlock(mm);
```

This asks the kernel to locate the physical page backing `user_address`.

The returned `struct page *` can then be converted to a PFN:

```c
pfn = page_to_pfn(page);
```

The physical address of the beginning of that page is:

```c
pfn << PAGE_SHIFT
```

Conceptually:

```text
User virtual address
        |
        | get_user_pages_remote()
        v
   struct page *
        |
        | page_to_pfn()
        v
       PFN
        |
        | PFN << PAGE_SHIFT
        v
Physical address
```

## Unload the Module

When finished:

```bash
sudo rmmod get_page
```

Then clean the build files:

```bash
make clean
```
