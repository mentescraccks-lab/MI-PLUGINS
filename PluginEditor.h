#pragma once

#include "PluginProcessor.h"
#include "GoldLookAndFeel.h"
#include "UiComponents.h"

class AndresBeatsClipAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    explicit AndresBeatsClipAudioProcessorEditor (AndresBeatsClipAudioProcessor&);
    ~AndresBeatsClipAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupKnob (juce::Slider&);

    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;

    GoldLookAndFeel laf;
    AndresBeatsClipAudioProcessor& proc;

    // Controles
    juce::Slider inputGain, ceiling, knee, mojo, alchemy, ceilTrim, dryWet, air;
    SliderAtt inputGainAtt, ceilingAtt, kneeAtt, mojoAtt, alchemyAtt, ceilTrimAtt, dryWetAtt, airAtt;

    // Textos de valor
    ValueBox inputGainBox, ceilingBox, kneeBox, mojoBox, alchemyBox, ceilTrimBox, dryWetBox;
    ValueBox kneeHeader, airValue;

    SegmentedSelector osSelector, modeSelector;
    ActiveButton activeButton;
    ButtonAtt activeAtt;

    GainReductionMeter grMeter;
    StereoMeter inMeter, outMeter;

    float dispInL = 0.0f, dispInR = 0.0f, dispOutL = 0.0f, dispOutR = 0.0f, dispGr = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndresBeatsClipAudioProcessorEditor)
};
