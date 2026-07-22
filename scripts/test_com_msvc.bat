@echo off
setlocal enabledelayedexpansion

echo ============================================
echo COM Smart Pointer Test (MSVC)
echo ============================================
echo.

REM --- Initialize MSVC environment (cl.exe / vcvars64.bat) ---
call "%~dp0_setup_msvc.bat"
if errorlevel 1 exit /b 1

REM --- Pin CWD to this script's folder so ..\tests and ..\include resolve ---
cd /d "%~dp0"

echo Compiling ..\tests\test_com.cpp -I..\include...
cl -nologo -W4 -EHsc -O2 -DUNICODE -D_UNICODE ..\tests\test_com.cpp -I..\include
if errorlevel 1 (
    echo [FAILED] Compilation failed
    exit /b 1
)
echo       OK

echo.
echo Running tests...
"%CD%\test_com.exe"
if errorlevel 1 (
    echo [FAILED] Tests failed
    exit /b 1
)

echo.
echo ============================================
echo ALL TESTS PASSED
echo ============================================
exit /b 0