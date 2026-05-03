@echo off
gcc \\host.lan\\data\hmh\main.c -o main.exe -luser32 -lgdi32
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
) else (
    echo Build failed!
    exit /b 1
)
