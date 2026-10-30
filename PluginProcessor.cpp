#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float kPi = juce::MathConstants<float>::pi;

    // u: señal normalizada al techo (1.0 = ceiling). t: inicio de la rodilla. w = 1 - t.
    // mode 0 = Modern (tanh), 1 = Classic (atan, más redondeado), 2 = Hard (recorte duro)
    inline float clipShape (float u, float t, float w, int mode) noexcept
    {
        if (mode == 2)
            return juce::jlimit (-1.0f, 1.0f, u);

        const float a = std::abs (u);
        if (a <= t)
            return u;

        const float s = (a - t) / w;
        const float r = (mode == 0) ? std::tanh (s)
                                    : (2.0f / kPi) * std::atan (kPi * 0.5f * s);
        const float y = t + w * r;
        return u < 0.0f ? -y : y;
    }

    inline float peakOf (const juce::dsp::AudioBlock<float>& b, size_t ch)
    {
        if (ch >= b.getNumChannels() || b.getNumSamples() == 0)
            return 0.0f;

        auto r = juce::FloatVectorOperations::findMinAndMax (b.getChannelPointer (ch), (int) b.getNumSamples());
        return juce::jmax (std::abs (r.getStart()), std::abs (r.getEnd()));
    }

    inline std::function<juce::String (float, int)> textFn (const juce::String& suffix)
    {
        return [suffix] (float v, int) { return juce::String (v, 1) + suffix; };
    }

    inline std::function<float (const juce::String&)> parseFn()
    {
        return [] (const juce::String& s) { return s.getFloatValue(); };
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout AndresBeatsClipAudioProcessor::createLayout()
{
    using namespace juce;
    using Attr = AudioParameterFloatAttributes;

    AudioProcessorValueTreeState::ParameterLayout layout;

    auto addFloat = [&layout] (const char* id, const char* name, float lo, float hi, float def, const String& suffix)
    {
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { id, 1 }, name, NormalisableRange<float> (lo, hi, 0.1f), def,
            Attr().withStringFromValueFunction (textFn (suffix)).withValueFromStringFunction (parseFn())));
    };

    addFloat (IDs::inputGain, "Input Gain", -24.0f, 24.0f, 0.0f,  " dB");
    addFloat (IDs::ceiling,   "Ceiling",    -12.0f,  0.0f, 0.0f,  " dB");
    addFloat (IDs::knee,      "Knee Soft",    0.0f, 100.0f, 15.0f, " %");
    addFloat (IDs::mojo,      "Gold Mojo",    0.0f, 100.0f, 25.0f, " %");
    addFloat (IDs::alchemy,   "Alchemy",      0.0f, 100.0f, 10.0f, " %");
    addFloat (IDs::air,       "High Air Shimmer", 0.0f, 100.0f, 15.0f, " %");
    addFloat (IDs::ceilTrim,  "Ceil Trim",  -12.0f,  6.0f, 0.0f,  " dB");
    addFloat (IDs::dryWet,    "Dry / Wet",    0.0f, 100.0f, 100.0f, " %");

    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { IDs::mode, 1 }, "Mode",
                                                        StringArray { "Modern", "Classic", "Hard" }, 0));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { IDs::os, 1 }, "Oversampling",
                                                        StringArray { "1x", "2x", "4x" }, 1));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { IDs::active, 1 }, "Active", true));

    return layout;
}

AndresBeatsClipAudioProcessor::AndresBeatsClipAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    pInputGain = apvts.getRawParameterValue (IDs::inputGain);
    pCeiling   = apvts.getRawParameterValue (IDs::ceiling);
    pKnee      = apvts.getRawParameterValue (IDs::knee);
    pMojo      = apvts.getRawParameterValue (IDs::mojo);
    pAlchemy   = apvts.getRawParameterValue (IDs::alchemy);
    pAir       = apvts.getRawParameterValue (IDs::air);
    pCeilTrim  = apvts.getRawParameterValue (IDs::ceilTrim);
    pDryWet    = apvts.getRawParameterValue (IDs::dryWet);
    pMode      = apvts.getRawParameterValue (IDs::mode);
    pOs        = apvts.getRawParameterValue (IDs::os);
    pActive    = apvts.getRawParameterValue (IDs::active);
}

