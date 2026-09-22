@echo off
setlocal EnableExtensions
cd /d "%~dp0"

set "SCRIPT=%~dp0make_manifest.py"
set "PYTHON=python"
python --version >nul 2>&1
if errorlevel 1 (
    set "PYTHON=py -3"
    py -3 --version >nul 2>&1
    if errorlevel 1 set "PYTHON="
)

if not defined PYTHON (
    echo Python was not found. Install Python 3 and try again.
    pause
    exit /b 1
)

:menu
cls
echo NLC manifest helper
echo.
echo 1 - Apply manifest and update menu version with current Moscow time
echo 2 - Verify manifest
echo 3 - Apply manifest without changing menu version
echo 4 - Apply manifest with a custom Moscow timestamp
echo 5 - Show make_manifest.py help
echo 0 - Exit
echo.
choice /c 123450 /n /m "Choose an action: "
if errorlevel 6 exit /b 0
if errorlevel 5 goto :help
if errorlevel 4 goto :custom_stamp
if errorlevel 3 (
    call :run --apply --no-menu-version
    goto :menu
)
if errorlevel 2 (
    call :run --verify
    goto :menu
)
if errorlevel 1 (
    call :run --apply
    goto :menu
)
goto :menu

:custom_stamp
set "PATCH_STAMP="
echo.
set /p "PATCH_STAMP=Enter DD.MM.YY_HH:MM (Moscow time): "
if not defined PATCH_STAMP goto :menu
call :run --apply --patch-stamp "%PATCH_STAMP%_MSK"
goto :menu

:help
call :run --help
goto :menu

:run
call %PYTHON% "%SCRIPT%" %*
set "RC=%ERRORLEVEL%"
echo.
if "%RC%"=="0" (echo Completed.) else (echo Failed with exit code %RC%.)
pause
exit /b %RC%
