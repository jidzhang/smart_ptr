@echo off
REM ============================================================
REM  test_matrix.bat - Run the smart_ptr suite against every
REM                     compiler _detect_compilers.bat finds
REM ============================================================
REM  Reads %TEMP%\_smart_ptr_compilers.lst (one record per line,
REM  LABEL;KIND;ARCH;TIER;SETUP;EXE) and runs the appropriate
REM  test set against each. No hardcoded pass counts -- each test
REM  binary is its own oracle (exits non-zero on failure).
REM
REM  Test set by tier:
REM    cpp98 (VS2005/2008/2010) -> demo only (no C++11 in those)
REM    full  (VS2012+, Clang, GCC) -> demo + comprehensive +
REM                                   comprehensive_mt + race + com
REM
REM  Env isolation: every MSVC/Clang target runs inside its own
REM  setlocal/endlocal so one vcvarsall cannot leak into the next.
REM  GCC needs no env setup (MinGW g++ is self-contained).
REM ============================================================
setlocal enabledelayedexpansion

cd /d "%~dp0"

echo ============================================
echo  smart_ptr Multi-Compiler Matrix
echo ============================================

set "LIST=%TEMP%\_smart_ptr_compilers.lst"
call "%~dp0_detect_compilers.bat"
if not exist "%LIST%" (
    echo.
    echo [ERROR] No compiler list produced.
    exit /b 1
)

set /a PASS=0
set /a FAIL=0
set /a SKIP=0

REM Single-command loop body: call delegates to a subroutine so each
REM iteration gets its own parse context (avoids for-block expansion traps).
for /f "usebackq delims=" %%R in ("%LIST%") do call :run_one "%%R"

del "%LIST%" >nul 2>&1
del _err.tmp 2>nul

REM Cleanup this run's artifacts only (explicit patterns; never bare test_*).
for %%f in (demo_*.exe comp_*.exe comp_mt_*.exe race_*.exe com_*.exe ts_*.exe sp_*.exe *.obj *.pdb *.ilk *.exp) do (
    if exist "%%f" del /q "%%f" 2>nul
)

echo.
echo ============================================
echo  MATRIX SUMMARY
echo ============================================
echo   Passed:  !PASS!
echo   Failed:  !FAIL!
echo   Skipped: !SKIP!
echo ============================================
if !FAIL! equ 0 (
    echo   ALL TARGETS PASSED
    exit /b 0
) else (
    echo   SOME TARGETS FAILED
    exit /b 1
)


REM ============================================================
REM  run_one subroutine -- parses one record, dispatches by KIND
REM ============================================================
:run_one
set "REC=%~1"
for /f "tokens=1-6 delims=;" %%a in ("%REC%") do (
    set "LABEL=%%a"
    set "KIND=%%b"
    set "ARCH=%%c"
    set "TIER=%%d"
    set "SETUP=%%e"
    set "EXE=%%f"
)
echo.
echo === !LABEL! ===
echo.
if /I "!KIND!"=="gcc" goto :run_gnu
goto :run_msvc


