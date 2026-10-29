@echo off
setlocal
cd /d "%~dp0\.."

where cmake >nul 2>nul
if errorlevel 1 (
  echo ERROR: CMake no se encuentra en PATH. Instala CMake y vuelve a abrir la terminal.
  exit /b 1
)

if not exist "JUCE\CMakeLists.txt" (
  echo ERROR: Falta la carpeta JUCE.
  echo Ejecuta: git clone https://github.com/juce-framework/JUCE.git JUCE
  exit /b 1
)

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
  echo.
  echo La configuracion fallo. Si el error menciona AAX, configura el AAX SDK de Avid
  echo o quita AAX temporalmente de FORMATS en CMakeLists.txt para compilar VST3.
  exit /b 1
)

cmake --build build --config Release
if errorlevel 1 (
  echo ERROR: La compilacion fallo. Revisa los mensajes anteriores.
  exit /b 1
)

echo.
echo Compilacion terminada. Busca los artefactos dentro de la carpeta build.
endlocal
