@echo off
taskkill /f /im main.exe 2>nul
gcc \\host.lan\data\hmh\main.cpp -o \\host.lan\data\hmh\main.exe -luser32 -lgdi32
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Creating task to open game on windows vm...
    schtasks /run /tn RunHMH
) else (
    echo Build failed!
    exit /b 1
)
