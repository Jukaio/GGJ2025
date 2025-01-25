@echo off

if exist "meta\binpack.exe" (
    echo meta\binpack.exe already exists, skipping compilation.
) else (

    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>nul
    if %ERRORLEVEL% NEQ 0 (
        echo Error: Failed to invoke vcvarsall.bat. Ensure Visual Studio is properly installed.
        exit /b 1
    )

    where cl >nul 2>nul
    if %ERRORLEVEL% NEQ 0 (
        echo Error: cl.exe not found in PATH after invoking vcvarsall.bat
        exit /b 1
    )

    echo Building asset pipeline with cl.exe...
    cl /std:c++17 meta\binpack.cpp /EHsc /Fo"meta\\" /Fe:meta\binpack.exe
    if %ERRORLEVEL% NEQ 0 (
        echo Error: Failed to compile binpack with cl.exe
        exit /b 1
    )

)

call "meta/binpack.exe"
