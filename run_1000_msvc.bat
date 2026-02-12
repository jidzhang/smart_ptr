@echo off
setlocal enabledelayedexpansion

echo ============================================
echo Running MSVC tests 1000 times
echo ============================================
echo.

set /a PASS_COUNT=0
set /a FAIL_COUNT=0

for /L %%i in (1,1,1000) do (
    echo Run %%i/1000...
    call "%CD%\test_run.bat" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Run %%i
        set /a FAIL_COUNT+=1
    ) else (
        echo   [OK] Run %%i
        set /a PASS_COUNT+=1
    )
)

echo.
echo ============================================
echo Results: !PASS_COUNT!/1000 passed, !FAIL_COUNT!/1000 failed
echo ============================================

if !FAIL_COUNT! EQU 0 (
    echo SUCCESS: All 1000 runs passed!
) else (
    echo FAILURE: !FAIL_COUNT! run(s) failed
)
exit /b !FAIL_COUNT!
