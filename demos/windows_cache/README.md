# Demo: Demonstrating Windows File Caching

This hands-on activity demonstrates how the Windows Cache Manager uses system memory (RAM) to cache file reads, drastically accelerating file access times on subsequent reads.

## Prerequisites

- Windows 10 or 11 system (standard user access; **no administrator privileges required**).
- PowerShell and Command Prompt (`cmd`).

## Step 1: Create a 256 MB File

Open Command Prompt (`cmd`) or PowerShell and run `fsutil` to create a 256 MB test file ($256 \times 1024 \times 1024 = 268,435,456\text{ bytes}$):

**DOS**

```text
fsutil file createnew C:\Users\%USERNAME%\Downloads\test.bin 268435456
```

(If using pure PowerShell, run cmd /c "fsutil file createnew C:\Users\$env:USERNAME\Downloads\test.bin 268435456").

Note: fsutil allocates space on storage without filling RAM. The file starts off completely uncached (Cold State).

## Step 2: Measure First Access (Cold Read / Cache Miss)

Open PowerShell and time how long it takes to read the newly created file from storage:

```PowerShell
Measure-Command { Get-Content -Path "C:\Users\$env:USERNAME\Downloads\test.bin" -ReadCount 0 }
```

Record the TotalSeconds value from the output.

During this initial read, Windows reads the file blocks directly from physical storage (SSD/HDD) and simultaneously stores a copy of those pages into RAM (the Standby List).

## Step 3: Measure Second Access (Warm Read / Cache Hit)

Run the exact same command a second time immediately afterward:

```PowerShell
Measure-Command { Get-Content -Path "C:\Users\$env:USERNAME\Downloads\test.bin" -ReadCount 0 }
```

Record the new TotalSeconds value.

## Expected Results & Observation

| **Access Attempt** | **Primary Storage Source** | **Execution Speed** |
|---|---|---|
| **First Access (Cold Read)** | Physical Disk (SSD/HDD) | Baseline (Slower) |
| **Second Access (Warm Read)** | System Memory (Windows Page Cache) | **10× to 50× Faster** |

## Why This Happens

1. **Cold Read (Cache Miss):** The first read is bottlenecked by physical drive access speeds and storage latency.

2. **Warm Read (Cache Hit):** The second read completes almost instantaneously because the Windows Cache Manager serves all 256 MB of data directly out of system RAM without reading from the disk.

## Cleanup

When finished, delete the test file to free up storage space:

```PowerShell
Remove-Item -Path "C:\Users\$env:USERNAME\Downloads\test.bin"
```
