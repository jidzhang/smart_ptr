@echo off
setlocal enabledelayedexpansion

echo ============================================
echo Building demo.cpp (MSVC)
echo ============================================
echo.

echo Compiling demo.cpp with cl.exe (C++98 standard)...
cl -nologo -W4 -EHsc -utf-8 -O2 demo.cpp
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
