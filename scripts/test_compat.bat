@echo off
setlocal enabledelayedexpansion

echo ============================================
echo  smart_ptr Multi-Compiler Compatibility Test
echo ============================================
echo.

set PASS=0
set FAIL=0
set SKIP=0

rem GCC - use MINGW_HOME environment variables
if defined MINGW4_HOME  set "GCC4=%MINGW4_HOME%\bin\g++.exe"
if defined MINGW12_HOME set "GCC12=%MINGW12_HOME%\bin\g++.exe"
rem VS2005/VS2008 - derived from COMNTOOLS environment variables (set by VS installer)
if defined VS80COMNTOOLS  set "VS2005=%VS80COMNTOOLS%..\..\VC\vcvarsall.bat"
if defined VS90COMNTOOLS  set "VS2008=%VS90COMNTOOLS%..\..\VC\vcvarsall.bat"
rem VS2019 - COMNTOOLS not set since VS2017, use vswhere instead
if defined VS160COMNTOOLS (
    set "VS2019=%VS160COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat"
) else (
    set "VSWHERE=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!VSWHERE!" (
        "!VSWHERE!" -version "[16.0,17.0)" -property installationPath > _vs_tmp.txt 2>nul
        set /p VS2019_PATH=<_vs_tmp.txt
        del _vs_tmp.txt 2>nul
        if defined VS2019_PATH set "VS2019=!VS2019_PATH!\VC\Auxiliary\Build\vcvarsall.bat"
    )
)

:: ============================================
:: GCC 4.7 (MinGW) - no env setup needed
:: ============================================
echo === GCC 4.7 (MinGW) ===
echo.

if not exist "%GCC4%" (
    echo   [SKIP] Not found: %GCC4%
    set /a SKIP+=3
    goto gcc12
)

echo   [1/3] Demo (C++98)...
"%GCC4%" -Wall -O2 -I../include -o demo_gcc4.exe ../demo.cpp 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Demo compilation
    set /a FAIL+=1
) else (
    "%CD%\demo_gcc4.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Demo execution
        set /a FAIL+=1
    ) else (
        echo   [PASS] Demo
        set /a PASS+=1
    )
)

echo   [2/3] test_comprehensive.cpp (smart_ptr.h)...
"%GCC4%" -std=c++11 -Wall -O2 -I../include -o test_comp_gcc4.exe ../tests/test_comprehensive.cpp 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_comp_gcc4.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 16/16
        set /a PASS+=1
    )
)

echo   [3/3] test_comprehensive_mt.cpp (smart_ptr_mt.h)...
"%GCC4%" -std=c++11 -Wall -O2 -I../include -o test_comp_mt_gcc4.exe ../tests/test_comprehensive_mt.cpp 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_comp_mt_gcc4.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 40/40
        set /a PASS+=1
    )
)

:: Note: GCC 4.7 MinGW lacks <thread>/<atomic>, skip race_condition and com tests

:gcc12
echo.

:: ============================================
:: GCC 12.2 (MinGW-W64) - no env setup needed
:: ============================================
echo === GCC 12.2 (MinGW-W64) ===
echo.

if not exist "%GCC12%" (
    echo   [SKIP] Not found: %GCC12%
    set /a SKIP+=5
    goto vs2005
)

echo   [1/5] Demo (C++98)...
"%GCC12%" -Wall -Wextra -O2 -I../include -o demo_gcc12.exe ../demo.cpp 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Demo compilation
    set /a FAIL+=1
) else (
    "%CD%\demo_gcc12.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Demo execution
        set /a FAIL+=1
    ) else (
        echo   [PASS] Demo
        set /a PASS+=1
    )
)

echo   [2/5] test_comprehensive.cpp (smart_ptr.h)...
"%GCC12%" -std=c++11 -Wall -Wextra -O2 -I../include -o test_comp_gcc12.exe ../tests/test_comprehensive.cpp 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_comp_gcc12.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 16/16
        set /a PASS+=1
    )
)

echo   [3/5] test_comprehensive_mt.cpp (smart_ptr_mt.h)...
"%GCC12%" -std=c++11 -Wall -Wextra -O2 -I../include -o test_comp_mt_gcc12.exe ../tests/test_comprehensive_mt.cpp 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_comp_mt_gcc12.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 40/40
        set /a PASS+=1
    )
)

echo   [4/5] test_race_condition.cpp...
"%GCC12%" -std=c++11 -Wall -Wextra -O2 -I../include -o test_race_gcc12.exe ../tests/test_race_condition.cpp -pthread 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_race_gcc12.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] Race condition
        set /a PASS+=1
    )
)

echo   [5/5] test_com.cpp...
"%GCC12%" -Wall -Wextra -O2 -I../include -DUNICODE -D_UNICODE -o test_com_gcc12.exe ../tests/test_com.cpp 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_com_gcc12.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 5/5
        set /a PASS+=1
    )
)

:vs2005
echo.

