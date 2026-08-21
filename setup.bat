@echo off
title Gamplo Tracker - Build
echo.
echo =============================================
echo  Gamplo Achievement Tracker - Build
echo =============================================
echo.
echo NOTE: Run this from "Developer Command Prompt for VS"
echo       (search your Start Menu for it)
echo.

:: ── Download SDL2 if missing ──────────────────────────────────
if not exist vendor\SDL2\include\SDL.h (
    echo [1/2] Downloading SDL2...
    powershell -NoProfile -Command ^
        "Invoke-WebRequest -Uri 'https://github.com/libsdl-org/SDL/releases/download/release-2.30.0/SDL2-devel-2.30.0-VC.zip' -OutFile 'vendor\SDL2.zip' -UseBasicParsing"
    if errorlevel 1 (
        echo FAILED to download SDL2. Check your internet connection.
        pause & exit /b 1
    )
    powershell -NoProfile -Command ^
        "Expand-Archive -Path 'vendor\SDL2.zip' -DestinationPath 'vendor\SDL2_tmp' -Force"
    move vendor\SDL2_tmp\SDL2-2.30.0 vendor\SDL2 >nul
    rmdir /s /q vendor\SDL2_tmp
    del vendor\SDL2.zip
    echo     OK
) else (
    echo [1/2] SDL2 already present.
)

:: ── Configure and build ───────────────────────────────────────
echo [2/2] Building...
if not exist build mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 ( echo CMake failed. & cd .. & pause & exit /b 1 )
cmake --build . --config Release
if errorlevel 1 ( echo Build failed. & cd .. & pause & exit /b 1 )
cd ..

echo.
echo =============================================
echo  Done! Run:  build\Release\gamplo_tracker.exe
echo  Or just:    run.bat
echo =============================================
echo.
pause
