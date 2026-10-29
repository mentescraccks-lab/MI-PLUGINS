# Loudness Analyzer — proyecto fuente

Este ZIP contiene el proyecto JUCE/CMake del plugin **Loudness Analyzer**, concebido como analizador (no debería modificar el audio que atraviesa el plugin).

## Importante: qué incluye este ZIP

- Código fuente C++ y GUI.
- Configuración de CMake para generar **VST3**, **AAX** y una aplicación Standalone.
- Scripts de ayuda para configurar y compilar en Windows.
- No incluye binarios `.vst3` ni `.aax` ya compilados. Esos artefactos deben generarse con un compilador y los SDK correspondientes en tu sistema operativo.
- El DSP que contiene esta versión es un **prototipo inicial**, no un medidor validado contra los vectores de referencia de ITU-R BS.1770/EBU R128. No uses sus valores para entrega de broadcast, certificación o decisiones críticas de mastering.

## Requisitos

### Windows
1. Visual Studio 2022 o posterior, con la carga **Desarrollo para el escritorio con C++** y un Windows SDK.
2. CMake 3.22 o posterior.
3. Git.
4. JUCE (este proyecto espera una carpeta `JUCE` dentro de la carpeta del proyecto).
5. Para generar AAX: el **AAX SDK oficial de Avid**, aceptando sus términos y configurando la ruta del SDK según las instrucciones de JUCE/Avid. El SDK no se incluye en este ZIP.

### macOS
Xcode, CMake, JUCE y, para AAX, el SDK oficial de Avid.

## Configuración inicial

Abre una terminal dentro de la carpeta del proyecto y ejecuta:

```bash
git clone https://github.com/juce-framework/JUCE.git JUCE
```

Si ya tienes JUCE en otra ubicación, puedes pasar su ruta con `-DJUCE_PATH=/ruta/a/JUCE`.

## Compilar en Windows

Puedes usar `Scripts\\build_windows.bat` desde una terminal de Windows. El script configura y compila Release.

También puedes ejecutar manualmente:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Los productos se generan dentro de `build`. La ubicación exacta depende de la versión de CMake/JUCE y del generador. Busca el artefacto `Loudness Analyzer.vst3` y, si el SDK AAX está instalado y detectado por JUCE, el artefacto AAX.

Si la configuración falla al intentar generar AAX, comprueba primero que el SDK de Avid está instalado y configurado correctamente. Como diagnóstico temporal puedes editar `CMakeLists.txt` y quitar `AAX` de `FORMATS`; esto permite compilar VST3 sin el SDK AAX.

## DSP incluido y limitaciones

La arquitectura separa procesador, editor, filtro K-weighting y medidor. La lectura de valores se comparte con la GUI mediante atomics y el procesamiento de audio no debe alterar intencionalmente el buffer.

Estado del medidor en esta versión:
- Momentary LUFS: aproximación/prototipo.
- Short-term LUFS: aproximación/prototipo.
- Integrated LUFS: no implementado de forma conforme.
- True Peak: no implementado.
- LRA: no implementado.
- No se han ejecutado pruebas de conformidad oficiales.

Antes de usarlo profesionalmente, faltan coeficientes exactos y verificados de K-weighting, ponderación de canales y exclusiones correspondientes, gating absoluto/relativo para LUFS-I, sobremuestreo para True Peak, cálculo LRA y pruebas con señales de referencia conocidas.

## Seguridad y uso

El plugin se ofrece como proyecto de desarrollo. No se garantiza compatibilidad con todas las versiones de Pro Tools/DAW ni que los valores sean conformes. AAX requiere el SDK oficial de Avid y puede estar sujeto a condiciones adicionales de firma, validación y distribución.
