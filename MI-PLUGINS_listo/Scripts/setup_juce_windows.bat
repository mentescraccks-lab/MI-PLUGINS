@echo off
setlocal
cd /d "%~dp0\.."
where git >nul 2>nul
if errorlevel 1 (
  echo ERROR: Git no se encuentra en PATH. Instala Git for Windows.
  exit /b 1
)
if exist "JUCE\CMakeLists.txt" (
  echo JUCE ya parece estar instalado en la carpeta JUCE.
  exit /b 0
)
git clone https://github.com/juce-framework/JUCE.git JUCE
if errorlevel 1 exit /b 1
echo JUCE descargado. Ahora ejecuta Scripts\build_windows.bat
endlocal
