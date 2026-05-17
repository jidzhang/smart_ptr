@echo off
setlocal enabledelayedexpansion

echo ============================================
echo smart_ptr Full Test Suite
echo ============================================
echo.

set /a PASS=0
set /a FAIL=0

:: --------------------------------------------
:: MSVC Tests
:: --------------------------------------------
echo === MSVC Tests ===
echo.

echo [1/6] smart_ptr.h (MSVC)...
cl -nologo -W4 -EHsc -utf-8 -O2 ..\tests\test_comprehensive.cpp -I..\include -Fetest_comprehensive_msvc.exe >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] Compile error
    set /a FAIL+=1
) else (
    "%CD%\test_comprehensive_msvc.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 13/13
        set /a PASS+=1
    )
)

echo [2/6] smart_ptr_mt.h (MSVC)...
cl -nologo -W4 -EHsc -utf-8 -O2 -Fetest_comprehensive_mt_msvc.exe ..\tests\test_comprehensive_mt.cpp -I..\include >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] Compile error
    set /a FAIL+=1
) else (
    "%CD%\test_comprehensive_mt_msvc.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 38/38
        set /a PASS+=1
    )
)

echo [3/6] test_com.cpp (MSVC)...
cl -nologo -W4 -EHsc -O2 -DUNICODE -D_UNICODE ..\tests\test_com.cpp -I..\include >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] Compile error
    set /a FAIL+=1
) else (
    "%CD%\test_com.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 5/5
        set /a PASS+=1
    )
)

:: --------------------------------------------
:: GCC Tests
:: --------------------------------------------
echo.
echo === GCC Tests ===
echo.

echo [4/6] smart_ptr.h (GCC)...
g++ -std=c++11 -Wall -O2 -o test_comprehensive_gcc.exe ../tests/test_comprehensive.cpp -I../include >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] Compile error
    set /a FAIL+=1
) else (
    "%CD%\test_comprehensive_gcc.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 13/13
        set /a PASS+=1
    )
)

echo [5/6] smart_ptr_mt.h (GCC)...
g++ -std=c++11 -Wall -O2 -o test_comprehensive_mt_gcc.exe ../tests/test_comprehensive_mt.cpp -I../include >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] Compile error
    set /a FAIL+=1
) else (
    "%CD%\test_comprehensive_mt_gcc.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 38/38
        set /a PASS+=1
    )
)

:: --------------------------------------------
:: Race Condition Stress Test
:: --------------------------------------------
echo.
echo === Race Condition ===
echo.

echo [6/6] stress test (MSVC + GCC)...
set /a RACE_PASS=0

cl -nologo -W4 -EHsc -utf-8 -O2 ..\tests\test_race_condition.cpp -I..\include -Fetest_race_msvc.exe >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] MSVC compile error
    set /a FAIL+=1
    goto :race_done
)

"%CD%\test_race_msvc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] MSVC race test failed
    set /a FAIL+=1
    goto :race_done
)
set /a RACE_PASS+=1

g++ -O2 -std=c++11 -o test_race_gcc.exe ../tests/test_race_condition.cpp -I../include -pthread >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] GCC compile error
    set /a FAIL+=1
    goto :race_done
)

"%CD%\test_race_gcc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] GCC race test failed
    set /a FAIL+=1
    goto :race_done
)
set /a RACE_PASS+=1

echo   [PASS] MSVC + GCC
set /a PASS+=1
goto :race_end

:race_done
echo   MSVC !RACE_PASS!/2, GCC !RACE_PASS!/2

:race_end

:: --------------------------------------------
:: Summary
:: --------------------------------------------
echo.
echo ============================================
echo Results: !PASS!/6 suites passed, !FAIL!/6 failed
echo ============================================

if !FAIL! equ 0 (
    echo.
    echo   smart_ptr.h:     13 tests (MSVC + GCC^)
    echo   smart_ptr_mt.h:  38 tests (MSVC + GCC^)
    echo   test_com.cpp:     5 tests (MSVC^)
    echo   race condition:   5 tests x2 (MSVC + GCC^)
    echo.
    echo   Total: 114 tests across 6 suites
    echo ============================================
    exit /b 0
) else (
    exit /b 1
)
