@echo off
setlocal
set "SCRIPT=%~dp0weapon_timing.ps1"

:menu
echo.
echo Weapon timing editor
echo 1. Export current timing profiles
echo 2. Validate profiles
echo 3. Preview changes
echo 4. Apply profiles to repository
echo 5. Create wider-disparity candidate
echo 0. Exit
set /p "CHOICE=Select: "
if "%CHOICE%"=="1" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action export
if "%CHOICE%"=="2" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action validate
if "%CHOICE%"=="3" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action preview
if "%CHOICE%"=="4" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action apply
if "%CHOICE%"=="5" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action spread
if "%CHOICE%"=="0" exit /b 0
pause
goto menu
