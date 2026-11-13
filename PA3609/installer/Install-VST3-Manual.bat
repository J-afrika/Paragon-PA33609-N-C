@echo off
:: Manual VST3 install (run as Administrator). Put this file next to the "Paragon PA-3609.vst3" folder.
net session >nul 2>&1 || (echo Please right-click and choose "Run as administrator". & pause & exit /b 1)
set "DEST=%CommonProgramFiles%\VST3"
if not exist "%DEST%" mkdir "%DEST%"
xcopy "%~dp0Paragon PA-3609.vst3" "%DEST%\Paragon PA-3609.vst3\" /E /I /Y
echo.
echo Installed to %DEST%. Rescan plugins in your DAW.
pause
