@echo off
setlocal EnableExtensions EnableDelayedExpansion

set "PROJECT_ROOT=%~dp0"
set "KEYBOARD=icst7789v"
set "KEYMAP=default"
set "BUILD_DIR=%PROJECT_ROOT%build"

echo.
echo === ICST7789V QMK UF2 build ===

rem Prefer an explicitly configured QMK_HOME. Otherwise try a few common locations.
if defined QMK_HOME goto :have_qmk_home
if exist "%PROJECT_ROOT%qmk_firmware\keyboards" set "QMK_HOME=%PROJECT_ROOT%qmk_firmware"
if defined QMK_HOME goto :have_qmk_home
if exist "%PROJECT_ROOT%..\qmk_firmware\keyboards" set "QMK_HOME=%PROJECT_ROOT%..\qmk_firmware"
if defined QMK_HOME goto :have_qmk_home
if exist "%USERPROFILE%\qmk_firmware\keyboards" set "QMK_HOME=%USERPROFILE%\qmk_firmware"
if defined QMK_HOME goto :have_qmk_home
if exist "C:\qmk_firmware\keyboards" set "QMK_HOME=C:\qmk_firmware"

echo ERROR: QMK firmware checkout not found.
echo Set QMK_HOME to your qmk_firmware folder, for example:
echo   set QMK_HOME=C:\GitHub\qmk_firmware
exit /b 2

:have_qmk_home
if not exist "%QMK_HOME%\keyboards" (
    echo ERROR: QMK_HOME does not look like a QMK firmware checkout:
    echo   %QMK_HOME%
    exit /b 3
)

where qmk >nul 2>&1
if errorlevel 1 (
    echo ERROR: The qmk command is not available in PATH.
    echo Run this BAT from a QMK-capable shell or install/configure QMK CLI first.
    exit /b 4
)

echo QMK_HOME: %QMK_HOME%
echo Installing keyboard definition...

set "TARGET=%QMK_HOME%\keyboards\%KEYBOARD%"
if exist "%TARGET%" rmdir /s /q "%TARGET%"
xcopy "%PROJECT_ROOT%keyboards\%KEYBOARD%" "%TARGET%\" /E /I /Y >nul
if errorlevel 1 (
    echo ERROR: Could not copy the keyboard definition into QMK.
    exit /b 5
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
del /q "%BUILD_DIR%\*.uf2" >nul 2>&1

pushd "%QMK_HOME%"
echo Building %KEYBOARD%:%KEYMAP% ...
qmk compile -kb %KEYBOARD% -km %KEYMAP%
set "RESULT=%ERRORLEVEL%"
if not "%RESULT%"=="0" (
    popd
    echo ERROR: QMK build failed with exit code %RESULT%.
    exit /b %RESULT%
)

set "UF2=%QMK_HOME%\%KEYBOARD%_%KEYMAP%.uf2"
if not exist "%UF2%" (
    set "NEWEST="
    for /f "delims=" %%F in ('dir /b /a-d /o-d "*.uf2" 2^>nul') do if not defined NEWEST set "NEWEST=%%F"
    if defined NEWEST set "UF2=%QMK_HOME%\!NEWEST!"
)
popd

if not exist "%UF2%" (
    echo ERROR: Build succeeded, but no UF2 file was found.
    exit /b 6
)

copy /y "%UF2%" "%BUILD_DIR%\ICST7789V.uf2" >nul
if errorlevel 1 (
    echo ERROR: Could not copy the UF2 into the build folder.
    exit /b 7
)

echo.
echo SUCCESS:
echo   %BUILD_DIR%\ICST7789V.uf2
echo.
exit /b 0
