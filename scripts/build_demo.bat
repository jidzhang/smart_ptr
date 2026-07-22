@echo off
setlocal enabledelayedexpansion

echo ============================================
echo Building demo.cpp (MSVC)
echo ============================================
echo.

REM --- Initialize MSVC environment (cl.exe / vcvars64.bat) ---
call "%~dp0_setup_msvc.bat"
if errorlevel 1 exit /b 1

REM --- Pin CWD to this script's folder so ..\tests and ..\include resolve ---
cd /d "%~dp0"

echo Compiling demo.cpp with cl.exe (C++98 standard)...
cl -nologo -W4 -EHsc -utf-8 -O2 -I..\include ..\demo.cpp
if errorlevel 1 (
    echo.
    echo [FAILED] Compilation failed
    exit /b 1
)

echo.
echo ============================================
echo Build successful!
echo Run: demo.exe
echo ============================================
"%CD%\demo.exe"
exit /b 0
