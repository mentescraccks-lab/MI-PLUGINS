#pragma once
#include <JuceHeader.h>

class AndresBeatsClipAudioProcessor : public juce::AudioProcessor
{
public:
    AndresBeatsClipAudioProcessor();
    ~AndresBeatsClipAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;
    std::atomic<float> inputPeak { 0.0f }, outputPeak { 0.0f }, gainReduction { 0.0f };

    static APVTS::ParameterLayout createParameterLayout();

private:
    float shapeSample (float x, float ceiling, float knee, int mode, float mojo, float alchemy) const noexcept;
    juce::dsp::Oversampling<float> oversampling;
    double currentSampleRate = 44100.0;
    float airStateL = 0.0f, airStateR = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndresBeatsClipAudioProcessor)
};
