## Programming Assignment: Implementing a Simple NTFS Driver

### Overview

In this assignment, you will write a Linux kernel module that implements a mini version of the NTFS file system. Your code must compile and run on the **course VM**.

### Learning Objectives

- Understand the NTFS file system.
- Understand the concepts of mounting and VFS superblocks.

### Specification

- This is a group project. Each group may consist of 1 to 4 students.

- An NTFS image is provided: [ntfs-img-kw-1.dd](ntfs-img-kw-1.dd). This image is a file created using `dd` and formatted as an NTFS file system. On a system that supports NTFS, users should be able to view files and create files on this image. For this assignment, your job is to implement a **minimal NTFS file system driver**. Your implementation only needs to support the following operations:

  - `mount` and `umount` 
  - `ls`, which lists the files and directories in the root directory.

- Your implementation must be submitted in a single C source file named `myntfs.c`. The file must contain no more than **1,000 lines of C code**. Do not split your implementation across multiple source or header files. Combining multiple lines of code into a single long line to circumvent this line limit is not permitted.

- A [Makefile](Makefile) is provided.

### Expected Results

First make sure the NTFS image is in your current directory:

```bash
test@test-vm:~$ ls ntfs-img-kw-1.dd
ntfs-img-kw-1.dd
```

Once your implementation is complete, load your kernel module, which should be named `myntfs.ko`. You should then be able to mount the file system at `/mnt`.

```bash
test@test-vm:~$ sudo insmod myntfs.ko
test@test-vm:~$ sudo mount -t myntfs ntfs-img-kw-1.dd /mnt -o loop
```
Once the file system is mounted, and if now you run `ls` to list the files and directories in the root directory of the file system, you should get the following results:

```bash
test@test-vm:~$ ls /mnt/
 dir-n-6   dir-r-4   file-n-1.dat   file-n-3.dat   file-n-4.dat   file-n-5.dat   file-r-1.dat   file-r-3.dat  'System Volume Information'
```

**We will only test the `mount` command and the `ls` command as shown above**.

After running these commands, you should also be able to unmount the file system and remove the kernel module. Neither operation should cause a crash.

```bash
test@test-vm:~$ sudo umount /mnt
test@test-vm:~$ sudo rmmod myntfs
test@test-vm:~$
```

### Submission

Submit the following two files via Submitty:

- `myntfs.c` — Your kernel module implementation.
- `README.txt` — Test results.

**Due Date:** 11:59 p.m., Monday, December 7, 2026.

Late submissions are accepted within three days of the deadline, with a 10% penalty applied to the earned grade. Submissions more than three days late will not be accepted or graded.

All programs must compile and run on the course VM.

Run all the commands shown in the Expected Results section and include their results in `README.txt`. Nothing else needs to be included in the README.

### Grading Rubric

**Total: 50 points**

| Criteria | Points |
|---|---:|
| `mount` command works correctly. After mounting, the provided NTFS image should be mounted at `/mnt`. You can use `mount \| grep myntfs` to verify that it is mounted successfully. There should be no crash of the mount program or kernel. | 15 |
| `ls` command works correctly. `sudo ls /mnt` should produce exactly the expected output. | 20 |
| `umount` command works correctly, without a program crash or kernel crash. | 5 |
| `README.txt` includes the required test results. | 10 |
| Loading or unloading the module causes a kernel crash. | -5 |
| Code can't compile on the course VM running 7.0-test Linux kernel. | -40 |
