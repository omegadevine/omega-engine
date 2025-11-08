@echo off
REM Quick commit and push script for Omega Engine
REM Usage: quick-commit.bat "Your commit message"

cd /d "%~dp0"

if "%~1"=="" (
    echo Error: Please provide a commit message
    echo Usage: quick-commit.bat "Your commit message"
    exit /b 1
)

echo [1/4] Checking git status...
git status

echo.
echo [2/4] Adding changes...
git add .

echo.
echo [3/4] Committing changes...
git commit -m "%~1"

echo.
echo [4/4] Pushing to origin/main...
git push origin main

echo.
echo Done! Changes have been pushed to GitHub.
pause
