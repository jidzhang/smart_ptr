#!/bin/bash

echo "========================================"
echo "Testing smart_ptr.h (GCC)"
echo "========================================"
echo ""

echo "[1/2] Compiling test_comprehensive.cpp..."
g++ -std=c++11 -Wall -O2 -I../include -o test_comprehensive_gcc.exe ../tests/test_comprehensive.cpp
if [ $? -ne 0 ]; then
    echo "[FAILED] Compilation failed"
    exit 1
fi
echo "      OK"

echo ""
echo "[2/2] Running tests..."
echo ""
"$PWD/test_comprehensive_gcc.exe"
if [ $? -ne 0 ]; then
    echo ""
    echo "[FAILED] Tests failed"
    exit 1
fi

echo ""
echo "========================================"
echo "ALL TESTS PASSED"
echo "========================================"
exit 0
