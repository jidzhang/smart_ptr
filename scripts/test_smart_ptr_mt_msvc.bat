@echo off
setlocal enabledelayedexpansion

echo ========================================
echo Testing smart_ptr_mt.h with MSVC
echo ========================================
echo.

echo [1/2] Compiling ..	ests	est_comprehensive_mt.cpp -I..include...
cl -nologo -W4 -EHsc -utf-8 -O2 -Fetest_comprehensive_mt_msvc.exe ..	ests	est_comprehensive_mt.cpp -I..include
if errorlevel 1 (
    echo [FAILED] Compilation failed
    exit /b 1
)
echo       OK

echo.
echo [2/2] Running tests...
echo.
"%CD%\test_comprehensive_mt_msvc.exe"
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
