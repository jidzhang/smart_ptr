#!/bin/bash

cd "$(dirname "$0")"

echo "============================================"
echo "Running GCC tests 100 times"
echo "============================================"
echo ""

PASS_COUNT=0
FAIL_COUNT=0

for i in $(seq 1 100); do
    echo "Run $i/100..."
    ./test_all_gcc.sh > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "   [OK] Run $i"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        echo "   [FAILED] Run $i"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
done

echo ""
echo "============================================"
echo "Results: $PASS_COUNT/100 passed, $FAIL_COUNT/100 failed"
echo "============================================"

if [ $FAIL_COUNT -eq 0 ]; then
    echo "SUCCESS: All 100 runs passed!"
    exit 0
else
    echo "FAILURE: $FAIL_COUNT run(s) failed"
    exit 1
fi
