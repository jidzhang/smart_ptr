@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Testing smart_ptr.h (GCC)
echo ========================================
echo.

echo [1/2] Compiling test_comprehensive.cpp...
g++ -std=c++98 -Wall -O2 -o test_comprehensive.exe test_comprehensive.cpp
if errorlevel 1 (
    echo [FAILED] Compilation failed
    exit /b 1
)
echo       OK

echo.
echo [2/2] Running tests...
echo.
test_comprehensive.exe
if errorlevel 1 (
    echo.
    echo [FAILED] Tests failed
    exit /b 1
)

echo.
echo ========================================
echo ALL TESTS PASSED
echo ========================================
exit /b 0

