@echo off
rem Build Апдейтер.exe with the csc.exe that ships with Windows (.NET 4).
setlocal
set CSC=%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe
if not exist "%CSC%" set CSC=%WINDIR%\Microsoft.NET\Framework\v4.0.30319\csc.exe
if not exist "%CSC%" (
    echo csc.exe not found - install .NET Framework 4.x
    exit /b 1
)
set OUT=%~dp0Апдейтер.exe
"%CSC%" /nologo /target:winexe /optimize+ /platform:anycpu /out:"%OUT%" ^
    /reference:System.dll /reference:System.Drawing.dll /reference:System.Windows.Forms.dll ^
    "%~dp0Launcher.cs"
if errorlevel 1 exit /b 1
echo built %OUT%
set OUT=%~dp0Апдейтер_без_weapons_levels.exe
"%CSC%" /nologo /target:winexe /optimize+ /platform:anycpu /define:NLC_PROTECTED_ASSETS /out:"%OUT%" ^
    /reference:System.dll /reference:System.Drawing.dll /reference:System.Windows.Forms.dll ^
    "%~dp0Launcher.cs"
if errorlevel 1 exit /b 1
echo built %OUT%
endlocal
