@echo off
if exist build\Release\gamplo_tracker.exe (
    cd build\Release && gamplo_tracker.exe
) else (
    echo Run setup.bat first!
    pause
)
