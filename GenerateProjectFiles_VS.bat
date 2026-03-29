@echo off
setlocal

set UE_VERSION=5.5
set ENGINE_PATH=E:\UE_%UE_VERSION%
set UPROJECT_FILE=

echo.
echo ================================================
echo   Unreal Engine %UE_VERSION% - Generate Project Files
echo   IDE: Visual Studio
echo ================================================
echo.

if "%UPROJECT_FILE%"=="" (
    for %%f in ("%~dp0*.uproject") do set UPROJECT_FILE=%%f
)

if "%UPROJECT_FILE%"=="" (
    echo [ERROR] Cannot find .uproject file.
    pause
    exit /b 1
)

echo [INFO] Project : %UPROJECT_FILE%
echo [INFO] Engine  : %ENGINE_PATH%
echo.

set UBT=%ENGINE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe

if not exist "%UBT%" (
    echo [ERROR] UnrealBuildTool not found:
    echo         %UBT%
    pause
    exit /b 1
)

echo [INFO] Generating Visual Studio project files...
echo.

"%UBT%" -projectfiles -project="%UPROJECT_FILE%" -game -rocket -progress -2022

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Done! Open the .sln file with Visual Studio.
) else (
    echo.
    echo [ERROR] Failed. ErrorLevel: %ERRORLEVEL%
)

echo.
pause
endlocal
