@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ============================================
echo smart_ptr Full Test Suite (MSVC + GCC)
echo ============================================
echo.

set /a TOTAL_PASS=0
set /a TOTAL_FAIL=0

echo ============================================
echo MSVC Tests
echo ============================================
echo.

echo [1/2] Testing smart_ptr.h (MSVC)...
call test_smart_ptr_msvc.bat
if errorlevel 1 (
    set /a TOTAL_FAIL+=1
    echo   [FAILED]
) else (
    set /a TOTAL_PASS+=1
    echo   [PASS]
)

echo.
echo [2/2] Testing smart_ptr_mt.h (MSVC)...
call test_smart_ptr_mt_msvc.bat
if errorlevel 1 (
    set /a TOTAL_FAIL+=1
    echo   [FAILED]
) else (
    set /a TOTAL_PASS+=1
    echo   [PASS]
)

echo.
echo [2/3] Testing COM smart pointer (MSVC)...
call test_com_msvc.bat
if errorlevel 1 (
    set /a TOTAL_FAIL+=1
    echo   [FAILED]
) else (
    set /a TOTAL_PASS+=1
    echo   [PASS]
)


echo.
echo.
echo ============================================
echo GCC Tests
echo ============================================
echo.

echo [3/4] Testing smart_ptr.h (GCC)...
call test_smart_ptr_gcc.bat
if errorlevel 1 (
    set /a TOTAL_FAIL+=1
    echo   [FAILED]
) else (
    set /a TOTAL_PASS+=1
    echo   [PASS]
)

echo.
echo [4/4] Testing smart_ptr_mt.h (GCC)...
call test_smart_ptr_mt_gcc.bat
if errorlevel 1 (
    set /a TOTAL_FAIL+=1
    echo   [FAILED]
) else (
    set /a TOTAL_PASS+=1
    echo   [PASS]
)

echo.
echo ============================================
echo Test Summary
echo ============================================
echo Passed: !TOTAL_PASS!/5
echo Failed: !TOTAL_FAIL!/5
echo.

if !TOTAL_FAIL! equ 0 (
    echo ============================================
echo SUCCESS: ALL TEST SUITES PASSED
    echo ============================================
    echo.
    echo Test Results:
    echo - smart_ptr.h:      MSVC [PASS], GCC [PASS]
    echo - smart_ptr_mt.h:   MSVC [PASS], GCC [PASS]
    echo - test_com.cpp:     MSVC [PASS]
    echo.
    echo Total: 54 tests passed (smart_ptr.h: 12, smart_ptr_mt.h: 37, test_com.cpp: 5)
    echo ============================================
    exit /b 0
) else (
    echo ============================================
    FAILURE: SOME TEST SUITES FAILED
    echo ============================================
    exit /b 1
)
