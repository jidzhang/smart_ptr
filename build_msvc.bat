@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ============================================
echo smart_ptr Build Script (MSVC)
echo ============================================

:: Setup VS2019 environment
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo Error: Failed to initialize VS2019 environment
    exit /b 1
)

echo.
echo Compiling demo.cpp (C++98 standard)...
cl -nologo -W4 -EHsc -O2 -DUNICODE -D_UNICODE demo.cpp
if errorlevel 1 (
    echo Error: demo.cpp compilation failed
    exit /b 1
)
echo demo.exe compiled successfully

echo.
echo Compiling test_smart_ptr.cpp (C++11 standard, for catch2)...
cl -nologo -W4 -EHsc -O2 -DUNICODE -D_UNICODE test_smart_ptr.cpp
if errorlevel 1 (
    echo Error: test_smart_ptr.cpp compilation failed
    exit /b 1
)
echo test_smart_ptr.exe compiled successfully

echo.
echo Compiling test_thread_safety.cpp (multi-threaded stress test)...
cl -nologo -W4 -EHsc test_thread_safety.cpp
if errorlevel 1 (
    echo Error: test_thread_safety.cpp compilation failed
    exit /b 1
)
echo test_thread_safety.exe compiled successfully

echo.
echo ============================================
echo All files compiled successfully!
echo ============================================

exit /b 0
