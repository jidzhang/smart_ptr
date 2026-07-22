@echo off
REM ============================================================
REM  _detect_compilers.bat - Enumerate compilers on this machine
REM ============================================================
REM  Probes every compiler the smart_ptr test matrix can use and
REM  writes one record per (compiler, arch) to a list file. The
REM  driver (test_matrix.bat) reads that file and runs the suite
REM  against each entry.
REM
REM  Record format (semicolon-delimited; '-' marks an unused field):
REM      LABEL;KIND;ARCH;TIER;SETUP;EXE
REM    LABEL  - human tag, e.g. VS2005-x86, Clang18-cl, GCC12
REM    KIND   - msvc | clang-cl | gcc
REM    ARCH   - x86 | amd64 | x64 | -     (arch passed to vcvarsall)
REM    TIER   - cpp98 (demo only) | full (full test set)
REM    SETUP  - vcvarsall/vcvars64 path for msvc/clang-cl, else '-'
REM    EXE    - compiler exe (cl for msvc; clang-cl path; g++ path)
REM
REM  Detection strategy:
REM    VS2005..VS2015 : VSxxCOMNTOOLS env var -> ..\..\VC\vcvarsall.bat
REM    VS2017+        : vswhere -all -products * -property installationPath
REM    Clang          : where clang-cl (MSVC drop-in, needs an MSVC env)
REM    GCC            : where g++
REM
REM  Note: vswhere must be invoked with a LITERAL "-products *" (cmd does
REM  not glob-expand '*'); a POSIX shell would mangle it. This script only
REM  ever runs under cmd, so that is safe.
REM
REM  Output: writes %TEMP%\_smart_ptr_compilers.lst and prints a summary.
REM  Returns: 0 always (missing compilers become missing records, not errors).
REM ============================================================
setlocal enabledelayedexpansion

set "LIST=%TEMP%\_smart_ptr_compilers.lst"
if exist "%LIST%" del "%LIST%" >nul 2>&1
set /a FOUND=0

REM --- MSVC via VSxxCOMNTOOLS (VS2005..VS2015) ---
REM  VS80=2005, VS90=2008, VS100=2010, VS110=2012, VS120=2013, VS140=2015.
REM  Pre-2012 toolchains lack <thread>/<atomic>/lambdas, so they only build
REM  the C++98 demo; 2012+ run the full set. VS2005's 64-bit arch token is
REM  "amd64"; VS2008+ accept "x64".
call :probe_comntools VS80COMNTOOLS  2005 cpp98 "x86 amd64"
call :probe_comntools VS90COMNTOOLS  2008 cpp98 "x86 x64"
call :probe_comntools VS100COMNTOOLS 2010 cpp98 "x86 x64"
call :probe_comntools VS110COMNTOOLS 2012 full  "x86 x64"
call :probe_comntools VS120COMNTOOLS 2013 full  "x86 x64"
call :probe_comntools VS140COMNTOOLS 2015 full  "x86 x64"

