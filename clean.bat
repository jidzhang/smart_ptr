@echo off
setlocal enabledelayedexpansion

echo ============================================
echo Clean Build Artifacts
echo ============================================

set FILES_DELETED=0

for %%f in (*.exe) do (
    echo Deleting: %%f
    del /f /q "%%f"
    set /A FILES_DELETED=!FILES_DELETED!+1
)

for %%f in (*.obj) do (
    echo Deleting: %%f
    del /f /q "%%f"
    set /A FILES_DELETED=!FILES_DELETED!+1
)

if !FILES_DELETED!==0 (
    echo No build artifacts found
) else (
    echo Deleted !FILES_DELETED! file(s)
)

echo Clean complete
echo ============================================

exit /b 0
