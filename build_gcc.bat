@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ============================================
echo smart_ptr Build Script (GCC/g++)
echo ============================================

:: Check if g++ is available
where g++ >nul 2>&1
if errorlevel 1 (
    echo Error: g++ not found in PATH
    echo Please install MinGW-w64 or add GCC to your PATH
    exit /b 1
)

echo.
echo Compiling demo.cpp (C++98 standard)...
g++ -Wall -Wextra -O2 -std=c++98 -o demo.exe demo.cpp
if errorlevel 1 (
    echo Error: demo.cpp compilation failed
    exit /b 1
)
echo demo.exe compiled successfully

echo.
echo Compiling test_smart_ptr.cpp (C++11 standard, for catch2)...
g++ -Wall -Wextra -O2 -std=c++11 -o test_smart_ptr.exe test_smart_ptr.cpp
if errorlevel 1 (
    echo Error: test_smart_ptr.cpp compilation failed
    exit /b 1
)
echo test_smart_ptr.exe compiled successfully

echo.
echo Compiling test_thread_safety.cpp (multi-threaded stress test)...
g++ -Wall -Wextra -O2 -std=c++11 -pthread -o test_thread_safety.exe test_thread_safety.cpp
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
