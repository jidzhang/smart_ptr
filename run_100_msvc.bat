@echo off
setlocal enabledelayedexpansion

echo ============================================
echo Running MSVC tests 100 times
echo Includes: demo.exe, test_smart_ptr.exe, test_thread_safety.exe
echo ============================================
echo.

set /a PASS_COUNT=0
set /a FAIL_COUNT=0

for /L %%i in (1,1,100) do (
    echo Run %%i/100...
    call .\test_run.bat >nul 2>&1
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
echo Results: !PASS_COUNT!/100 passed, !FAIL_COUNT!/100 failed
echo ============================================

if !FAIL_COUNT! EQU 0 (
    echo SUCCESS: All 100 runs passed!
) else (
    echo FAILURE: !FAIL_COUNT! run(s) failed
)

REM Dummy label to prevent "failed was unexpected" error
:end_label
exit /b 0