bool AndresBeatsClipAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

juce::dsp::Oversampling<float>* AndresBeatsClipAudioProcessor::getOversampler() noexcept
{
    if (osIndex == 1) return &os2x;
    if (osIndex == 2) return &os4x;
    return nullptr;
}

void AndresBeatsClipAudioProcessor::setupSmoothing()
{
    const double osRate = sampleRate * (double) osFactor;

    ceilSm.reset (osRate, 0.03);
    kneeSm.reset (osRate, 0.03);
    mojoSm.reset (osRate, 0.03);
    alchSm.reset (osRate, 0.03);

    ceilSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pCeiling->load()));
    kneeSm.setCurrentAndTargetValue (pKnee->load() * 0.01f);
    mojoSm.setCurrentAndTargetValue (pMojo->load() * 0.01f);
    alchSm.setCurrentAndTargetValue (pAlchemy->load() * 0.01f);

    dcR = 1.0f - (juce::MathConstants<float>::twoPi * 20.0f / (float) osRate);
}

void AndresBeatsClipAudioProcessor::applyOversampling (int newIndex)
{
    osIndex  = juce::jlimit (0, 2, newIndex);
    osFactor = (osIndex == 0) ? 1 : (osIndex == 1 ? 2 : 4);

    float latency = 0.0f;
    if (auto* o = getOversampler())
    {
        o->reset();
        latency = (float) o->getLatencyInSamples();
    }

    const int latencyInt = juce::roundToInt (latency);
    setLatencySamples (latencyInt);
    mixer.setWetLatency ((float) latencyInt);

    for (int i = 0; i < 2; ++i) { dcX[i] = 0.0f; dcY[i] = 0.0f; }
    setupSmoothing();
}

void AndresBeatsClipAudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;
    maxBlock = juce::jmax (1, samplesPerBlock);

    const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlock, 2 };

    os2x.initProcessing ((size_t) maxBlock);
    os4x.initProcessing ((size_t) maxBlock);

    mixer.prepare (spec);
    mixer.setMixingRule (juce::dsp::DryWetMixingRule::linear);
    mixer.setWetMixProportion (pActive->load() > 0.5f ? pDryWet->load() * 0.01f : 0.0f);

    airFilter.prepare (spec);
    airFilter.reset();
    lastAir = -1.0f;

    inSm.reset (sampleRate, 0.03);
    outSm.reset (sampleRate, 0.03);
    inSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pInputGain->load()));
    outSm.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pCeilTrim->load()));

    osIndex = -1;
    applyOversampling ((int) pOs->load());
}

void AndresBeatsClipAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    const int wantedOs = (int) pOs->load();
    if (wantedOs != osIndex)
        applyOversampling (wantedOs);

    juce::dsp::AudioBlock<float> all (buffer);
    const size_t total = all.getNumSamples();
    size_t pos = 0;

    while (pos < total)
    {
        const size_t n = juce::jmin ((size_t) maxBlock, total - pos);
        processChunk (all.getSubBlock (pos, n));
        pos += n;
    }
}

