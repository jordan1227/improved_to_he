@echo off
setlocal
set "SCRIPT=%~dp0weapon_audio_timing.ps1"

:menu
echo.
echo Weapon audio timing tool
echo 1. Scan reload sounds (recommended default)
echo 2. Scan draw and holster sounds
echo 3. Scan reload, draw, and holster sounds
echo 4. Validate enabled rows
echo 5. Preview enabled rows
echo 6. Render enabled rows
echo 7. Patch configs to rendered variants
echo 0. Exit
set /p "CHOICE=Select: "
if "%CHOICE%"=="1" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action scan -Mode reload
if "%CHOICE%"=="2" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action scan -Mode ready
if "%CHOICE%"=="3" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action scan -Mode all
if "%CHOICE%"=="4" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action validate -Mode all
if "%CHOICE%"=="5" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action preview -Mode all
if "%CHOICE%"=="6" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action render -Mode all
if "%CHOICE%"=="7" powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SCRIPT%" -Action apply-config -Mode all
if "%CHOICE%"=="0" exit /b 0
pause
goto menu
