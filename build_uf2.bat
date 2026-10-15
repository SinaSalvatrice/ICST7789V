@echo off
setlocal EnableExtensions

set "PROJECT_ROOT=%~dp0"
set "PROJECT_ROOT_NO_SLASH=%PROJECT_ROOT:~0,-1%"

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
echo QMK_HOME: %QMK_HOME%
echo.

"%QMK_BASH%" -lc "PROJECT_ROOT='$(cygpath -u '%PROJECT_ROOT_NO_SLASH%')' QMK_HOME='$(cygpath -u '%QMK_HOME%')' bash '$(cygpath -u '%PROJECT_ROOT_NO_SLASH%')/build_uf2.sh'"
set "RESULT=%ERRORLEVEL%"

if not "%RESULT%"=="0" (
    echo.
    echo ERROR: Build failed with exit code %RESULT%.
    exit /b %RESULT%
)

echo.
echo UF2 ready:
echo   %PROJECT_ROOT%build\ICST7789V.uf2
echo.
exit /b 0
