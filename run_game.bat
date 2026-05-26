@echo off
REM === Kirby's Adventure 一鍵執行 ===
SET PATH=C:\Qt\Qt5.12.12\5.12.12\mingw73_64\bin;C:\Qt\Qt5.12.12\Tools\mingw730_64\bin;%PATH%
cd /d "%~dp0build\release"
start KirbyAdventure.exe
