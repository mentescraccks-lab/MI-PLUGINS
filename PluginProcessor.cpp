#include "PluginProcessor.h"
#include "PluginEditor.h"

AndresBeatsClipAudioProcessor::AndresBeatsClipAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout()),
      oversampling (2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true)
{
}

AndresBeatsClipAudioProcessor::APVTS::ParameterLayout AndresBeatsClipAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout p;
    auto add = [&p](std::unique_ptr<juce::RangedAudioParameter> param) { p.add(std::move(param)); };
    add(std::make_unique<juce::AudioParameterFloat>("input", "Input Gain", -18.0f, 18.0f, 0.0f));
    add(std::make_unique<juce::AudioParameterFloat>("ceiling", "Ceiling", -12.0f, 0.0f, 0.0f));
    add(std::make_unique<juce::AudioParameterFloat>("knee", "Knee Soft", 0.0f, 100.0f, 15.0f));
    add(std::make_unique<juce::AudioParameterFloat>("mojo", "Gold Mojo", 0.0f, 100.0f, 25.0f));
    add(std::make_unique<juce::AudioParameterFloat>("alchemy", "Alchemy", 0.0f, 100.0f, 10.0f));
    add(std::make_unique<juce::AudioParameterFloat>("air", "High Air Shimmer", 0.0f, 100.0f, 15.0f));
    add(std::make_unique<juce::AudioParameterFloat>("trim", "Ceil Trim", -12.0f, 12.0f, 0.0f));
    add(std::make_unique<juce::AudioParameterFloat>("mix", "Dry Wet", 0.0f, 100.0f, 100.0f));
    add(std::make_unique<juce::AudioParameterChoice>("mode", "Mode", juce::StringArray{"Modern", "Classic", "Hard"}, 0));
    add(std::make_unique<juce::AudioParameterChoice>("oversampling", "Oversampling", juce::StringArray{"1x", "2x", "4x"}, 1));
    add(std::make_unique<juce::AudioParameterBool>("active", "Active", true));
    return p;
}

void AndresBeatsClipAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    oversampling.reset();
    oversampling.initProcessing ((size_t) samplesPerBlock);
    airStateL = airStateR = 0.0f;
}

void AndresBeatsClipAudioProcessor::releaseResources() {}

bool AndresBeatsClipAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

float AndresBeatsClipAudioProcessor::shapeSample (float x, float ceiling, float knee, int mode, float mojo, float alchemy) const noexcept
{
    const float sign = x < 0.0f ? -1.0f : 1.0f;
    const float a = std::abs(x);
    const float c = juce::jmax (0.001f, ceiling);
    const float k = juce::jlimit (0.0f, 0.99f, knee);
    float y = a;

    if (mode == 2) // Hard
        y = juce::jmin (a, c);
    else
    {
        const float start = c * (1.0f - k);
        if (a <= start || k < 0.001f)
            y = (a <= c ? a : (mode == 0 ? c : c * std::tanh(a / c)));
        else
        {
            const float t = juce::jlimit (0.0f, 1.0f, (a - start) / juce::jmax (0.0001f, c - start));
            const float eased = t * t * (3.0f - 2.0f * t);
            const float clipped = mode == 0 ? c : c * std::tanh(a / c);
            y = start + (clipped - start) * eased;
            y = juce::jmin (y, c);
        }
    }

    // Subtle transformer/tube-inspired harmonic colour; level compensated.
    const float drive = mojo * 0.18f;
    const float coloured = y + drive * (0.65f * y * y * sign - 0.22f * y * y * y);
    const float smooth = juce::jlimit (0.0f, 1.0f, alchemy);
    const float softened = c * std::tanh (coloured / c);
    return sign * (coloured * (1.0f - smooth) + softened * smooth);
}

void AndresBeatsClipAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels(), samples = buffer.getNumSamples();
    if (channels == 0 || samples == 0) return;

    const auto get = [this](const char* id) { return parameters.getRawParameterValue(id)->load(); };
    const bool active = get("active") > 0.5f;
    const float inGain = juce::Decibels::decibelsToGain(get("input"));
    const float ceilingDb = get("ceiling") + get("trim");
    const float ceiling = juce::Decibels::decibelsToGain(ceilingDb);
    const float knee = get("knee") / 100.0f;
    const float mojo = get("mojo") / 100.0f;
    const float alchemy = get("alchemy") / 100.0f;
    const float mix = get("mix") / 100.0f;
    const float air = get("air") / 100.0f * 0.22f;
    const int mode = (int) get("mode");
    float inPeak = 0.0f, outPeak = 0.0f, grPeak = 0.0f;

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        float airState = (ch == 0 ? airStateL : airStateR);
        for (int i = 0; i < samples; ++i)
        {
            const float dry = data[i];
            const float driven = dry * inGain;
            inPeak = juce::jmax (inPeak, std::abs (driven));
            float wet = driven;
            if (active)
            {
                wet = shapeSample (driven, ceiling, knee, mode, mojo, alchemy);
                // Simple high-shelf-like air lift from the high-frequency difference.
                const float low = 0.985f * airState + 0.015f * wet;
                const float high = wet - low;
                airState = low;
                wet += high * air;
            }
            const float result = active ? (driven * (1.0f - mix) + wet * mix) : driven;
            grPeak = juce::jmax (grPeak, juce::jmax (0.0f, std::abs(driven) - std::abs(wet)));
            data[i] = result;
            outPeak = juce::jmax (outPeak, std::abs(result));
        }
        if (ch == 0) airStateL = airState; else if (ch == 1) airStateR = airState;
    }
    inputPeak.store (inPeak);
    outputPeak.store (outPeak);
    gainReduction.store (grPeak);
}

juce::AudioProcessorEditor* AndresBeatsClipAudioProcessor::createEditor()
{
    return new AndresBeatsClipAudioProcessorEditor (*this);
}

void AndresBeatsClipAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AndresBeatsClipAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AndresBeatsClipAudioProcessor();
}
