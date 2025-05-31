@echo off
setlocal enabledelayedexpansion

:: Breakout Games Collection - All-in-one launcher
:: This batch file will automatically set up and run the games

echo =========================================================
echo            BREAKOUT GAMES COLLECTION LAUNCHER
echo =========================================================
echo.
echo This launcher will automatically set up everything needed
echo to run the games on your Windows computer.
echo.
echo Please wait while we check your system...
echo.

:: Create directories if they don't exist
if not exist "DLLs" mkdir DLLs
if not exist "sounds" mkdir sounds
if not exist "temp" mkdir temp

:: Check if we have all required DLLs
set NEED_DOWNLOAD=0
if not exist "SDL2.dll" set NEED_DOWNLOAD=1
if not exist "SDL2_ttf.dll" set NEED_DOWNLOAD=1
if not exist "SDL2_mixer.dll" set NEED_DOWNLOAD=1

:: Download DLLs if needed
if "%NEED_DOWNLOAD%"=="1" (
    echo Some required files are missing. Downloading them now...
    echo This will only happen once.
    echo.

    :: Use PowerShell to download files
    echo Downloading SDL2 libraries...
    powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://www.libsdl.org/release/SDL2-2.0.22-win32-x86.zip' -OutFile 'temp\sdl2.zip'}"
    powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.18-win32-x86.zip' -OutFile 'temp\sdl2_ttf.zip'}"
    powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.4-win32-x86.zip' -OutFile 'temp\sdl2_mixer.zip'}"

    :: Extracting DLLs (using PowerShell to unzip)
    echo Extracting files...
    powershell -Command "& {Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::ExtractToDirectory('temp\sdl2.zip', 'temp\sdl2')}"
    powershell -Command "& {Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::ExtractToDirectory('temp\sdl2_ttf.zip', 'temp\sdl2_ttf')}"
    powershell -Command "& {Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::ExtractToDirectory('temp\sdl2_mixer.zip', 'temp\sdl2_mixer')}"

    :: Copy DLLs to main directory
    echo Copying files to the correct location...
    copy /Y "temp\sdl2\*.dll" "."
    copy /Y "temp\sdl2_ttf\*.dll" "."
    copy /Y "temp\sdl2_mixer\*.dll" "."
    
    :: Also save them in DLLs folder for backup
    copy /Y "temp\sdl2\*.dll" "DLLs\"
    copy /Y "temp\sdl2_ttf\*.dll" "DLLs\"
    copy /Y "temp\sdl2_mixer\*.dll" "DLLs\"

    echo Cleaning up temporary files...
    rmdir /S /Q temp
    
    echo Download and setup complete!
) else (
    echo All required libraries are already installed.
)

:: Check for required executables
set HAVE_BREAKOUT=0
set HAVE_TERMINAL=0

if exist "breakout.exe" set HAVE_BREAKOUT=1
if exist "terminal_maze.exe" set HAVE_TERMINAL=1

:: Check for font file
if not exist "Arial.ttf" (
    echo Font file not found. Downloading...
    powershell -Command "& {[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; Invoke-WebRequest -Uri 'https://github.com/googlefonts/roboto/raw/main/src/hinted/Roboto-Regular.ttf' -OutFile 'Arial.ttf'}"
    echo Font downloaded.
)

:: Check for sound files
if not exist "sounds\paddle_hit.wav" (
    echo Creating placeholder sound files...
    echo. > "sounds\paddle_hit.wav"
    echo. > "sounds\block_hit.wav"
    echo. > "sounds\power_up.wav"
    echo. > "sounds\level_complete.wav"
    echo. > "sounds\game_over.wav"
    echo. > "sounds\menu_select.wav"
    echo. > "sounds\menu_click.wav"
    echo Sound files created.
)

echo.
echo =========================================================
echo.

:menu
echo Please select a game to play:
echo.
echo 1. Breakout Game (brick-breaking game with power-ups)
if "%HAVE_BREAKOUT%"=="0" echo    [NOT FOUND]
echo 2. Terminal Maze (text-based maze navigation)
if "%HAVE_TERMINAL%"=="0" echo    [NOT FOUND]
echo 3. Exit
echo.

set /p choice=Enter your choice (1-3): 

if "%choice%"=="1" (
    if "%HAVE_BREAKOUT%"=="1" (
        echo Starting Breakout Game...
        start "" "breakout.exe"
    ) else (
        echo.
        echo Breakout Game executable not found.
        echo Would you like to download a pre-compiled version? (Y/N)
        set /p download=
        if /i "!download!"=="Y" (
            echo This feature is not implemented yet.
            echo Please visit the project website to download the latest version.
            pause
        )
    )
    goto menu
) else if "%choice%"=="2" (
    if "%HAVE_TERMINAL%"=="1" (
        echo Starting Terminal Maze...
        start "" "terminal_maze.exe"
    ) else (
        echo.
        echo Terminal Maze executable not found.
        echo Would you like to download a pre-compiled version? (Y/N)
        set /p download=
        if /i "!download!"=="Y" (
            echo This feature is not implemented yet.
            echo Please visit the project website to download the latest version.
            pause
        )
    )
    goto menu
) else if "%choice%"=="3" (
    echo Thank you for playing!
    exit /b
) else (
    echo Invalid choice. Please try again.
    goto menu
)

endlocal

