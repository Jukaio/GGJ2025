@echo off
if not exist "build/Engine.sln" (
    echo Solution not found. Generating...
    call gensln.bat
    start "" "build/Engine.sln"
) else (
    start "" "build/Engine.sln"
)
pause
