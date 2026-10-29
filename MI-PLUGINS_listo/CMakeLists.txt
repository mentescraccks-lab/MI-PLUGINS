cmake_minimum_required(VERSION 3.22)
project(LoudnessAnalyzer VERSION 0.2.0)

# JUCE: usa ./JUCE o -DJUCE_PATH si existen; si no, lo descarga automaticamente.
if(DEFINED JUCE_PATH AND EXISTS "${JUCE_PATH}/CMakeLists.txt")
    add_subdirectory("${JUCE_PATH}" JUCE)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.6
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(LoudnessAnalyzer
    COMPANY_NAME "LoudnessTools"
    BUNDLE_ID "com.loudnesstools.loudnessanalyzer"
    PLUGIN_MANUFACTURER_CODE LdTl
    PLUGIN_CODE LdA2
    FORMATS VST3
    PRODUCT_NAME "Loudness Analyzer"
    IS_MIDI_EFFECT FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_SYNTH FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Analyzer" "Tools"
)

target_sources(LoudnessAnalyzer PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/DSP/KWeightingFilter.cpp
    Source/DSP/KWeightingFilter.h
    Source/DSP/LoudnessMeter.cpp
    Source/DSP/LoudnessMeter.h
)

juce_generate_juce_header(LoudnessAnalyzer)

target_compile_features(LoudnessAnalyzer PRIVATE cxx_std_17)
target_compile_definitions(LoudnessAnalyzer PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(LoudnessAnalyzer PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
