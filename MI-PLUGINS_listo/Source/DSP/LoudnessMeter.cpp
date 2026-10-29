#include "LoudnessMeter.h"
#include <cmath>

void LoudnessMeter::prepare(double sampleRate, int channels)
{
    rate = sampleRate;
    numChannels = juce::jmax(1, channels);
    maxHistory = static_cast<size_t>(juce::jmax(1.0, rate * 3.0));
    filters.clear();
    filters.resize(static_cast<size_t>(numChannels));
    for (auto& f : filters) f.prepare(rate);
    history.assign(static_cast<size_t>(numChannels), std::vector<float>(maxHistory, 0.0f));
    reset();
}
void LoudnessMeter::reset()
{
    for (auto& f : filters) f.reset();
    for (auto& channel : history) std::fill(channel.begin(), channel.end(), 0.0f);
    writeIndex = 0;
    momentaryLUFS = shortTermLUFS = -100.0f;
}
void LoudnessMeter::processBlock(const juce::AudioBuffer<float>& buffer)
{
    const int channels = juce::jmin(numChannels, buffer.getNumChannels());
    const int samples = buffer.getNumSamples();
    for (int i = 0; i < samples; ++i) {
        for (int ch = 0; ch < channels; ++ch)
            history[static_cast<size_t>(ch)][writeIndex] =
                filters[static_cast<size_t>(ch)].processSample(buffer.getReadPointer(ch)[i]);
        writeIndex = (writeIndex + 1) % maxHistory;
    }
    momentaryLUFS = windowLUFS(static_cast<size_t>(rate * 0.4));
    shortTermLUFS = windowLUFS(static_cast<size_t>(rate * 3.0));
}
float LoudnessMeter::windowLUFS(size_t samples) const
{
    if (history.empty()) return -100.0f;
    samples = juce::jmin(samples, maxHistory);
    double energy = 0.0;
    size_t count = 0;
    for (const auto& channel : history) {
        for (size_t i = 0; i < samples; ++i) {
            const size_t idx = (writeIndex + maxHistory - 1 - i) % maxHistory;
            const double v = channel[idx];
            energy += v * v;
            ++count;
        }
    }
    if (count == 0 || energy <= 1.0e-20) return -100.0f;
    // Placeholder mapping; does not implement official channel weights/gating.
    return static_cast<float>(-0.691 + 10.0 * std::log10(energy / static_cast<double>(count)));
}
