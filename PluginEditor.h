#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Custom analog-style knob: machined metal face, dark knurled rim and gold pointer.
class AndresBeatsAnalogLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;
};

class AndresBeatsClipAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit AndresBeatsClipAudioProcessorEditor (AndresBeatsClipAudioProcessor&);
    ~AndresBeatsClipAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using Slider = juce::Slider;
    using Attach = juce::AudioProcessorValueTreeState::SliderAttachment;
    AndresBeatsClipAudioProcessor& audioProcessor;
    AndresBeatsAnalogLookAndFeel analogLookAndFeel;
    juce::Label title, subtitle, coreTitle, inputTitle, outputTitle, footer;
    juce::Label inputValue, ceilingValue, kneeValue, mojoValue, alchemyValue, trimValue, mixValue, airValue;
    Slider input, ceiling, knee, mojo, alchemy, trim, mix, air;
    std::unique_ptr<Attach> inputAttach, ceilingAttach, kneeAttach, mojoAttach, alchemyAttach, trimAttach, mixAttach, airAttach;
    juce::ComboBox mode, oversampling;
    juce::ToggleButton active;
    juce::Label inputMeterLabel, outputMeterLabel, grLabel;
    float inDb = -60.0f, outDb = -60.0f, grDb = 0.0f;
    void timerCallback() override;
    void setupKnob (Slider&, const juce::String&, bool rotary = true);
    void setupLabel (juce::Label&, const juce::String&, int size, juce::Colour colour, bool bold = false);
    void drawMeter (juce::Graphics&, juce::Rectangle<float>, float, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndresBeatsClipAudioProcessorEditor)
};
