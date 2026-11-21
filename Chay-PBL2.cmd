@echo off
setlocal
cd /d "%~dp0"
if not exist "build\qt-phase1\pbl2_gui.exe" (
    echo Chua tim thay build\qt-phase1\pbl2_gui.exe. Hay build lai giao dien.
    pause
    exit /b 1
)
set "PATH=C:\Qt\6.11.2\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;%PATH%"
start "" "build\qt-phase1\pbl2_gui.exe"
