## Command

Load the module like this, which cause the code to run on CPU 0.

```bash
sudo taskset -c 0 insmod ./ipi_test.ko
```

Then check the dmesg log.
