#!/bin/bash

echo "============================================"
echo "smart_ptr Full Test Suite (GCC)"
echo "============================================"
echo ""

PASS=0
FAIL=0

# --------------------------------------------
# GCC Tests
# --------------------------------------------
echo "=== GCC Tests ==="
echo ""

echo "[1/4] smart_ptr.h (GCC)..."
g++ -std=c++11 -Wall -O2 -I../include -o test_comprehensive_gcc.exe ../tests/test_comprehensive.cpp > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "  [FAILED] Compile error"
    FAIL=$((FAIL+1))
else
    "$PWD/test_comprehensive_gcc.exe" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "  [FAILED] Tests failed"
        FAIL=$((FAIL+1))
    else
        echo "  [PASS] 13/13"
        PASS=$((PASS+1))
    fi
fi

echo "[2/4] smart_ptr_mt.h (GCC)..."
g++ -std=c++11 -Wall -O2 -I../include -o test_comprehensive_mt_gcc.exe ../tests/test_comprehensive_mt.cpp > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "  [FAILED] Compile error"
    FAIL=$((FAIL+1))
else
    "$PWD/test_comprehensive_mt_gcc.exe" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "  [FAILED] Tests failed"
        FAIL=$((FAIL+1))
    else
        echo "  [PASS] 38/38"
        PASS=$((PASS+1))
    fi
fi

echo "[3/4] test_com.cpp (GCC)..."
g++ -std=c++11 -Wall -O2 -I../include -DUNICODE -D_UNICODE -o test_com_gcc.exe ../tests/test_com.cpp > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "  [FAILED] Compile error"
    FAIL=$((FAIL+1))
else
    "$PWD/test_com_gcc.exe" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "  [FAILED] Tests failed"
        FAIL=$((FAIL+1))
    else
        echo "  [PASS] 5/5"
        PASS=$((PASS+1))
    fi
fi

# --------------------------------------------
# Race Condition Stress Test
# --------------------------------------------
echo ""
echo "=== Race Condition ==="
echo ""

echo "[4/4] stress test (GCC)..."
g++ -O2 -std=c++11 -I../include -o test_race_gcc.exe ../tests/test_race_condition.cpp -pthread > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "  [FAILED] GCC compile error"
    FAIL=$((FAIL+1))
else
    "$PWD/test_race_gcc.exe" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "  [FAILED] GCC race test failed"
        FAIL=$((FAIL+1))
    else
        echo "  [PASS] GCC"
        PASS=$((PASS+1))
    fi
fi

# --------------------------------------------
# Summary
# --------------------------------------------
echo ""
echo "============================================"
echo "Results: $PASS/4 suites passed, $FAIL/4 failed"
echo "============================================"

if [ $FAIL -eq 0 ]; then
    echo ""
    echo "  smart_ptr.h:     13 tests (GCC)"
    echo "  smart_ptr_mt.h:  38 tests (GCC)"
    echo "  test_com.cpp:     5 tests (GCC)"
    echo "  race condition:   5 tests (GCC)"
    echo ""
    echo "  Total: 61 tests across 4 suites"
    echo "============================================"
    exit 0
else
    exit 1
fi