REM --- Resolve vswhere.exe (used for both VS2017+ and Clang's setup) ---
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VSW_TMP=%TEMP%\_smart_ptr_vsinstall.tmp"

REM --- MSVC via vswhere (VS2017+) : enumerate ALL installs, both arches ---
set "LATEST_VCVARS64="
if exist "%VSWHERE%" (
    "%VSWHERE%" -all -products * -property installationPath > "%VSW_TMP%" 2>nul
    for /f "usebackq delims=" %%P in ("%VSW_TMP%") do (
        set "IP=%%P"
        set "VCA=!IP!\VC\Auxiliary\Build\vcvarsall.bat"
        if exist "!VCA!" (
            REM Tag by the year folder in the install path (2017/2019/2022).
            set "TAG=VS"
            if not "!IP:2017=!"=="!IP!" set "TAG=VS2017"
            if not "!IP:2019=!"=="!IP!" set "TAG=VS2019"
            if not "!IP:2022=!"=="!IP!" set "TAG=VS2022"
            if "!TAG!"=="VS" set "TAG=VSnext"
            call :emit "!TAG!-x86" msvc x86 full "!VCA!" cl
            call :emit "!TAG!-x64" msvc x64 full "!VCA!" cl
            REM Remember the first (highest-version) vcvars64 for Clang.
            if not defined LATEST_VCVARS64 set "LATEST_VCVARS64=!IP!\VC\Auxiliary\Build\vcvars64.bat"
        )
    )
    del "%VSW_TMP%" >nul 2>&1
)

REM --- Clang (clang-cl, MSVC drop-in). Needs an MSVC env for headers/libs. ---
set "CLANG_CL="
where clang-cl >nul 2>&1
if not errorlevel 1 (
    for /f "delims=" %%C in ('where clang-cl') do if not defined CLANG_CL set "CLANG_CL=%%C"
)
if defined CLANG_CL if defined LATEST_VCVARS64 if exist "%LATEST_VCVARS64%" (
    call :emit "Clang-cl" clang-cl x64 full "%LATEST_VCVARS64%" "%CLANG_CL%"
)

REM --- GCC via MINGW4_HOME / MINGW12_HOME, then PATH fallback ---
REM  MinGW-4 (GCC 4.x) lacks <thread>/<atomic>, so it only builds the
REM  non-threaded set (tier "gcc4"); MinGW-12 runs the full set ("full").
set "GCC_FOUND="
call :probe_mingw MINGW4_HOME  GCC4  gcc4
call :probe_mingw MINGW12_HOME GCC12 full
if not defined GCC_FOUND (
    where g++ >nul 2>&1
    if not errorlevel 1 (
        set "GCC_EXE="
        for /f "delims=" %%C in ('where g++') do if not defined GCC_EXE set "GCC_EXE=%%C"
        if defined GCC_EXE call :emit "GCC-path" gcc - full - "!GCC_EXE!"
    )
)

echo.
echo Detected !FOUND! compiler/^arch target^(s^):
if exist "%LIST%" (
    for /f "usebackq delims=;" %%a in ("%LIST%") do echo    %%a
)
echo.

endlocal & set "DETECTED_LIST=%TEMP%\_smart_ptr_compilers.lst"
exit /b 0

REM ============================================================
REM  probe_comntools subroutine -- args: ENVVAR YEAR TIER "arch arch"
REM  (do not write the label with a leading ':' inside comments; cmd's
REM   label scanner can mistake it for the real label.)
REM ============================================================
:probe_comntools
set "ENVVAR=%~1"
set "YEAR=%~2"
set "TIER=%~3"
set "ARCHES=%~4"
call set "COMN=%%%ENVVAR%%%"
if not defined COMN goto :eof
set "VCA=!COMN!..\..\VC\vcvarsall.bat"
if not exist "!VCA!" goto :eof
for %%A in (!ARCHES!) do call :emit "VS!YEAR!-%%A" msvc %%A !TIER! "!VCA!" cl
goto :eof

REM ============================================================
REM  probe_mingw subroutine -- args: ENVVAR LABEL TIER
REM  Resolves <ENVVAR>\bin\g++.exe. Sets GCC_FOUND so the caller knows
REM  whether to fall back to PATH.
REM ============================================================
:probe_mingw
set "ENVVAR=%~1"
call set "MINGWHOME=%%%ENVVAR%%%"
if not defined MINGWHOME goto :eof
set "G=!MINGWHOME!\bin\g++.exe"
if not exist "!G!" goto :eof
call :emit "%~2" gcc - "%~3" - "!G!"
set "GCC_FOUND=1"
goto :eof

REM ============================================================
REM  emit subroutine -- args: LABEL KIND ARCH TIER SETUP EXE
REM ============================================================
:emit
>>"%LIST%" echo %~1;%~2;%~3;%~4;%~5;%~6
set /a FOUND+=1
goto :eof
