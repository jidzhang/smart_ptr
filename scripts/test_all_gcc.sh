#!/bin/bash

cd "$(dirname "$0")"

echo "============================================"
echo "smart_ptr Full Test Suite (GCC)"
echo "============================================"
echo ""

PASS=0
FAIL=0

# Detect platform
OS="$(uname -s)"
if [[ "$OS" == MINGW* || "$OS" == CYGWIN* || "$OS" == MSYS* ]]; then
    ON_WINDOWS=1
else
    ON_WINDOWS=0
fi

# Total number of suites (always 5; test_com is skipped on non-Windows)
TOTAL_SUITES=5

# --------------------------------------------
# GCC Tests
# --------------------------------------------
echo "=== GCC Tests ==="
echo ""

echo "[1/5] smart_ptr.h (GCC)..."
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

echo "[2/5] smart_ptr_mt.h (GCC)..."
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

if [ $ON_WINDOWS -eq 1 ]; then
    echo "[3/5] test_com.cpp (GCC)..."
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
else
    echo "[3/5] test_com.cpp (GCC)..."
    echo "  [SKIP] Windows-only (requires windows.h)"
fi

# --------------------------------------------
# Race Condition Stress Test
# --------------------------------------------
echo ""
echo "=== Race Condition ==="
echo ""

echo "[4/5] stress test (GCC)..."
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
# Thread Safety Stress Test
# --------------------------------------------
echo ""
echo "=== Thread Safety ==="
echo ""

echo "[5/5] thread safety (GCC)..."
g++ -O2 -std=c++11 -I../include -o test_thread_safety_gcc.exe ../tests/test_thread_safety.cpp -pthread > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "  [FAILED] GCC compile error"
    FAIL=$((FAIL+1))
else
    "$PWD/test_thread_safety_gcc.exe" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "  [FAILED] GCC thread safety test failed"
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
if [ $ON_WINDOWS -eq 1 ]; then
    echo "Results: $PASS/$TOTAL_SUITES suites passed, $FAIL/$TOTAL_SUITES failed"
else
    SKIPPED=$((TOTAL_SUITES - PASS - FAIL))
    echo "Results: $PASS/$TOTAL_SUITES suites passed, $FAIL/$TOTAL_SUITES failed ($SKIPPED skipped)"
fi
echo "============================================"

if [ $FAIL -eq 0 ]; then
    echo ""
    echo "  smart_ptr.h:        13 tests (GCC)"
    echo "  smart_ptr_mt.h:     38 tests (GCC)"
    if [ $ON_WINDOWS -eq 1 ]; then
        echo "  test_com.cpp:        5 tests (GCC)"
        echo "  race condition:      5 tests (GCC)"
        echo "  thread safety:       8 tests (GCC)"
        echo ""
        echo "  Total: 69 tests across 5 suites"
    else
        echo "  test_com.cpp:        SKIPPED (Windows-only)"
        echo "  race condition:      5 tests (GCC)"
        echo "  thread safety:       8 tests (GCC)"
        echo ""
        echo "  Total: 64 tests across 4 suites (1 skipped)"
    fi
    echo "============================================"
    exit 0
else
    exit 1
fi
