@echo off
setlocal enabledelayedexpansion

echo ============================================
echo smart_ptr Full Test Suite
echo ============================================
echo.

REM --- Initialize MSVC environment (cl.exe / vcvars64.bat) ---
call "%~dp0_setup_msvc.bat"
if errorlevel 1 exit /b 1

REM --- Pin CWD to this script's folder so ..\tests and ..\include resolve ---
cd /d "%~dp0"

set /a PASS=0
set /a FAIL=0

:: --------------------------------------------
:: MSVC Tests
:: --------------------------------------------
echo === MSVC Tests ===
echo.

echo [1/8] smart_ptr.h (MSVC)...
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
        echo   [PASS] 16/16
        set /a PASS+=1
    )
)

echo [2/8] smart_ptr_mt.h (MSVC)...
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
        echo   [PASS] 40/40
        set /a PASS+=1
    )
)

echo [3/8] test_com.cpp (MSVC)...
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

echo [4/8] smart_ptr.h (GCC)...
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
        echo   [PASS] 16/16
        set /a PASS+=1
    )
)

echo [5/8] smart_ptr_mt.h (GCC)...
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
        echo   [PASS] 40/40
        set /a PASS+=1
    )
)

:: --------------------------------------------
:: Race Condition Stress Test
:: --------------------------------------------
echo.
echo === Race Condition ===
echo.

echo [6/8] stress test (MSVC + GCC)...
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
:: Thread Safety Stress Test
:: --------------------------------------------
echo.
echo === Thread Safety ===
echo.

echo [7/8] thread safety (MSVC + GCC)...
set /a TS_PASS=0

cl -nologo -W4 -EHsc -utf-8 -O2 ..\tests\test_thread_safety.cpp -I..\include -Fetest_thread_safety_msvc.exe >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] MSVC compile error
    set /a FAIL+=1
    goto :ts_done
)

"%CD%\test_thread_safety_msvc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] MSVC thread safety test failed
    set /a FAIL+=1
    goto :ts_done
)
set /a TS_PASS+=1

g++ -O2 -std=c++11 -o test_thread_safety_gcc.exe ../tests/test_thread_safety.cpp -I../include -pthread >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] GCC compile error
    set /a FAIL+=1
    goto :ts_done
)

"%CD%\test_thread_safety_gcc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] GCC thread safety test failed
    set /a FAIL+=1
    goto :ts_done
)
set /a TS_PASS+=1

echo   [PASS] MSVC + GCC
set /a PASS+=1
goto :ts_end

:ts_done
echo   MSVC !TS_PASS!/2, GCC !TS_PASS!/2

:ts_end

:: --------------------------------------------
:: Unit Test (Catch2 framework, C++11)
:: --------------------------------------------
echo.
echo === Unit Test (Catch2) ===
echo.

echo [8/8] smart_ptr unit test (MSVC + GCC)...
set /a UT_PASS=0

cl -nologo -W4 -EHsc -utf-8 -O2 ..\tests\test_smart_ptr.cpp -I..\include -Fetest_smart_ptr_msvc.exe >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] MSVC compile error
    set /a FAIL+=1
    goto :ut_done
)

"%CD%\test_smart_ptr_msvc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] MSVC unit test failed
    set /a FAIL+=1
    goto :ut_done
)
set /a UT_PASS+=1

g++ -std=c++11 -Wall -O2 -o test_smart_ptr_gcc.exe ../tests/test_smart_ptr.cpp -I../include >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] GCC compile error
    set /a FAIL+=1
    goto :ut_done
)

"%CD%\test_smart_ptr_gcc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] GCC unit test failed
    set /a FAIL+=1
    goto :ut_done
)
set /a UT_PASS+=1

echo   [PASS] MSVC + GCC
set /a PASS+=1
goto :ut_end

:ut_done
echo   MSVC !UT_PASS!/2, GCC !UT_PASS!/2

:ut_end

:: --------------------------------------------
:: Summary
:: --------------------------------------------
echo.
echo ============================================
echo Results: !PASS!/8 suites passed, !FAIL!/8 failed
echo ============================================

if !FAIL! equ 0 (
    echo.
    echo   smart_ptr.h:     16 tests (MSVC + GCC^)
    echo   smart_ptr_mt.h:  40 tests (MSVC + GCC^)
    echo   test_com.cpp:     5 tests (MSVC^)
    echo   race condition:   5 tests x2 (MSVC + GCC^)
    echo   thread safety:   8 tests x2 (MSVC + GCC^)
    echo   smart_ptr (Catch2^): 39 tests x2 (MSVC + GCC^)
    echo.
    echo   Total: 221 tests across 8 suites
    echo ============================================
    exit /b 0
) else (
    exit /b 1
)
