#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class LoudnessAnalyzerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    explicit LoudnessAnalyzerAudioProcessorEditor(LoudnessAnalyzerAudioProcessor&);
    ~LoudnessAnalyzerAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    LoudnessAnalyzerAudioProcessor& audioProcessor;
    juce::Label titleLabel, momentaryLabel, shortTermLabel, statusLabel;
    juce::TextButton resetButton { "RESET" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoudnessAnalyzerAudioProcessorEditor)
};
