@echo off
REM === Kirby's Adventure 一鍵編譯+執行 ===
SET PATH=C:\Qt\Qt5.12.12\5.12.12\mingw73_64\bin;C:\Qt\Qt5.12.12\Tools\mingw730_64\bin;%PATH%

echo [1/3] Running qmake...
if not exist build mkdir build
cd build
qmake ..\KirbyAdventure.pro -spec win32-g++
if errorlevel 1 (echo qmake failed! & pause & exit /b 1)

echo [2/3] Compiling...
mingw32-make -j4
if errorlevel 1 (echo Compile failed! & pause & exit /b 1)

echo [3/3] Copying Dataset...
if not exist release\Dataset (
    xcopy /E /I /Q "..\Dataset" "release\Dataset"
)

echo.
echo ============================
echo  Build successful!
echo  Launching game...
echo ============================
cd release
start KirbyAdventure.exe
