@echo off
REM ============================================================
REM  _setup_msvc.bat - Initialize MSVC build environment
REM ============================================================
REM  Shared helper invoked (via "call") by every batch script in
REM  this folder that needs cl.exe / link.exe.
REM
REM  Locates the latest installed Visual Studio via vswhere and
REM  calls its vcvars64.bat. Safe to call multiple times: if
REM  cl.exe is already on PATH it returns immediately.
REM
REM  This helper does NOT use setlocal/endlocal, because
REM  vcvars64.bat modifies the current environment (PATH,
REM  INCLUDE, LIB, ...) and those changes must propagate to the
REM  calling script.
REM
REM  Usage (from any .bat in this folder):
REM      call "%~dp0_setup_msvc.bat"
REM      if errorlevel 1 exit /b 1
REM
REM  Returns: 0 on success (cl.exe available), 1 on failure.
REM  Does NOT change the caller's current directory.
REM ============================================================

REM --- Idempotency: skip if cl.exe is already available ---
where cl.exe >nul 2>&1
if not errorlevel 1 exit /b 0

REM --- Override: honor an explicit VCVARS64=path\to\vcvars64.bat ---
REM  Escape hatch for non-standard installs (D: drive, Build Tools only,
REM  vswhere misconfigured) -- point directly at vcvars64.bat.
if defined VCVARS64 (
    set "VCVARS=%VCVARS64%"
    call "%VCVARS64%" >nul 2>&1
    if errorlevel 1 goto :vcvars_failed
    where cl.exe >nul 2>&1
    if errorlevel 1 goto :vcvars_failed
    exit /b 0
)

REM --- Resolve the vswhere.exe path ---
REM  Program Files (x86) contains parentheses; we only ever use
REM  %VSWHERE% inside double quotes and inside a FOR /F backquote
REM  command, never inside a parenthesized IF/FOR block, so the
REM  ")" in the path cannot break block parsing.
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :no_vswhere

REM --- Resolve the latest VS installation that has vcvars64.bat ---
REM  vswhere is asked for the installationPath line. We write it to a
REM  temp file and read it back with set /p -- this avoids any quirks
REM  of parsing a "(x86)" path through FOR /F in every cmd version.
set "VSWHERE_TMP=%TEMP%\_smart_ptr_vsinstall.tmp"
"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath > "%VSWHERE_TMP%" 2>nul

set "VSINSTALL="
for /f "usebackq delims=" %%i in ("%VSWHERE_TMP%") do set "VSINSTALL=%%i"

if not defined VSINSTALL (
    "%VSWHERE%" -latest -products * -property installationPath > "%VSWHERE_TMP%" 2>nul
    set "VSINSTALL="
    for /f "usebackq delims=" %%i in ("%VSWHERE_TMP%") do set "VSINSTALL=%%i"
)

del "%VSWHERE_TMP%" >nul 2>&1

if not defined VSINSTALL goto :no_vsinstall

REM --- Build the vcvars64.bat path from the installation root and call it ---
set "VCVARS=%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
if not exist "%VCVARS%" goto :no_vcvars

call "%VCVARS%" >nul 2>&1
if errorlevel 1 goto :vcvars_failed

REM --- Final verification: cl.exe must now be reachable ---
where cl.exe >nul 2>&1
if errorlevel 1 goto :vcvars_failed
exit /b 0

:no_vswhere
echo [_setup_msvc] ERROR: vswhere.exe not found. Visual Studio may not be installed,
echo [_setup_msvc]        or set the VCVARS64 environment variable to your vcvars64.bat.
exit /b 1

:no_vsinstall
echo [_setup_msvc] ERROR: No Visual Studio installation was found.
echo [_setup_msvc]        Install the "Desktop development with C++" workload,
echo [_setup_msvc]        or set VCVARS64 to the full path of your vcvars64.bat.
exit /b 1

:no_vcvars
echo [_setup_msvc] ERROR: vcvars64.bat not found under: %VSINSTALL%
exit /b 1

:vcvars_failed
echo [_setup_msvc] ERROR: Failed to initialize MSVC environment from: %VCVARS%
exit /b 1