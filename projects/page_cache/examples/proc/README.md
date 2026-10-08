## Testing

### 1. Build the Module

Run:

```bash
make
```

This should create:

```text
my_proc.ko
```

### 2. Load the Kernel Module

```bash
sudo insmod my_proc.ko
```

Check that the `/proc` entry was created:

```bash
ls -l /proc/my_proc
```

### 3. Pass Input to the Kernel Module

The module expects three values:

```text
PID ADDRESS FILE
```

For example:

```bash
echo "12345 0x7f1234000000 data.bin" > /proc/my_proc
```

The kernel module receives the entire line as a string and uses `sscanf()` to parse the three values:

```text
12345              0x7f1234000000       data.bin
  │                         │                 │
  %d                       %lx              %255s
  │                         │                 │
  ↓                         ↓                 ↓
pid_number            user_address        filename
```

### 4. View the Kernel Output

```bash
sudo dmesg | tail
```

You should see output similar to:

```text
my_proc: PID     = 12345
my_proc: Address = 0x7f1234000000
my_proc: File    = data.bin
```

The exact PID and address depend on the values you provide.

### 5. Test Invalid Input

Try providing fewer than three arguments:

```bash
echo "12345 data.bin" > /proc/my_proc
```

The module should report:

```text
my_proc: invalid input
```

This demonstrates why the return value of `sscanf()` is checked:

```c
if (sscanf(input, "%d %lx %255s",
           &pid_number,
           &user_address,
           filename) != 3) {
    ...
}
```

### 6. Unload the Module

When finished:

```bash
sudo rmmod my_proc
```

The `/proc/my_proc` entry should be removed automatically when the module is unloaded.

### 7. Clean Up

Remove the generated kernel-module build files:

```bash
make clean
```
