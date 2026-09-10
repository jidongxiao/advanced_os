#!/bin/bash
# cpu_stress.sh
# A simple CPU-intensive script to test 'nice'

# Default runtime in seconds
RUNTIME=${1:-100}

# echo "Starting CPU-intensive task for $RUNTIME seconds..."
START=$(date +%s)

while true; do
  # Simple calculation loop to consume CPU
  for i in {1..1000000}; do
    x=$((i*i % 12345))
  done

  NOW=$(date +%s)
  ELAPSED=$((NOW - START))
  if [ "$ELAPSED" -ge "$RUNTIME" ]; then
    break
  fi
done

echo "Done! Ran for $RUNTIME seconds."