:: ============================================
:: VS2005 (Demo only - C++98 compatibility)
:: Each MSVC version is isolated with setlocal/endlocal
:: ============================================
echo === VS2005 x86 ===
echo.
setlocal
if not exist "%VS2005%" (
    endlocal
    echo   [SKIP] vcvarsall.bat not found
    set /a SKIP+=1
    goto vs2005_x64
)
call "%VS2005%" x86 >nul 2>&1
where cl.exe >nul 2>&1
if errorlevel 1 (
    endlocal
    echo   [SKIP] cl.exe not available
    set /a SKIP+=1
    goto vs2005_x64
)
echo   [1/1] Demo (C++98)...
cl -nologo -W3 -EHsc -O2 -I..\include ..\demo.cpp -Fedemo_vs2005_x86.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Demo compilation
    set /a FAIL+=1
) else (
    "%CD%\demo_vs2005_x86.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Demo execution
        set /a FAIL+=1
    ) else (
        echo   [PASS] Demo
        set /a PASS+=1
    )
)
endlocal & set "PASS=%PASS%" & set "FAIL=%FAIL%" & set "SKIP=%SKIP%"

:vs2005_x64
echo.

echo === VS2005 x64 ===
echo.
setlocal
if not exist "%VS2005%" (
    endlocal
    echo   [SKIP] vcvarsall.bat not found
    set /a SKIP+=1
    goto vs2008
)
call "%VS2005%" amd64 >nul 2>&1
where cl.exe >nul 2>&1
if errorlevel 1 (
    endlocal
    echo   [SKIP] cl.exe not available for amd64
    set /a SKIP+=1
    goto vs2008
)
echo   [1/1] Demo (C++98)...
cl -nologo -W3 -EHsc -O2 -I..\include ..\demo.cpp -Fedemo_vs2005_x64.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Demo compilation
    set /a FAIL+=1
) else (
    "%CD%\demo_vs2005_x64.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Demo execution
        set /a FAIL+=1
    ) else (
        echo   [PASS] Demo
        set /a PASS+=1
    )
)
endlocal & set "PASS=%PASS%" & set "FAIL=%FAIL%" & set "SKIP=%SKIP%"

:vs2008
echo.

:: ============================================
:: VS2008 (Demo only - C++98 compatibility)
:: ============================================
echo === VS2008 x86 ===
echo.
setlocal
if not exist "%VS2008%" (
    endlocal
    echo   [SKIP] vcvarsall.bat not found
    set /a SKIP+=1
    goto vs2008_x64
)
call "%VS2008%" x86 >nul 2>&1
where cl.exe >nul 2>&1
if errorlevel 1 (
    endlocal
    echo   [SKIP] cl.exe not available
    set /a SKIP+=1
    goto vs2008_x64
)
echo   [1/1] Demo (C++98)...
cl -nologo -W3 -EHsc -O2 -I..\include ..\demo.cpp -Fedemo_vs2008_x86.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Demo compilation
    set /a FAIL+=1
) else (
    "%CD%\demo_vs2008_x86.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Demo execution
        set /a FAIL+=1
    ) else (
        echo   [PASS] Demo
        set /a PASS+=1
    )
)
endlocal & set "PASS=%PASS%" & set "FAIL=%FAIL%" & set "SKIP=%SKIP%"

:vs2008_x64
echo.

echo === VS2008 x64 ===
echo.
setlocal
if not exist "%VS2008%" (
    endlocal
    echo   [SKIP] vcvarsall.bat not found
    set /a SKIP+=1
    goto vs2019
)
call "%VS2008%" x64 >nul 2>&1
where cl.exe >nul 2>&1
if errorlevel 1 (
    endlocal
    echo   [SKIP] cl.exe not available for x64
    set /a SKIP+=1
    goto vs2019
)
echo   [1/1] Demo (C++98)...
cl -nologo -W3 -EHsc -O2 -I..\include ..\demo.cpp -Fedemo_vs2008_x64.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Demo compilation
    set /a FAIL+=1
) else (
    "%CD%\demo_vs2008_x64.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Demo execution
        set /a FAIL+=1
    ) else (
        echo   [PASS] Demo
        set /a PASS+=1
    )
)
endlocal & set "PASS=%PASS%" & set "FAIL=%FAIL%" & set "SKIP=%SKIP%"

:vs2019
echo.

:: ============================================
:: VS2019 (Full tests)
:: ============================================
echo === VS2019 x86 ===
echo.
setlocal
if not exist "%VS2019%" (
    endlocal
    echo   [SKIP] vcvarsall.bat not found
    set /a SKIP+=5
    goto vs2019_x64
)
call "%VS2019%" x86 >nul 2>&1
where cl.exe >nul 2>&1
if errorlevel 1 (
    endlocal
    echo   [SKIP] cl.exe not available
    set /a SKIP+=5
    goto vs2019_x64
)

echo   [1/5] Demo (C++98)...
cl -nologo -W4 -EHsc -utf-8 -O2 -I..\include ..\demo.cpp -Fedemo_vs2019_x86.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Demo compilation
    set /a FAIL+=1
) else (
    "%CD%\demo_vs2019_x86.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Demo execution
        set /a FAIL+=1
    ) else (
        echo   [PASS] Demo
        set /a PASS+=1
    )
)

