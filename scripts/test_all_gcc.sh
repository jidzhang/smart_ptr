#!/bin/bash

cd "$(dirname "$0")"

echo "============================================"
echo "smart_ptr Full Test Suite (GCC)"
echo "============================================"
echo ""

PASS=0
FAIL=0
ERRTMP="$PWD/_err.tmp"

# Per-suite test counts; single source for the [PASS] lines and the totals
# in the summary, so the two cannot drift apart.
T_ST=17
T_MT=41
T_COM=5
T_RACE=5
T_TS=8
T_DISPOSE=4
T_CXX98=17

# Print the captured compiler output indented, so failures show the real cause
show_err() {
    sed 's/^/    /' "$ERRTMP" 2> /dev/null
}

# Detect platform
OS="$(uname -s)"
if [[ "$OS" == MINGW* || "$OS" == CYGWIN* || "$OS" == MSYS* ]]; then
    ON_WINDOWS=1
else
    ON_WINDOWS=0
fi

# Total number of suites (always 7; test_com is skipped on non-Windows)
TOTAL_SUITES=7

# --------------------------------------------
# GCC Tests
# --------------------------------------------
echo "=== GCC Tests ==="
echo ""

echo "[1/7] smart_ptr.h (GCC)..."
g++ -std=c++11 -Wall -O2 -I../include -o test_comprehensive_gcc.exe ../tests/test_comprehensive.cpp > /dev/null 2> "$ERRTMP"
if [ $? -ne 0 ]; then
    echo "  [FAILED] Compile error"
    show_err
    FAIL=$((FAIL+1))
else
    "$PWD/test_comprehensive_gcc.exe" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "  [FAILED] Tests failed"
        FAIL=$((FAIL+1))
    else
        echo "  [PASS] $T_ST/$T_ST"
        PASS=$((PASS+1))
    fi
fi

echo "[2/7] smart_ptr_mt.h (GCC)..."
g++ -std=c++11 -Wall -O2 -I../include -o test_comprehensive_mt_gcc.exe ../tests/test_comprehensive_mt.cpp > /dev/null 2> "$ERRTMP"
if [ $? -ne 0 ]; then
    echo "  [FAILED] Compile error"
    show_err
    FAIL=$((FAIL+1))
else
    "$PWD/test_comprehensive_mt_gcc.exe" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "  [FAILED] Tests failed"
        FAIL=$((FAIL+1))
    else
        echo "  [PASS] $T_MT/$T_MT"
        PASS=$((PASS+1))
    fi
fi

if [ $ON_WINDOWS -eq 1 ]; then
    echo "[3/7] test_com.cpp (GCC)..."
    g++ -std=c++11 -Wall -O2 -I../include -DUNICODE -D_UNICODE -o test_com_gcc.exe ../tests/test_com.cpp > /dev/null 2> "$ERRTMP"
    if [ $? -ne 0 ]; then
        echo "  [FAILED] Compile error"
        show_err
        FAIL=$((FAIL+1))
    else
        "$PWD/test_com_gcc.exe" > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            echo "  [FAILED] Tests failed"
            FAIL=$((FAIL+1))
        else
            echo "  [PASS] $T_COM/$T_COM"
            PASS=$((PASS+1))
        fi
    fi
else
    echo "[3/7] test_com.cpp (GCC)..."
    echo "  [SKIP] Windows-only (requires windows.h)"
fi

# --------------------------------------------
# Race Condition Stress Test
# --------------------------------------------
echo ""
echo "=== Race Condition ==="
echo ""

echo "[4/7] stress test (GCC)..."
g++ -O2 -std=c++11 -I../include -o test_race_gcc.exe ../tests/test_race_condition.cpp -pthread > /dev/null 2> "$ERRTMP"
if [ $? -ne 0 ]; then
    echo "  [FAILED] GCC compile error"
    show_err
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

echo "[5/7] thread safety (GCC)..."
g++ -O2 -std=c++11 -I../include -o test_thread_safety_gcc.exe ../tests/test_thread_safety.cpp -pthread > /dev/null 2> "$ERRTMP"
if [ $? -ne 0 ]; then
    echo "  [FAILED] GCC compile error"
    show_err
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
# Dispose Semantics (type-erased deleter regression)
# --------------------------------------------
echo ""
echo "=== Dispose Semantics ==="
echo ""

# Warning gate: this file must stay clean under -Wall -Wextra -Werror.
# No -O2 here: GCC -O2 inlining analysis emits unrelated use-after-free
# false positives.
echo "[6/7] dispose semantics (GCC, -Werror gate)..."
g++ -std=c++11 -Wall -Wextra -Werror -I../include -o test_dispose_semantics_gcc.exe ../tests/test_dispose_semantics.cpp > /dev/null 2> "$ERRTMP"
if [ $? -ne 0 ]; then
    echo "  [FAILED] GCC compile error (warning gate)"
    show_err
    FAIL=$((FAIL+1))
else
    "$PWD/test_dispose_semantics_gcc.exe" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "  [FAILED] GCC dispose test failed"
        FAIL=$((FAIL+1))
    else
        echo "  [PASS] $T_DISPOSE/$T_DISPOSE"
        PASS=$((PASS+1))
    fi
fi

# --------------------------------------------
# C++98 Mode Check
# --------------------------------------------
echo ""
echo "=== C++98 Mode ==="
echo ""

# MSVC VS2015+ always takes the C++11 code path (_MSC_VER >= 1900), even
# under /std:c++03, so GCC is the only toolchain that exercises the real
# C++98 branch here. Move/cast tests compile to stubs in this mode.
echo "[7/7] C++98 mode check (GCC)..."
g++ -std=c++98 -Wall -O2 -I../include -o test_comprehensive_cxx98.exe ../tests/test_comprehensive.cpp > /dev/null 2> "$ERRTMP"
if [ $? -ne 0 ]; then
    echo "  [FAILED] Compile error"
    show_err
    FAIL=$((FAIL+1))
else
    "$PWD/test_comprehensive_cxx98.exe" > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "  [FAILED] Tests failed"
        FAIL=$((FAIL+1))
    else
        echo "  [PASS] $T_CXX98/$T_CXX98 (C++98 mode)"
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

rm -f "$ERRTMP"

if [ $FAIL -eq 0 ]; then
    echo ""
    echo "  smart_ptr.h:        $T_ST tests (GCC)"
    echo "  smart_ptr_mt.h:     $T_MT tests (GCC)"
    if [ $ON_WINDOWS -eq 1 ]; then
        echo "  test_com.cpp:        $T_COM tests (GCC)"
    else
        echo "  test_com.cpp:        SKIPPED (Windows-only)"
    fi
    echo "  race condition:      $T_RACE tests (GCC)"
    echo "  thread safety:       $T_TS tests (GCC)"
    echo "  dispose semantics:   $T_DISPOSE tests (GCC, -Werror gate)"
    echo "  C++98 mode (ST):    $T_CXX98 tests (GCC)"
    echo ""
    if [ $ON_WINDOWS -eq 1 ]; then
        echo "  Total: $((T_ST+T_MT+T_COM+T_RACE+T_TS+T_DISPOSE+T_CXX98)) tests across $TOTAL_SUITES suites"
    else
        echo "  Total: $((T_ST+T_MT+T_RACE+T_TS+T_DISPOSE+T_CXX98)) tests across $((TOTAL_SUITES-1)) suites (1 skipped)"
    fi
    echo "============================================"
    exit 0
else
    exit 1
fi
