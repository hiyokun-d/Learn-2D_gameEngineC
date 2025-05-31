@echo off
echo Setting up SDL2 games...
echo.

if not exist bin (
    mkdir bin
)

echo Copying DLLs to bin folder...
copy /Y dlls\*.dll bin\
copy /Y Arial.ttf bin\

echo.
echo Setup complete! You can now run the games using run_games.bat
pause
