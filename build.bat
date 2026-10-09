@echo off
rem ==========================================================================
rem build.bat - Build project Kalkulator Rangkaian Elektronika (ASM + C)
rem Kebutuhan: NASM (nasm) dan GCC MinGW-w64 (gcc) sudah ada di PATH.
rem Pakai:    build.bat        -> build GUI (kalkulator_rangkaian.exe)
rem           build.bat test   -> build + jalankan uji logika assembly
rem ==========================================================================
setlocal
cd /d "%~dp0"

where nasm >nul 2>nul
if errorlevel 1 (
    echo [ERROR] NASM tidak ditemukan di PATH. Install dari https://www.nasm.us
    exit /b 1
)
where gcc >nul 2>nul
if errorlevel 1 (
    echo [ERROR] GCC MinGW-w64 tidak ditemukan di PATH.
    exit /b 1
)

echo [1/2] Assembling ohm.asm ...
nasm -f win64 ohm.asm -o ohm.obj
if errorlevel 1 (
    echo [ERROR] Assembly gagal.
    exit /b 1
)

echo [2/2] Compiling + linking main.c ...
gcc -O2 -Wall -municode -mwindows main.c ohm.obj -o kalkulator_rangkaian.exe -lgdi32
if errorlevel 1 (
    echo [ERROR] Build GUI gagal.
    exit /b 1
)

echo.
echo Build sukses: kalkulator_rangkaian.exe

if /i "%~1"=="test" (
    echo.
    echo --- Menjalankan uji logika assembly ---
    gcc -O2 -Wall test_ohm.c ohm.obj -o test_ohm.exe
    if errorlevel 1 (
        echo [ERROR] Build uji gagal.
        exit /b 1
    )
    test_ohm.exe
    exit /b %ERRORLEVEL%
)

endlocal
