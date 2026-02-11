@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ============================================
echo smart_ptr Test Script
echo ============================================

:: Build first if executables don't exist
if not exist demo.exe (
    echo Executables not found, building first...
    call build.bat
    if errorlevel 1 exit /b 1
)

echo.
echo ----------------------------------------
echo Running demo.exe
echo ----------------------------------------
demo.exe
if errorlevel 1 (
    echo Error: demo.exe failed
    exit /b 1
)

echo.
echo ----------------------------------------
echo Running test_smart_ptr.exe (unit tests)
echo ----------------------------------------
test_smart_ptr.exe
if errorlevel 1 (
    echo Error: Unit tests failed
    exit /b 1
)

echo.
echo ----------------------------------------
echo Running test_thread_safety.exe (thread safety)
echo ----------------------------------------
test_thread_safety.exe
if errorlevel 1 (
    echo Error: Thread safety tests failed
    exit /b 1
)

echo.
echo ============================================
echo All tests passed!
echo ============================================

exit /b 0
