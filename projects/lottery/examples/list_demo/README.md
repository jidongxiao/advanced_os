# How to Run and Observe the Output

## Build the module:

```bash
make
```

## Load the module:

```bash
sudo insmod list_demo.ko
```

## Check the kernel log (dmesg):

```bash
sudo dmesg | tail -n 10
```

Expected output:

```plaintext
[list_demo] Module loaded.
[list_demo] === Phase 1: Added items to Queue A using list_add_tail ===
[list_demo] Queue A contents: [101] -> [202] -> END
[list_demo] Queue B contents: (empty)
[list_demo] === Phase 2: Moved Item 101 to Queue B using list_move_tail ===
[list_demo] Queue A contents: [202] -> END
[list_demo] Queue B contents: [101] -> END
```

## Unload the module:

```bash
sudo rmmod list_demo
```
