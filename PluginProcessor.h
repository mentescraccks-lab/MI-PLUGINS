#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace IDs
{
    inline constexpr const char* inputGain = "inputGain";
    inline constexpr const char* ceiling   = "ceiling";
    inline constexpr const char* knee      = "knee";
    inline constexpr const char* mojo      = "mojo";
    inline constexpr const char* alchemy   = "alchemy";
    inline constexpr const char* air       = "air";
    inline constexpr const char* ceilTrim  = "ceilTrim";
    inline constexpr const char* dryWet    = "dryWet";
    inline constexpr const char* mode      = "mode";
    inline constexpr const char* os        = "os";
    inline constexpr const char* active    = "active";
}

class AndresBeatsClipAudioProcessor : public juce::AudioProcessor
{
public:
    AndresBeatsClipAudioProcessor();
    ~AndresBeatsClipAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "AndresBeats Clip"; }
    bool acceptsMidi() const override  { return false; }
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

    juce::AudioProcessorValueTreeState apvts;

    // Medidores (el editor los lee ~30 veces por segundo)
    std::atomic<float> inPeakL { 0.0f }, inPeakR { 0.0f };
    std::atomic<float> outPeakL { 0.0f }, outPeakR { 0.0f };
    std::atomic<float> grDb { 0.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    void processChunk (juce::dsp::AudioBlock<float> block);
    void applyOversampling (int newIndex);
    void setupSmoothing();
    juce::dsp::Oversampling<float>* getOversampler() noexcept;

    std::atomic<float>* pInputGain = nullptr;
    std::atomic<float>* pCeiling   = nullptr;
    std::atomic<float>* pKnee      = nullptr;
    std::atomic<float>* pMojo      = nullptr;
    std::atomic<float>* pAlchemy   = nullptr;
    std::atomic<float>* pAir       = nullptr;
    std::atomic<float>* pCeilTrim  = nullptr;
    std::atomic<float>* pDryWet    = nullptr;
    std::atomic<float>* pMode      = nullptr;
    std::atomic<float>* pOs        = nullptr;
    std::atomic<float>* pActive    = nullptr;

    // Oversampling lineal (FIR equiripple) 2x y 4x. 1x = sin oversampling.
    juce::dsp::Oversampling<float> os2x { 2, 1, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true };
    juce::dsp::Oversampling<float> os4x { 2, 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true };

    juce::dsp::DryWetMixer<float> mixer;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> airFilter;

    using Smoothed = juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>;
    Smoothed inSm, outSm;                       // a frecuencia normal
    Smoothed ceilSm, kneeSm, mojoSm, alchSm;    // a frecuencia sobremuestreada

    double sampleRate = 44100.0;
    int maxBlock = 512;
    int osIndex = -1;
    int osFactor = 1;
    float lastAir = -1.0f;
    float dcR = 0.999f;
    float dcX[2] { 0.0f, 0.0f };
    float dcY[2] { 0.0f, 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndresBeatsClipAudioProcessor)
};
