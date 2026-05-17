@echo off
setlocal enabledelayedexpansion

echo ============================================
echo COM Smart Pointer Test (MSVC)
echo ============================================
echo.

echo Compiling ..	ests	est_com.cpp -I..include...
cl -nologo -W4 -EHsc -O2 -DUNICODE -D_UNICODE ..	ests	est_com.cpp -I..include
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
