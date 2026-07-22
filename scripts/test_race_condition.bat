@echo off
setlocal enabledelayedexpansion

echo ============================================
echo smart_ptr_mt Race Condition Stress Test
echo ============================================
echo.

REM --- Initialize MSVC environment (cl.exe / vcvars64.bat) ---
call "%~dp0_setup_msvc.bat"
if errorlevel 1 exit /b 1

REM --- Pin CWD to this script's folder so ..\tests and ..\include resolve ---
cd /d "%~dp0"

echo [MSVC] Compiling...
cl -nologo -W4 -EHsc -utf-8 -O2 ..\tests\test_race_condition.cpp -I..\include -Fetest_race_msvc.exe
if errorlevel 1 (
    echo [FAILED] MSVC compilation failed
    exit /b 1
)
echo       OK

echo [MSVC] Running...
echo.
"%CD%\test_race_msvc.exe"
if errorlevel 1 (
    echo.
    echo [FAILED] MSVC test failed
    exit /b 1
)

echo.
echo [GCC] Compiling...
g++ -O2 -std=c++11 -o test_race_gcc.exe ..\tests\test_race_condition.cpp -I..\include -pthread
if errorlevel 1 (
    echo [FAILED] GCC compilation failed
    exit /b 1
)
echo       OK

echo [GCC] Running...
echo.
"%CD%\test_race_gcc.exe"
if errorlevel 1 (
    echo.
    echo [FAILED] GCC test failed
    exit /b 1
)

echo.
echo ============================================
echo ALL RACE CONDITION TESTS PASSED (MSVC+GCC)
echo ============================================
exit /b 0