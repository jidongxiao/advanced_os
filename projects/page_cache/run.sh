#!/bin/bash

set -e

MODULE="pagecacheTest.ko"
MODULE_NAME="pagecacheTest"
PROC_ENTRY="/proc/pagecache_test"
OUTPUT="readFile.out"

READ_PID=""

cleanup()
{
    echo
    echo "========================================"
    echo "Cleaning up"
    echo "========================================"

    if [ -n "$READ_PID" ] && kill -0 "$READ_PID" 2>/dev/null; then
        echo "Stopping readFile (PID $READ_PID)..."

        kill -TERM "$READ_PID" 2>/dev/null || true
        sleep 0.2

        if kill -0 "$READ_PID" 2>/dev/null; then
            echo "readFile did not exit; forcing termination..."
            kill -KILL "$READ_PID" 2>/dev/null || true
        fi

        wait "$READ_PID" 2>/dev/null || true
    fi

    if lsmod | grep -q "^${MODULE_NAME} "; then
        echo "Removing $MODULE_NAME..."
        sudo rmmod "$MODULE_NAME"
    fi

    rm -f "$OUTPUT"
}

trap cleanup EXIT

echo "========================================"
echo "Cleaning up previous module"
echo "========================================"

if lsmod | grep -q "^${MODULE_NAME} "; then
    echo "Removing already-loaded $MODULE_NAME..."
    sudo rmmod "$MODULE_NAME"
fi

rm -f "$OUTPUT"

echo
echo "========================================"
echo "Building the experiment"
echo "========================================"

make

echo
echo "========================================"
echo "Loading kernel module"
echo "========================================"

sudo insmod "$MODULE"

echo
echo "========================================"
echo "Running readFile"
echo "========================================"

./readFile > "$OUTPUT" 2>&1 &
READ_PID=$!

echo "readFile process started: PID $READ_PID"

#
# Wait until readFile has printed the user buffer address.
#
while true; do

    if grep -q "User buffer virtual address:" "$OUTPUT"; then
        break
    fi

    if ! kill -0 "$READ_PID" 2>/dev/null; then
        echo
        echo "ERROR: readFile terminated unexpectedly."
        echo
        cat "$OUTPUT"
        exit 1
    fi

    sleep 0.1
done

cat "$OUTPUT"

#
# Extract PID and user-space virtual address.
#
PID=$(grep "^PID:" "$OUTPUT" | awk '{print $2}')
ADDRESS=$(grep "^User buffer virtual address:" "$OUTPUT" | awk '{print $5}')

echo
echo "========================================"
echo "Verifying readFile"
echo "========================================"

if ! kill -0 "$PID" 2>/dev/null; then
    echo "ERROR: readFile process $PID is no longer running."
    exit 1
fi

ps -p "$PID" -o pid,ppid,state,comm

echo
echo "readFile PID:     $PID"
echo "User buffer:      $ADDRESS"

echo
echo "========================================"
echo "Running kernel-module test"
echo "========================================"

if [ ! -e "$PROC_ENTRY" ]; then
    echo "ERROR: $PROC_ENTRY does not exist."
    exit 1
fi

echo "$PID $ADDRESS data.bin" | sudo tee "$PROC_ENTRY" > /dev/null

echo
echo "========================================"
echo "Kernel output"
echo "========================================"

sudo dmesg | tail -n 80

echo
echo "========================================"
echo "Experiment complete"
echo "========================================"