REM ============================================================
REM  run_msvc subroutine -- msvc and clang-cl targets
REM  Isolates vcvars in a nested setlocal; carries tallies out.
REM ============================================================
:run_msvc
setlocal
set "CC=!EXE!"
if not "!SETUP!"=="-" (
    if /I "!KIND!"=="clang-cl" (
        call "!SETUP!" >nul 2>&1
    ) else (
        call "!SETUP!" !ARCH! >nul 2>&1
    )
)
REM Verify the compiler is reachable before testing.
for %%X in ("!CC!") do set "CCNAME=%%~nX%%~xX"
where "!CCNAME!" >nul 2>&1
if errorlevel 1 (
    echo   [SKIP] !CCNAME! not available after setup
    endlocal & set "PASS=%PASS%" & set "FAIL=%FAIL%" & set "SKIP=%SKIP%"
    goto :eof
)
if "!TIER!"=="cpp98" (
    set "BASE=-nologo -W3 -EHsc -O2"
    call :msvc_test demo "..\demo.cpp" "demo_!LABEL!.exe" "!BASE!"
) else (
    set "BASE=-nologo -W4 -EHsc -utf-8 -O2"
    set "COMFLAGS=-nologo -W4 -EHsc -O2 -DUNICODE -D_UNICODE"
    call :msvc_test demo    "..\demo.cpp"                        "demo_!LABEL!.exe"    "!BASE!"
    call :msvc_test comp    "..\tests\test_comprehensive.cpp"    "comp_!LABEL!.exe"    "!BASE!"
    call :msvc_test comp_mt "..\tests\test_comprehensive_mt.cpp" "comp_mt_!LABEL!.exe" "!BASE!"
    call :msvc_test race    "..\tests\test_race_condition.cpp"   "race_!LABEL!.exe"   "!BASE!"
    call :msvc_test com     "..\tests\test_com.cpp"              "com_!LABEL!.exe"    "!COMFLAGS!"
    call :msvc_test ts      "..\tests\test_thread_safety.cpp"    "ts_!LABEL!.exe"     "!BASE!"
    call :msvc_test sp      "..\tests\test_smart_ptr.cpp"        "sp_!LABEL!.exe"     "!BASE!"
)
endlocal & set "PASS=%PASS%" & set "FAIL=%FAIL%" & set "SKIP=%SKIP%"
goto :eof


REM ============================================================
REM  run_gnu subroutine -- GCC (no env setup needed)
REM ============================================================
:run_gnu
set "GCC=!EXE!"
set "GBASE=-std=c++11 -Wall -O2"
set "DEMOBASE=-Wall -O2"
REM gcc4 tier (old MinGW, no <thread>): demo + comprehensive only.
call :gnu_test demo    "..\demo.cpp"                        "demo_!LABEL!.exe"    "!DEMOBASE!"
call :gnu_test comp    "..\tests\test_comprehensive.cpp"    "comp_!LABEL!.exe"    "!GBASE!"
if "!TIER!"=="full" (
    call :gnu_test comp_mt "..\tests\test_comprehensive_mt.cpp" "comp_mt_!LABEL!.exe" "!GBASE!"
    call :gnu_test race    "..\tests\test_race_condition.cpp"   "race_!LABEL!.exe"   "!GBASE! -pthread"
    call :gnu_test ts      "..\tests\test_thread_safety.cpp"    "ts_!LABEL!.exe"     "!GBASE! -pthread"
    call :gnu_test sp      "..\tests\test_smart_ptr.cpp"        "sp_!LABEL!.exe"     "!GBASE!"
    call :gnu_test com     "..\tests\test_com.cpp"              "com_!LABEL!.exe"    "-Wall -O2 -DUNICODE -D_UNICODE"
)
goto :eof


REM ============================================================
REM  msvc_test subroutine -- args: NAME SRC OUT FLAGS
REM  Uses CC/LABEL from the caller's setlocal scope.
REM ============================================================
:msvc_test
"!CC!" %~4 -I..\include "%~2" -Fe"%~3" >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp 2>nul
    echo   [FAIL] !LABEL! / %~1 ^(*compile*^)
    set /a FAIL+=1
    goto :eof
)
"%CD%\%~3" >nul 2>&1
if errorlevel 1 (
    echo   [FAIL] !LABEL! / %~1 ^(*run*^)
    set /a FAIL+=1
    goto :eof
)
echo   [PASS] !LABEL! / %~1
set /a PASS+=1
goto :eof


REM ============================================================
REM  gnu_test subroutine -- args: NAME SRC OUT FLAGS
REM  Uses GCC/LABEL from the caller's scope.
REM ============================================================
:gnu_test
"!GCC!" %~4 -I../include "%~2" -o "%~3" >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp 2>nul
    echo   [FAIL] !LABEL! / %~1 ^(*compile*^)
    set /a FAIL+=1
    goto :eof
)
"%CD%\%~3" >nul 2>&1
if errorlevel 1 (
    echo   [FAIL] !LABEL! / %~1 ^(*run*^)
    set /a FAIL+=1
    goto :eof
)
echo   [PASS] !LABEL! / %~1
set /a PASS+=1
goto :eof
