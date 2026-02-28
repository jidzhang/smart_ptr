@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ============================================
echo smart_ptr Test Script
echo ============================================

:: Build first if executables don't exist
if not exist demo.exe (
    echo Executables not found, building first...
    call "%CD%\build_msvc.bat"
    if errorlevel 1 exit /b 1
)

echo.
echo ----------------------------------------
echo Running demo.exe
echo ----------------------------------------
"%CD%\demo.exe"
if errorlevel 1 (
    echo Error: demo.exe failed with exit code %errorlevel%
    exit /b 1
)

echo.
echo ----------------------------------------
echo Running test_smart_ptr.exe (unit tests)
echo ----------------------------------------
if exist test_smart_ptr.exe (
    "%CD%\test_smart_ptr.exe"
    if errorlevel 1 (
        echo Error: Unit tests failed with exit code %errorlevel%
        exit /b 1
    )
) else (
    echo Skipping catch2 tests (compiler not supported)
)

echo.
echo ----------------------------------------
echo Running test_thread_safety.exe (thread safety)
echo ----------------------------------------
if exist test_thread_safety.exe (
    "%CD%\test_thread_safety.exe"
    if errorlevel 1 (
        echo Error: Thread safety tests failed with exit code %errorlevel%
        exit /b 1
    )
) else (
    echo Warning: test_thread_safety.exe not found, skipping
)

echo.
echo ----------------------------------------
echo Running test_com.exe (COM smart pointer)
echo ----------------------------------------
if exist test_com.exe (
    "%CD%\test_com.exe"
    if errorlevel 1 (
        echo Error: COM tests failed with exit code %errorlevel%
        exit /b 1
    )
) else (
    echo Warning: test_com.exe not found, skipping
)

echo.
echo ============================================
echo All tests passed!
echo ============================================

exit /b 0
