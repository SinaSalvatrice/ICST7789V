@echo off
setlocal EnableExtensions

set "PROJECT_ROOT=%~dp0"
set "PROJECT_ROOT=%PROJECT_ROOT:~0,-1%"

if not defined QMK_HOME set "QMK_HOME=%USERPROFILE%\qmk_firmware"
if not exist "%QMK_HOME%\Makefile" (
    echo ERROR: QMK firmware checkout not found:
    echo   %QMK_HOME%
    echo.
    echo Set QMK_HOME first if your checkout is elsewhere.
    exit /b 2
)

set "QMK_BASH=C:\QMK_MSYS\usr\bin\bash.exe"
if not exist "%QMK_BASH%" (
    echo ERROR: QMK MSYS was not found at:
    echo   %QMK_BASH%
    exit /b 3
)

echo.
echo === ICST7789V QMK UF2 build ===
echo Regenerating layer UI and keymap from design\layer_system.json...
where py >nul 2>nul
if not errorlevel 1 (
    py -3 "%PROJECT_ROOT%\scripts\generate_layer_system.py"
) else (
    python "%PROJECT_ROOT%\scripts\generate_layer_system.py"
)
if errorlevel 1 (
    echo ERROR: Layer metadata generation failed.
    exit /b 4
)
echo QMK_HOME: %QMK_HOME%
echo.

rem Use a clean non-interactive MSYS shell. The Windows environment variables
rem are inherited and converted to POSIX paths inside the shell.
"%QMK_BASH%" --noprofile --norc -c "export HOME=\"$(cygpath -u \"$USERPROFILE\")\"; export PROJECT_ROOT=\"$(cygpath -u \"$PROJECT_ROOT\")\"; export QMK_HOME=\"$(cygpath -u \"$QMK_HOME\")\"; export PATH=\"$HOME/bin:/c/QMK_MSYS/mingw64/bin:/c/QMK_MSYS/usr/bin:/opt/qmk/bin:$PATH\"; bash \"$PROJECT_ROOT/build_uf2.sh\""
set "RESULT=%ERRORLEVEL%"

if not "%RESULT%"=="0" (
    echo.
    echo ERROR: Build failed with exit code %RESULT%.
    exit /b %RESULT%
)

echo.
echo UF2 ready:
echo   %PROJECT_ROOT%\build\ICST7789V.uf2
echo.
exit /b 0
