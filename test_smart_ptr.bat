@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Testing smart_ptr.h (MSVC)
echo ========================================
echo.

echo [1/2] Compiling test_comprehensive.cpp...
cl -nologo -W4 -EHsc -utf-8 -O2 -Fe:test_comprehensive_msvc.exe test_comprehensive.cpp
if errorlevel 1 (
    echo [FAILED] Compilation failed
    exit /b 1
)
echo       OK

echo.
echo [2/2] Running tests...
echo.
test_comprehensive_msvc.exe
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
