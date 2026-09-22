#!/bin/bash

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root (sudo ./run_lottery.sh)"
  exit 1
fi

make || exit 1

echo "========================================="
echo " Installing 'lottery' Kernel Module"
echo "========================================="
insmod lottery.ko

sleep 0.5
if [ -e /dev/lottery ]; then
    chmod 666 /dev/lottery
else
    echo "Error: /dev/lottery was not created."
    rmmod lottery
    exit 1
fi

echo ""
echo "========================================="
echo " Starting Multi-Core Experiment (20 Tasks)"
echo " Target: lucas(47) across 20 processes"
echo " Ticket Distribution: 10 to 200 tickets"
echo "========================================="
echo ""

PIDS=()
TICKETS=(10 10 10 10 10 30 30 30 30 30 60 60 60 60 60 100 100 150 150 200)

echo "Spawning 20 processes..."

for i in "${!TICKETS[@]}"; do
    t_count=${TICKETS[$i]}
    ./app "$t_count" 47 &
    pid=$!
    PIDS+=("$pid")
    echo "  [Process $(printf "%02d" $((i+1)))] PID: $pid | Tickets: $t_count"
done

echo ""
echo "All 20 processes launched."
echo "Waiting 3 seconds for initial process registration to settle..."
sleep 3

# Clear registration logs so students only measure the steady-state execution
dmesg -c > /dev/null

echo "--------------------------------------------------------"
echo "Capturing Steady-State Load Balancing Metrics..."
echo "--------------------------------------------------------"

START_TIME=$(date +%s.%N)

wait "${PIDS[@]}"

END_TIME=$(date +%s.%N)
ELAPSED=$(echo "$END_TIME - $START_TIME" | bc 2>/dev/null || echo "N/A")

echo ""
echo "========================================="
echo " Total Steady-State Execution Time: ${ELAPSED}s"
echo "========================================="
echo ""

echo "========================================="
echo " Steady-State Queue Load Snapshots"
echo "========================================="
dmesg | grep "\[LOTTERY_STATUS\]"

echo ""
echo "========================================="
echo " Steady-State Migration Events"
echo "========================================="
MIGRATION_LOGS=$(dmesg | grep "Load Balance:")
if [ -z "$MIGRATION_LOGS" ]; then
    echo "No task migrations required during steady state (System in Equilibrium)."
else
    echo "$MIGRATION_LOGS"
    MIGRATION_COUNT=$(echo "$MIGRATION_LOGS" | wc -l)
    echo ""
    echo "Total Steady-State Migrations: $MIGRATION_COUNT"
fi

echo ""
echo "========================================="
echo " Cleaning Up Module"
echo "========================================="
rmmod lottery
echo "Done."