echo   [2/5] test_comprehensive.cpp (smart_ptr.h)...
cl -nologo -W4 -EHsc -utf-8 -O2 -I..\include ..\tests\test_comprehensive.cpp -Fetest_comp_vs2019_x86.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_comp_vs2019_x86.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 16/16
        set /a PASS+=1
    )
)

echo   [3/5] test_comprehensive_mt.cpp (smart_ptr_mt.h)...
cl -nologo -W4 -EHsc -utf-8 -O2 -I..\include ..\tests\test_comprehensive_mt.cpp -Fetest_comp_mt_vs2019_x86.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_comp_mt_vs2019_x86.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 40/40
        set /a PASS+=1
    )
)

echo   [4/5] test_race_condition.cpp...
cl -nologo -W4 -EHsc -utf-8 -O2 -I..\include ..\tests\test_race_condition.cpp -Fetest_race_vs2019_x86.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_race_vs2019_x86.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] Race condition
        set /a PASS+=1
    )
)

echo   [5/5] test_com.cpp...
cl -nologo -W4 -EHsc -O2 -DUNICODE -D_UNICODE -I..\include ..\tests\test_com.cpp -Fetest_com_vs2019_x86.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_com_vs2019_x86.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 5/5
        set /a PASS+=1
    )
)
endlocal & set "PASS=%PASS%" & set "FAIL=%FAIL%" & set "SKIP=%SKIP%"

:vs2019_x64
echo.

echo === VS2019 x64 ===
echo.
setlocal
if not exist "%VS2019%" (
    endlocal
    echo   [SKIP] vcvarsall.bat not found
    set /a SKIP+=5
    goto summary
)
call "%VS2019%" x64 >nul 2>&1
where cl.exe >nul 2>&1
if errorlevel 1 (
    endlocal
    echo   [SKIP] cl.exe not available
    set /a SKIP+=5
    goto summary
)

echo   [1/5] Demo (C++98)...
cl -nologo -W4 -EHsc -utf-8 -O2 -I..\include ..\demo.cpp -Fedemo_vs2019_x64.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Demo compilation
    set /a FAIL+=1
) else (
    "%CD%\demo_vs2019_x64.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Demo execution
        set /a FAIL+=1
    ) else (
        echo   [PASS] Demo
        set /a PASS+=1
    )
)

echo   [2/5] test_comprehensive.cpp (smart_ptr.h)...
cl -nologo -W4 -EHsc -utf-8 -O2 -I..\include ..\tests\test_comprehensive.cpp -Fetest_comp_vs2019_x64.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_comp_vs2019_x64.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 16/16
        set /a PASS+=1
    )
)

echo   [3/5] test_comprehensive_mt.cpp (smart_ptr_mt.h)...
cl -nologo -W4 -EHsc -utf-8 -O2 -I..\include ..\tests\test_comprehensive_mt.cpp -Fetest_comp_mt_vs2019_x64.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_comp_mt_vs2019_x64.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 40/40
        set /a PASS+=1
    )
)

echo   [4/5] test_race_condition.cpp...
cl -nologo -W4 -EHsc -utf-8 -O2 -I..\include ..\tests\test_race_condition.cpp -Fetest_race_vs2019_x64.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_race_vs2019_x64.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] Race condition
        set /a PASS+=1
    )
)

echo   [5/5] test_com.cpp...
cl -nologo -W4 -EHsc -O2 -DUNICODE -D_UNICODE -I..\include ..\tests\test_com.cpp -Fetest_com_vs2019_x64.exe >nul 2>_err.tmp
if errorlevel 1 (
    type _err.tmp
    echo   [FAIL] Compilation
    set /a FAIL+=1
) else (
    "%CD%\test_com_vs2019_x64.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAIL] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] 5/5
        set /a PASS+=1
    )
)
endlocal & set "PASS=%PASS%" & set "FAIL=%FAIL%" & set "SKIP=%SKIP%"

:summary
echo.

:: ============================================
:: Cleanup
:: ============================================
del _err.tmp 2>nul
for %%f in (demo_gcc*.exe demo_vs*.exe test_*_gcc*.exe test_*_vs*.exe *.obj) do (
    del /f /q "%%f" 2>nul
)

:: ============================================
:: Summary
:: ============================================
echo ============================================
echo  SUMMARY
echo ============================================
echo.
echo   Passed:  !PASS!
echo   Failed:  !FAIL!
echo   Skipped: !SKIP!
echo.
echo   GCC 4.7:   Demo + test_comprehensive (no ^<thread^>)
echo   GCC 12.2:  Full tests
echo   VS2005:    Demo only (x86 + x64)
echo   VS2008:    Demo only (x86 + x64)
echo   VS2019:    Full tests (x86 + x64)
echo.

if !FAIL! equ 0 (
    echo   ALL TESTS PASSED
    echo ============================================
    exit /b 0
) else (
    echo   SOME TESTS FAILED
    echo ============================================
    exit /b 1
)
