#pragma once
#include <JuceHeader.h>

// Prototype only. Not a validated BS.1770 K-weighting implementation.
class KWeightingFilter
{
public:
    void prepare(double sampleRate);
    void reset();
    float processSample(float input);
private:
    juce::dsp::IIR::Filter<float> shelf, highPass;
    juce::dsp::IIR::Coefficients<float>::Ptr shelfCoefficients, highPassCoefficients;
};
