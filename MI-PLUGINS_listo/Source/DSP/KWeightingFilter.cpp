#include "KWeightingFilter.h"

void KWeightingFilter::prepare(double sampleRate)
{
    juce::dsp::ProcessSpec spec { sampleRate, 1, 1 };
    shelf.prepare(spec);
    highPass.prepare(spec);
    // Generic approximations for scaffolding only; replace with verified BS.1770 coefficients.
    shelfCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        sampleRate, 1681.974, 0.707, 1.5849);
    highPassCoefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(
        sampleRate, 38.0, 0.5);
    *shelf.coefficients = *shelfCoefficients;
    *highPass.coefficients = *highPassCoefficients;
}
void KWeightingFilter::reset() { shelf.reset(); highPass.reset(); }
float KWeightingFilter::processSample(float input)
{
    return highPass.processSample(shelf.processSample(input));
}
