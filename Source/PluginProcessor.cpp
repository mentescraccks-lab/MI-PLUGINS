#include "PluginProcessor.h"

AndresBeatsClipAudioProcessor::AndresBeatsClipAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // 2x Oversampling para eliminar aliasing de clipping
    oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
        2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
}

AndresBeatsClipAudioProcessor::~AndresBeatsClipAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout AndresBeatsClipAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "input", 1 }, "Input", juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "ceiling", 1 }, "Ceiling", juce::NormalisableRange<float> (-24.0f, 0.0f, 0.1f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "knee", 1 }, "Knee", juce::NormalisableRange<float> (0.0f, 100.0f, 0.5f), 15.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "gold", 1 }, "Gold Mojo", juce::NormalisableRange<float> (0.0f, 100.0f, 0.5f), 25.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "alchemy", 1 }, "Alchemy", juce::NormalisableRange<float> (0.0f, 100.0f, 0.5f), 10.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "highEnhance", 1 }, "High Air", juce::NormalisableRange<float> (0.0f, 100.0f, 0.5f), 15.0f));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "mode", 1 }, "Mode", juce::StringArray { "Modern", "Classic", "Hard" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "output", 1 }, "Output", juce::NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mix", 1 }, "Mix", juce::NormalisableRange<float> (0.0f, 100.0f, 0.5f), 100.0f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "bypass", 1 }, "Bypass", false));

    return { params.begin(), params.end() };
}

void AndresBeatsClipAudioProcessor::prepareToPlay (double /*sampleRate*/, int samplesPerBlock)
{
    oversampler->initProcessing (static_cast<size_t> (samplesPerBlock));
    setLatencySamples (static_cast<int> (oversampler->getLatencyInSamples()));

    highShelfFilterL.reset();
    highShelfFilterR.reset();

    // Coeficientes iniciales (neutros) para que el filtro nunca quede sin inicializar
    auto neutral = juce::dsp::IIR::Coefficients<float>::makeHighShelf (getSampleRate() * 2.0, 8500.0f, 0.707f, 1.0f);
    highShelfFilterL.coefficients = neutral;
    highShelfFilterR.coefficients = neutral;

    dcX1L = dcY1L = dcX1R = dcY1R = 0.0f;
}

void AndresBeatsClipAudioProcessor::releaseResources()
{
    oversampler->reset();
}

bool AndresBeatsClipAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Solo estéreo (el DSP procesa L/R)
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

float AndresBeatsClipAudioProcessor::processClipperSample (float x, float ceilVal, float kneeNorm, int mode)
{
    const float sign = (x < 0.0f) ? -1.0f : 1.0f;
    const float absX = std::abs (x);

    // Hard Clip Mode
    if (mode == 2)
        return (absX > ceilVal) ? sign * ceilVal : x;

    const float kneeSpan = ceilVal * (0.01f + kneeNorm * 0.45f);
    const float lowerThresh = ceilVal - kneeSpan;

    if (absX <= lowerThresh)
        return x;

    // Modern Mode: polinomio suave (smoothstep)
    if (mode == 0)
    {
        if (absX >= ceilVal)
            return sign * ceilVal;

        const float t = (absX - lowerThresh) / (ceilVal - lowerThresh + 1e-6f);
        const float smoothFactor = t * t * (3.0f - 2.0f * t);
        return sign * (lowerThresh + smoothFactor * (ceilVal - lowerThresh));
    }
    // Classic Mode: curva analógica asintótica basada en tanh
    else
    {
        const float headroom = ceilVal - lowerThresh;
        const float normDelta = (absX - lowerThresh) / (headroom + 1e-6f);
        const float softDelta = std::tanh (normDelta) * headroom;
        return sign * std::min (ceilVal, lowerThresh + softDelta);
    }
}

void AndresBeatsClipAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (*apvts.getRawParameterValue ("bypass") > 0.5f)
        return;

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const float inGain       = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("input")->load());
    const float ceilGain     = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("ceiling")->load());
    const float kneeNorm     = apvts.getRawParameterValue ("knee")->load() * 0.01f;
    const float goldNorm     = apvts.getRawParameterValue ("gold")->load() * 0.01f;
    const float alchemyNorm  = apvts.getRawParameterValue ("alchemy")->load() * 0.01f;
    const float highAirAmt   = apvts.getRawParameterValue ("highEnhance")->load() * 0.01f;
    const int   mode         = static_cast<int> (apvts.getRawParameterValue ("mode")->load());
    const float outGain      = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("output")->load());
    const float mix          = apvts.getRawParameterValue ("mix")->load() * 0.01f;

    // Backup de señal seca para Mix
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf (buffer);

    // 1. Pre-Gain
    buffer.applyGain (inGain);

    // 2. Oversampling Up-sample
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::AudioBlock<float> osBlock = oversampler->processSamplesUp (block);

    const int osSamples = static_cast<int> (osBlock.getNumSamples());
    float* osLeft  = osBlock.getChannelPointer (0);
    float* osRight = osBlock.getChannelPointer (1);

    // Filtro High-Shelf (8.5 kHz)
    if (highAirAmt > 0.001f)
    {
        const float boostDb = highAirAmt * 4.5f;
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            getSampleRate() * 2.0, 8500.0f, 0.707f, juce::Decibels::decibelsToGain (boostDb));
        highShelfFilterL.coefficients = coeffs;
        highShelfFilterR.coefficients = coeffs;
    }

    float maxBlockRedDb = 0.0f;

    for (int i = 0; i < osSamples; ++i)
    {
        float sL = osLeft[i];
        float sR = osRight[i];

        // High Air Shimmer
        if (highAirAmt > 0.001f)
        {
            sL = highShelfFilterL.processSample (sL);
            sR = highShelfFilterR.processSample (sR);
        }

        // Gold Mojo (armónicos pares e impares)
        if (goldNorm > 0.001f)
        {
            const float gDrive = 1.0f + goldNorm * 1.5f;
            const float g2nd   = goldNorm * 0.18f;
            const float g3rd   = goldNorm * 0.10f;

            sL = sL * gDrive + (sL * sL) * g2nd - (sL * sL * sL) * g3rd;
            sR = sR * gDrive + (sR * sR) * g2nd - (sR * sR * sR) * g3rd;
        }

        const float prePeak = std::max (std::abs (sL), std::abs (sR));

        // Precision Clipping
        sL = processClipperSample (sL, ceilGain, kneeNorm, mode);
        sR = processClipperSample (sR, ceilGain, kneeNorm, mode);

        const float postPeak = std::max (std::abs (sL), std::abs (sR));
        if (prePeak > ceilGain && prePeak > 1e-4f)
        {
            const float redRatio = postPeak / prePeak;
            if (redRatio < 0.999f)
                maxBlockRedDb = std::max (maxBlockRedDb, (1.0f - redRatio) * 20.0f);
        }

        // Alchemy Diffusion (suavizado analógico)
        if (alchemyNorm > 0.001f)
        {
            const float softenL = std::tanh (sL / (ceilGain + 1e-6f)) * ceilGain;
            const float softenR = std::tanh (sR / (ceilGain + 1e-6f)) * ceilGain;
            sL = juce::jmap (alchemyNorm * 0.5f, sL, softenL);
            sR = juce::jmap (alchemyNorm * 0.5f, sR, softenR);
        }

        // DC Blocker (IIR de 1 polo)
        const float dcPole = 0.9985f;
        float fL = sL - dcX1L + dcPole * dcY1L;
        dcX1L = sL; dcY1L = fL; sL = fL;

        float fR = sR - dcX1R + dcPole * dcY1R;
        dcX1R = sR; dcY1R = fR; sR = fR;

        osLeft[i]  = sL;
        osRight[i] = sR;
    }

    // 3. Oversampling Down-sample
    oversampler->processSamplesDown (block);

    // 4. Output Makeup & Dry/Wet Mix
    const int numSamples = buffer.getNumSamples();
    float* outL = buffer.getWritePointer (0);
    float* outR = buffer.getWritePointer (1);
    const float* dryL = dryBuffer.getReadPointer (0);
    const float* dryR = dryBuffer.getReadPointer (1);

    for (int i = 0; i < numSamples; ++i)
    {
        const float wetSampleL = outL[i] * outGain;
        const float wetSampleR = outR[i] * outGain;

        outL[i] = juce::jlimit (-1.5f, 1.5f, juce::jmap (mix, dryL[i], wetSampleL));
        outR[i] = juce::jlimit (-1.5f, 1.5f, juce::jmap (mix, dryR[i], wetSampleR));
    }

    clipReductionDb.store (maxBlockRedDb);
}

void AndresBeatsClipAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AndresBeatsClipAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorEditor* AndresBeatsClipAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AndresBeatsClipAudioProcessor();
}
