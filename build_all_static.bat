@echo off
echo ================================================
echo Building Omega Engine Demos - Static Linking
echo ================================================
echo.

REM Add MinGW to PATH
set PATH=C:\msys64\mingw64\bin;%PATH%

REM Compiler flags for static linking
set CFLAGS=-std=c++17 -O3 -static-libgcc -static-libstdc++ -static

echo [1/8] Building omega_demo.exe...
g++ %CFLAGS% -o omega_demo.exe omega_demo.cpp
if %ERRORLEVEL% NEQ 0 goto :error

echo [2/8] Building dungeon_quest.exe...
g++ %CFLAGS% -o dungeon_quest.exe dungeon_quest.cpp
if %ERRORLEVEL% NEQ 0 goto :error

echo [3/8] Building dungeon_quest_fixed.exe...
g++ %CFLAGS% -o dungeon_quest_fixed.exe dungeon_quest_fixed.cpp
if %ERRORLEVEL% NEQ 0 goto :error

echo [4/8] Building dungeon_quest_v2.exe...
g++ %CFLAGS% -o dungeon_quest_v2.exe dungeon_quest_v2.cpp
if %ERRORLEVEL% NEQ 0 goto :error

echo [5/8] Building dungeon_quest_v3.exe...
g++ %CFLAGS% -o dungeon_quest_v3.exe dungeon_quest_v3.cpp
if %ERRORLEVEL% NEQ 0 goto :error

echo [6/8] Building dungeon_quest_v3_1.exe...
g++ %CFLAGS% -o dungeon_quest_v3_1.exe dungeon_quest_v3_1.cpp
if %ERRORLEVEL% NEQ 0 goto :error

echo [7/8] Building dungeon_quest_v3_2.exe...
g++ %CFLAGS% -o dungeon_quest_v3_2.exe dungeon_quest_v3_2.cpp
if %ERRORLEVEL% NEQ 0 goto :error

echo [8/8] Building dungeon_quest_v4_lightning.exe...
g++ %CFLAGS% -o dungeon_quest_v4_lightning.exe dungeon_quest_v4_lightning.cpp
if %ERRORLEVEL% NEQ 0 goto :error

echo.
echo ================================================
echo SUCCESS! All executables built with static linking
echo ================================================
echo.
echo All .exe files are now portable and do not require DLLs!
echo You can copy them to any Windows computer and run directly.
echo.
echo File sizes:
dir /B *.exe | findstr "dungeon\|omega"
echo.
pause
goto :end

:error
echo.
echo ================================================
echo ERROR! Build failed
echo ================================================
echo.
echo Check that:
echo  - MinGW is installed at C:\msys64\mingw64\bin
echo  - All .cpp source files exist
echo  - You have write permissions
echo.
pause
exit /b 1

:end
