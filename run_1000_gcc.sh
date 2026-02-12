#!/bin/bash

echo "============================================"
echo "Running GCC tests 1000 times"
echo "============================================"
echo ""

PASS_COUNT=0
FAIL_COUNT=0

for i in {1..1000}; do
    echo "Run $i/1000..."
    ./test_gcc.bat > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "   [OK] Run $i"
        ((PASS_COUNT++))
    else
        echo "   [FAILED] Run $i"
        ((FAIL_COUNT++))
    fi
done

echo ""
echo "============================================"
echo "Results: $PASS_COUNT/1000 passed, $FAIL_COUNT/1000 failed"
echo "============================================"

if [ $FAIL_COUNT -eq 0 ]; then
    echo "SUCCESS: All 1000 runs passed!"
    exit 0
else
    echo "FAILURE: $FAIL_COUNT run(s) failed"
    exit 1
fi
