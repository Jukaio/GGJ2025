@echo off

rem Check if binpack.exe exists and if it's older than binpack.cpp using PowerShell
for /f "delims=" %%i in ('powershell -Command "(Get-Item 'meta\binpack.exe').LastWriteTime -lt (Get-Item 'meta\binpack.cpp').LastWriteTime"') do set result=%%i

if "%result%"=="True" (
    echo meta\binpack.exe is out of date, recompiling...
    rem Delete the .state file
    if exist "meta\.state" (
        echo Deleting meta\.state file...
        del /f "meta\.state"
    )
) else (
    echo meta\binpack.exe is up-to-date, skipping compilation.
    goto :run_binpack
)

rem Setup Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: Failed to invoke vcvarsall.bat. Ensure Visual Studio is properly installed.
    exit /b 1
)

rem Check if cl.exe exists
where cl >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: cl.exe not found in PATH after invoking vcvarsall.bat
    exit /b 1
)

rem Compile binpack.cpp
echo Building asset pipeline with cl.exe...
cl /std:c++17 meta\binpack.cpp /EHsc /Fo"meta\\" /Fe:meta\binpack.exe
if %ERRORLEVEL% NEQ 0 (
    echo Error: Failed to compile binpack with cl.exe
    exit /b 1
)

:run_binpack
rem Run the binpack executable
call "meta/binpack.exe"
