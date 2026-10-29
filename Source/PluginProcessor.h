#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

class AndresBeatsClipAudioProcessor : public juce::AudioProcessor
{
public:
    AndresBeatsClipAudioProcessor();
    ~AndresBeatsClipAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "AndresBeats Clip"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ValueTreeState para automatización en el DAW
    juce::AudioProcessorValueTreeState apvts;

    // Telemetría accesible por el Editor
    std::atomic<float> clipReductionDb { 0.0f };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Filtros High-Shelf para "High Air Shimmer"
    juce::dsp::IIR::Filter<float> highShelfFilterL;
    juce::dsp::IIR::Filter<float> highShelfFilterR;

    // DC Blocker states
    float dcX1L = 0.0f, dcY1L = 0.0f;
    float dcX1R = 0.0f, dcY1R = 0.0f;

    // Oversampling 2x
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    // DSP Clipper Algorithm
    float processClipperSample (float x, float ceiling, float kneeNorm, int mode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndresBeatsClipAudioProcessor)
};
