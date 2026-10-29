cmake_minimum_required(VERSION 3.22)
project(AndresBeatsClip VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

set(AB_FORMATS VST3)
if(APPLE)
    list(APPEND AB_FORMATS AU)
endif()

juce_add_plugin(AndresBeatsClip
    COMPANY_NAME "AndresBeats"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_MANUFACTURER_CODE AnBt
    PLUGIN_CODE AbCl
    FORMATS ${AB_FORMATS}
    PRODUCT_NAME "AndresBeats Clip")

target_sources(AndresBeatsClip PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h)

target_compile_features(AndresBeatsClip PRIVATE cxx_std_17)

target_compile_definitions(AndresBeatsClip PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(AndresBeatsClip
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
