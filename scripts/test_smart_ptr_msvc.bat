@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Testing smart_ptr.h (MSVC)
echo ========================================
echo.

echo [1/2] Compiling ..	ests	est_comprehensive.cpp -I..include...
cl -nologo -W4 -EHsc -utf-8 -O2 ..	ests	est_comprehensive.cpp -I..include -Fetest_comprehensive_msvc.exe
if errorlevel 1 (
    echo [FAILED] Compilation failed
    exit /b 1
)
echo       OK

echo.
echo [2/2] Running tests...
echo.
"%CD%\test_comprehensive_msvc.exe"
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
