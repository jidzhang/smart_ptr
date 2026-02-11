@echo off
setlocal enabledelayedexpansion

echo ============================================
echo Clean Build Artifacts
echo ============================================
echo.

set "FILES_DELETED=0"

:: Delete .exe files (but not .bat)
for %%f in (*.exe) do (
    echo Deleting: %%f
    del /f /q "%%f"
    set /a FILES_DELETED+=1
)

:: Delete .obj files
for %%f in (*.obj) do (
    echo Deleting: %%f
    del /f /q "%%f"
    set /a FILES_DELETED+=1
)

echo.
if !FILES_DELETED! == 0 (
    echo No build artifacts found.
) else (
    echo Deleted !FILES_DELETED! file(s).
)

echo.
echo Clean complete.
echo ============================================

exit /b 0
