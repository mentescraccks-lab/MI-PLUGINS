#pragma once
#include <JuceHeader.h>
#include "KWeightingFilter.h"
#include <vector>

// Prototype rolling-window energy meter; not standards-compliant.
class LoudnessMeter
{
public:
    void prepare(double sampleRate, int channels);
    void reset();
    void processBlock(const juce::AudioBuffer<float>& buffer);
    float getMomentaryLUFS() const { return momentaryLUFS; }
    float getShortTermLUFS() const { return shortTermLUFS; }
private:
    double rate = 44100.0;
    int numChannels = 2;
    std::vector<KWeightingFilter> filters;
    std::vector<std::vector<float>> history;
    size_t writeIndex = 0;
    size_t maxHistory = 1;
    float momentaryLUFS = -100.0f, shortTermLUFS = -100.0f;
    float windowLUFS(size_t samples) const;
};