void AndresBeatsClipAudioProcessor::processChunk (juce::dsp::AudioBlock<float> block)
{
    const size_t numCh = juce::jmin ((size_t) 2, block.getNumChannels());
    const size_t numS  = block.getNumSamples();

    const bool active = pActive->load() > 0.5f;
    const int mode    = (int) pMode->load();

    // Dry/Wet. Con ACTIVE apagado la mezcla va a 0 => solo señal seca (retrasada para compensar la latencia).
    mixer.setWetMixProportion (active ? pDryWet->load() * 0.01f : 0.0f);
    mixer.pushDrySamples (block);

    // 1) Input gain
    inSm.setTargetValue (juce::Decibels::decibelsToGain (pInputGain->load()));
    for (size_t i = 0; i < numS; ++i)
    {
        const float g = inSm.getNextValue();
        for (size_t ch = 0; ch < numCh; ++ch)
            block.getChannelPointer (ch)[i] *= g;
    }

    inPeakL.store (juce::jmax (inPeakL.load(), peakOf (block, 0)));
    inPeakR.store (juce::jmax (inPeakR.load(), peakOf (block, 1)));

    // 2) High Air Shimmer: shelf alto a 10 kHz (hasta +6 dB) ANTES del clipper,
    //    así el techo sigue siendo respetado.
    const float air = pAir->load();
    if (air != lastAir)
    {
        lastAir = air;
        *airFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, 10000.0f, 0.5f, juce::Decibels::decibelsToGain (air * 0.06f));
    }
    if (air > 0.01f)
        airFilter.process (juce::dsp::ProcessContextReplacing<float> (block));

    // 3) Núcleo no lineal (sobremuestreado)
    auto* os = getOversampler();
    juce::dsp::AudioBlock<float> work = (os != nullptr) ? os->processSamplesUp (block) : block;

    ceilSm.setTargetValue (juce::Decibels::decibelsToGain (pCeiling->load()));
    kneeSm.setTargetValue (pKnee->load() * 0.01f);
    mojoSm.setTargetValue (pMojo->load() * 0.01f);
    alchSm.setTargetValue (pAlchemy->load() * 0.01f);

    float preMax = 0.0f, postMax = 0.0f;
    const size_t workN = work.getNumSamples();

    for (size_t i = 0; i < workN; ++i)
    {
        const float c  = ceilSm.getNextValue();
        const float kn = kneeSm.getNextValue();
        const float mj = mojoSm.getNextValue();
        const float al = alchSm.getNextValue();

        const float t = 1.0f - 0.75f * kn;
        const float w = juce::jmax (1.0e-3f, 1.0f - t);
        const float invC = 1.0f / c;

        for (size_t ch = 0; ch < numCh; ++ch)
        {
            float* d = work.getChannelPointer (ch);

            const float u0 = d[i] * invC;
            const float a0 = std::abs (u0);
            preMax = juce::jmax (preMax, a0);

            // Gold Mojo: armónicos pares (|x| sin DC). Alchemy: armónicos impares (x^3).
            const float eHp = a0 - dcX[ch] + dcR * dcY[ch];
            dcX[ch] = a0;
            dcY[ch] = eHp;

            const float u = u0 + mj * 0.30f * eHp + al * 0.20f * u0 * u0 * u0;
            const float y = clipShape (u, t, w, mode);

            postMax = juce::jmax (postMax, std::abs (y));
            d[i] = y * c;
        }
    }

    if (os != nullptr)
        os->processSamplesDown (block);

    // 4) Medidor de reducción de ganancia
    float gr = 0.0f;
    if (preMax > 1.0e-4f)
        gr = juce::jmin (0.0f, 20.0f * std::log10 (juce::jmax (postMax, 1.0e-6f) / preMax));
    grDb.store (juce::jmin (grDb.load(), gr));

    // 5) Ceil Trim (ganancia de salida)
    outSm.setTargetValue (juce::Decibels::decibelsToGain (pCeilTrim->load()));
    for (size_t i = 0; i < numS; ++i)
    {
        const float g = outSm.getNextValue();
        for (size_t ch = 0; ch < numCh; ++ch)
            block.getChannelPointer (ch)[i] *= g;
    }

    // 6) Mezcla dry/wet
    mixer.mixWetSamples (block);

    outPeakL.store (juce::jmax (outPeakL.load(), peakOf (block, 0)));
    outPeakR.store (juce::jmax (outPeakR.load(), peakOf (block, 1)));
}

juce::AudioProcessorEditor* AndresBeatsClipAudioProcessor::createEditor()
{
    return new AndresBeatsClipAudioProcessorEditor (*this);
}

void AndresBeatsClipAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void AndresBeatsClipAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AndresBeatsClipAudioProcessor();
}
