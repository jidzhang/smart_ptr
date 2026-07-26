@echo off
setlocal enabledelayedexpansion

echo ============================================
echo smart_ptr Full Test Suite
echo ============================================
echo.

REM --- Initialize MSVC environment (cl.exe / vcvars64.bat) ---
call "%~dp0_setup_msvc.bat"
if errorlevel 1 exit /b 1

REM --- Pin CWD to this script's folder so ..\tests and ..\include resolve ---
cd /d "%~dp0"

REM --- Remove stale artifacts; a binary still running cannot be deleted and
REM     would make the matching compile step fail with "Permission denied".
REM     Every artifact this script builds matches test_*.exe.
for %%f in (test_*.exe *.obj *.pdb *.ilk _err.tmp) do (
    if exist "%%f" (
        del /q "%%f" >nul 2>&1
        if exist "%%f" echo   [WARN] %%f is locked by a running process - terminate it and retry
    )
)

set /a PASS=0
set /a FAIL=0

REM --- Per-suite test counts; single source for the [PASS] lines and the
REM     derived total in the summary, so the two cannot drift apart.
set /a T_ST=17
set /a T_MT=41
set /a T_COM=5
set /a T_RACE=5
set /a T_TS=8
set /a T_CATCH=39
set /a T_DISPOSE=4
set /a T_CXX98=17

:: --------------------------------------------
:: MSVC Tests
:: --------------------------------------------
echo === MSVC Tests ===
echo.

echo [1/10] smart_ptr.h (MSVC)...
cl -nologo -W4 -EHsc -utf-8 -O2 ..\tests\test_comprehensive.cpp -I..\include -Fetest_comprehensive_msvc.exe >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] Compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
) else (
    "%CD%\test_comprehensive_msvc.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] !T_ST!/!T_ST!
        set /a PASS+=1
    )
)

echo [2/10] smart_ptr_mt.h (MSVC)...
cl -nologo -W4 -EHsc -utf-8 -O2 -Fetest_comprehensive_mt_msvc.exe ..\tests\test_comprehensive_mt.cpp -I..\include >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] Compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
) else (
    "%CD%\test_comprehensive_mt_msvc.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] !T_MT!/!T_MT!
        set /a PASS+=1
    )
)

echo [3/10] test_com.cpp (MSVC)...
cl -nologo -W4 -EHsc -O2 -DUNICODE -D_UNICODE ..\tests\test_com.cpp -I..\include >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] Compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
) else (
    "%CD%\test_com.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] !T_COM!/!T_COM!
        set /a PASS+=1
    )
)

:: --------------------------------------------
:: GCC Tests
:: --------------------------------------------
echo.
echo === GCC Tests ===
echo.

echo [4/10] smart_ptr.h (GCC)...
g++ -std=c++11 -Wall -O2 -o test_comprehensive_gcc.exe ../tests/test_comprehensive.cpp -I../include >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] Compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
) else (
    "%CD%\test_comprehensive_gcc.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] !T_ST!/!T_ST!
        set /a PASS+=1
    )
)

echo [5/10] smart_ptr_mt.h (GCC)...
g++ -std=c++11 -Wall -O2 -o test_comprehensive_mt_gcc.exe ../tests/test_comprehensive_mt.cpp -I../include >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] Compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
) else (
    "%CD%\test_comprehensive_mt_gcc.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] !T_MT!/!T_MT!
        set /a PASS+=1
    )
)

:: --------------------------------------------
:: Race Condition Stress Test
:: --------------------------------------------
echo.
echo === Race Condition ===
echo.

echo [6/10] stress test (MSVC + GCC)...
set /a RACE_PASS=0

cl -nologo -W4 -EHsc -utf-8 -O2 ..\tests\test_race_condition.cpp -I..\include -Fetest_race_msvc.exe >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] MSVC compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
    goto :race_done
)

"%CD%\test_race_msvc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] MSVC race test failed
    set /a FAIL+=1
    goto :race_done
)
set /a RACE_PASS+=1

g++ -O2 -std=c++11 -o test_race_gcc.exe ../tests/test_race_condition.cpp -I../include -pthread >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] GCC compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
    goto :race_done
)

"%CD%\test_race_gcc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] GCC race test failed
    set /a FAIL+=1
    goto :race_done
)
set /a RACE_PASS+=1

echo   [PASS] MSVC + GCC
set /a PASS+=1
goto :race_end

:race_done
echo   MSVC !RACE_PASS!/2, GCC !RACE_PASS!/2

:race_end

:: --------------------------------------------
:: Thread Safety Stress Test
:: --------------------------------------------
echo.
echo === Thread Safety ===
echo.

echo [7/10] thread safety (MSVC + GCC)...
set /a TS_PASS=0

cl -nologo -W4 -EHsc -utf-8 -O2 ..\tests\test_thread_safety.cpp -I..\include -Fetest_thread_safety_msvc.exe >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] MSVC compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
    goto :ts_done
)

"%CD%\test_thread_safety_msvc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] MSVC thread safety test failed
    set /a FAIL+=1
    goto :ts_done
)
set /a TS_PASS+=1

g++ -O2 -std=c++11 -o test_thread_safety_gcc.exe ../tests/test_thread_safety.cpp -I../include -pthread >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] GCC compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
    goto :ts_done
)

"%CD%\test_thread_safety_gcc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] GCC thread safety test failed
    set /a FAIL+=1
    goto :ts_done
)
set /a TS_PASS+=1

