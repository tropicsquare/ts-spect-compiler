#!/bin/bash

# Check if the correct number of arguments are provided
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 <program> <min_expected_time> <max_expected_time>"
    exit 1
fi

# Change timeformat to print elapsed time in s
export LC_NUMERIC="en_US.UTF-8"
TIMEFORMAT=%R

# Assign arguments to variables
PROGRAM="$1"
EXPECTED_TIME_MIN="$2"
EXPECTED_TIME_MAX="$3"

echo "*************************************************************************"
echo "* Running test $PROGRAM"
echo "*************************************************************************"
ELAPSED_TIME=$( { time $TS_REPO_ROOT/build/src/apps/spect_iss --program=$PROGRAM --timing-accurate --execution-time-step=200 > /dev/null; } 2>&1 )

echo "*************************************************************************"
echo "* Checking duration"
echo "*************************************************************************"
echo "Expected time: [${EXPECTED_TIME_MIN}ns : ${EXPECTED_TIME_MAX}ns]"
echo "Elapsed time : ${ELAPSED_TIME}ns"

if (( $(echo "$ELAPSED_TIME > $EXPECTED_TIME_MIN && $ELAPSED_TIME < $EXPECTED_TIME_MAX" | bc -l) )); then
  echo "Elapsed time matches expected time"
  exit 0
else
  echo "Elapsed time does not match expected time"
  exit 1
fi
