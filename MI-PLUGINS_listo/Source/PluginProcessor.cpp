#include "PluginProcessor.h"
#include "PluginEditor.h"

LoudnessAnalyzerAudioProcessor::LoudnessAnalyzerAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void LoudnessAnalyzerAudioProcessor::prepareToPlay(double sampleRate, int)
{
    loudnessMeter.prepare(sampleRate, getTotalNumInputChannels());
    resetMeters();
}

void LoudnessAnalyzerAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    // Analyzer-only: meter reads the buffer; it intentionally does not write to it.
    loudnessMeter.processBlock(buffer);
    momentary.store(loudnessMeter.getMomentaryLUFS(), std::memory_order_relaxed);
    shortTerm.store(loudnessMeter.getShortTermLUFS(), std::memory_order_relaxed);
}

bool LoudnessAnalyzerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();
    if (input != output)
        return false;
    return input == juce::AudioChannelSet::mono()
        || input == juce::AudioChannelSet::stereo();
}

juce::AudioProcessorEditor* LoudnessAnalyzerAudioProcessor::createEditor()
{
    return new LoudnessAnalyzerAudioProcessorEditor(*this);
}

void LoudnessAnalyzerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    destData.reset();
}

void LoudnessAnalyzerAudioProcessor::setStateInformation(const void*, int) {}

void LoudnessAnalyzerAudioProcessor::resetMeters()
{
    loudnessMeter.reset();
    momentary.store(-100.0f, std::memory_order_relaxed);
    shortTerm.store(-100.0f, std::memory_order_relaxed);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LoudnessAnalyzerAudioProcessor();
}
