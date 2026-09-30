@echo off
setlocal EnableExtensions
cd /d "%~dp0"

rem --- Git: use the one from PATH, otherwise look inside GitHub Desktop ---
where git >nul 2>&1
if errorlevel 1 (
    for /f "delims=" %%D in ('dir /b /ad /o-n "%LOCALAPPDATA%\GitHubDesktop\app-*" 2^>nul') do (
        if exist "%LOCALAPPDATA%\GitHubDesktop\%%D\resources\app\git\cmd\git.exe" (
            set "PATH=%PATH%;%LOCALAPPDATA%\GitHubDesktop\%%D\resources\app\git\cmd"
            goto :git_ok
        )
    )
    echo Git was not found. Install Git for Windows and try again.
    pause
    exit /b 1
)
:git_ok

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
echo 6 - Check changelog status
echo 7 - Generate/catch up changelog (14-day window)
echo 8 - Generate changelog from a chosen base revision
echo 0 - Exit
echo.
echo Notes:
echo  - The manifest is built from HEAD: uncommitted gamedata changes are not included.
echo  - After 1/3/4 commit manifest.txt (and the two ui_st_other.xml files for 1/4).
echo  - Verify (2) reports STALE until those changes are committed.
echo.
set "ACTION="
set /p "ACTION=Choose an action: "
if "%ACTION%"=="0" exit /b 0
if "%ACTION%"=="8" goto :changelog_since
if "%ACTION%"=="7" (
    call :run --changelog
    goto :menu
)
if "%ACTION%"=="6" (
    call :run --changelog-status
    goto :menu
)
if "%ACTION%"=="5" goto :help
if "%ACTION%"=="4" goto :custom_stamp
if "%ACTION%"=="3" (
    call :warn_dirty
    call :run --apply --no-menu-version
    goto :menu
)
if "%ACTION%"=="2" (
    call :run --verify
    goto :menu
)
if "%ACTION%"=="1" (
    call :warn_dirty
    call :run --apply
    goto :menu
)
echo Invalid action. Please choose a number from the menu.
pause
goto :menu

:changelog_since
set "CHANGELOG_SINCE="
echo.
echo Enter a Git base revision. The changelog includes commits after it.
echo Example: HEAD~5   or   80b7ba5
set /p "CHANGELOG_SINCE=Base revision: "
if not defined CHANGELOG_SINCE (
    echo No base revision entered. Returning to the menu.
    goto :menu
)
call :run --changelog --changelog-since "%CHANGELOG_SINCE%"
goto :menu

:custom_stamp
set "PATCH_STAMP="
echo.
set /p "PATCH_STAMP=Enter DD.MM.YY_HH:MM (Moscow time): "
if not defined PATCH_STAMP goto :menu
call :warn_dirty
call :run --apply --patch-stamp "%PATCH_STAMP%_MSK"
goto :menu

:help
call :run --help
goto :menu

:warn_dirty
set "DIRTY="
for /f "delims=" %%L in ('git status --porcelain -- gamedata he_gui.dll bin_x64 2^>nul') do set "DIRTY=1"
if defined DIRTY (
    echo.
    echo WARNING: uncommitted changes in shipped paths. The manifest uses HEAD,
    echo so they are NOT included until committed. Continuing anyway.
)
exit /b 0

:run
echo Working...
call %PYTHON% "%SCRIPT%" %*
set "RC=%ERRORLEVEL%"
echo.
if "%RC%"=="0" (echo Completed.) else (echo Failed with exit code %RC%.)
pause
exit /b %RC%