echo   [PASS] MSVC + GCC
set /a PASS+=1
goto :ts_end

:ts_done
echo   MSVC !TS_PASS!/2, GCC !TS_PASS!/2

:ts_end

:: --------------------------------------------
:: Unit Test (Catch2 framework, C++11)
:: --------------------------------------------
echo.
echo === Unit Test (Catch2) ===
echo.

echo [8/10] smart_ptr unit test (MSVC + GCC)...
set /a UT_PASS=0

cl -nologo -W4 -EHsc -utf-8 -O2 ..\tests\test_smart_ptr.cpp -I..\include -Fetest_smart_ptr_msvc.exe >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] MSVC compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
    goto :ut_done
)

"%CD%\test_smart_ptr_msvc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] MSVC unit test failed
    set /a FAIL+=1
    goto :ut_done
)
set /a UT_PASS+=1

g++ -std=c++11 -Wall -O2 -o test_smart_ptr_gcc.exe ../tests/test_smart_ptr.cpp -I../include >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] GCC compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
    goto :ut_done
)

"%CD%\test_smart_ptr_gcc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] GCC unit test failed
    set /a FAIL+=1
    goto :ut_done
)
set /a UT_PASS+=1

echo   [PASS] MSVC + GCC
set /a PASS+=1
goto :ut_end

:ut_done
echo   MSVC !UT_PASS!/2, GCC !UT_PASS!/2

:ut_end

:: --------------------------------------------
:: Dispose Semantics (type-erased deleter regression)
:: --------------------------------------------
echo.
echo === Dispose Semantics ===
echo.

echo [9/10] dispose semantics (MSVC + GCC)...
set /a DS_PASS=0

cl -nologo -W4 -EHsc -utf-8 -O2 ..\tests\test_dispose_semantics.cpp -I..\include -Fetest_dispose_semantics_msvc.exe >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] MSVC compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
    goto :ds_done
)

"%CD%\test_dispose_semantics_msvc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] MSVC dispose test failed
    set /a FAIL+=1
    goto :ds_done
)
set /a DS_PASS+=1

REM Warning gate: this file must stay clean under -Wall -Wextra -Werror.
REM No -O2 here: GCC -O2 inlining analysis emits unrelated use-after-free
REM false positives.
g++ -std=c++11 -Wall -Wextra -Werror -o test_dispose_semantics_gcc.exe ../tests/test_dispose_semantics.cpp -I../include >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] GCC compile error ^(warning gate^)
    type _err.tmp 2>nul
    set /a FAIL+=1
    goto :ds_done
)

"%CD%\test_dispose_semantics_gcc.exe" >nul 2>&1
if errorlevel 1 (
    echo   [FAILED] GCC dispose test failed
    set /a FAIL+=1
    goto :ds_done
)
set /a DS_PASS+=1

echo   [PASS] MSVC + GCC
set /a PASS+=1
goto :ds_end

:ds_done
echo   MSVC !DS_PASS!/2, GCC !DS_PASS!/2

:ds_end

:: --------------------------------------------
:: C++98 Mode Check (GCC only)
:: --------------------------------------------
echo.
echo === C++98 Mode ===
echo.

echo [10/10] C++98 mode check (GCC)...
REM MSVC VS2015+ always takes the C++11 code path (_MSC_VER at least 1900),
REM even under /std:c++03, so only GCC can exercise the real C++98 branch.
REM Move/cast tests compile to stubs in this mode; everything else runs.
g++ -std=c++98 -Wall -O2 -o test_comprehensive_cxx98.exe ../tests/test_comprehensive.cpp -I../include >nul 2>_err.tmp
if errorlevel 1 (
    echo   [FAILED] Compile error
    type _err.tmp 2>nul
    set /a FAIL+=1
) else (
    "%CD%\test_comprehensive_cxx98.exe" >nul 2>&1
    if errorlevel 1 (
        echo   [FAILED] Tests failed
        set /a FAIL+=1
    ) else (
        echo   [PASS] !T_CXX98!/!T_CXX98! ^(C++98 mode^)
        set /a PASS+=1
    )
)

:: --------------------------------------------
:: Summary
:: --------------------------------------------
echo.
echo ============================================
echo Results: !PASS!/10 suites passed, !FAIL!/10 failed
echo ============================================

del _err.tmp 2>nul

if !FAIL! equ 0 (
    set /a TOTAL=T_ST*2+T_MT*2+T_COM+T_RACE*2+T_TS*2+T_CATCH*2+T_DISPOSE*2+T_CXX98
    echo.
    echo   smart_ptr.h:     !T_ST! tests (MSVC + GCC^)
    echo   smart_ptr_mt.h:  !T_MT! tests (MSVC + GCC^)
    echo   test_com.cpp:     !T_COM! tests (MSVC^)
    echo   race condition:   !T_RACE! tests x2 (MSVC + GCC^)
    echo   thread safety:   !T_TS! tests x2 (MSVC + GCC^)
    echo   smart_ptr (Catch2^): !T_CATCH! tests x2 (MSVC + GCC^)
    echo   dispose semantics: !T_DISPOSE! tests x2 (MSVC + GCC^)
    echo   C++98 mode (ST^):  !T_CXX98! tests (GCC^)
    echo.
    echo   Total: !TOTAL! tests across 10 suites
    echo ============================================
    exit /b 0
) else (
    exit /b 1
)
