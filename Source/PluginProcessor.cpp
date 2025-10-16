/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DrumDecapitatorAudioProcessor::DrumDecapitatorAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)), parameters(*this, nullptr, "Parameters", {
        std::make_unique<juce::AudioParameterFloat>(
            "att1", "Attack1",
            juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
            0.0f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "deltaAtk", "DeltaAtk",
            juce::NormalisableRange<float>(1.0f, 250.0f, 0.1f, 0.3f),
            40.0f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "rel1", "Release1",
            juce::NormalisableRange<float>(10.0f, 2500.0f, 1.0f, 0.3f),
            40.0f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "offset", "Offset",
            juce::NormalisableRange<float>(-40.0f, 40.0f, 0.1f),
            -12.0f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "transient", "Transient",
            juce::NormalisableRange<float>(-1.0f, 2.0f, 0.01f),
            0.0f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "sustain", "Sustain",
            juce::NormalisableRange<float>(-1.0f, 2.0f, 0.01f),
            0.0f
        ),
        // Per-band transient shaper params
        std::make_unique<juce::AudioParameterFloat>(
            "transientLow", "Transient Low",
            juce::NormalisableRange<float>(-1.0f, 2.0f, 0.01f), 0.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "sustainLow", "Sustain Low",
            juce::NormalisableRange<float>(-1.0f, 2.0f, 0.01f), 0.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "transientHigh", "Transient High",
            juce::NormalisableRange<float>(-1.0f, 2.0f, 0.01f), 0.0f),
        std::make_unique<juce::AudioParameterFloat>(
            "sustainHigh", "Sustain High",
            juce::NormalisableRange<float>(-1.0f, 2.0f, 0.01f), 0.0f),
        // Mute / Solo
        std::make_unique<juce::AudioParameterBool>("lowMute",  "Low Mute",  false),
        std::make_unique<juce::AudioParameterBool>("lowSolo",  "Low Solo",  false),
        std::make_unique<juce::AudioParameterBool>("highMute", "High Mute", false),
        std::make_unique<juce::AudioParameterBool>("highSolo", "High Solo", false),
        std::make_unique<juce::AudioParameterFloat>(
            "mix", "Mix",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            1.0f
        ),
        std::make_unique<juce::AudioParameterFloat>(
            "CrossoverFreq", "Crossover Frequency",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f),
            800.0f),
        std::make_unique<juce::AudioParameterBool>("delta", "Delta", false),
        std::make_unique<juce::AudioParameterFloat>(
            "clipperCurve", "Clipper Curve",
            juce::NormalisableRange<float>(0.000001f, 0.5f, 0.000001f, 0.3f),
            0.000001f,
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) {
                return juce::String(value, 6);
            }
        ),
         std::make_unique<juce::AudioParameterFloat>(
            "clipperThresholdDB", "Clipper Threshold",
            juce::NormalisableRange<float>(-20.0f, 0.0f, 0.1f), 
            0.0f, 
            juce::String(),
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) {
                return juce::String(value, 1) + " dB";
            },
            [](const juce::String& text) {
                return text.getFloatValue(); 
            })

		}),
            waveViewer(1), InputViewer(1), CurveViewer(1)
{
    waveViewer.setRepaintRate(30);
    waveViewer.setBufferSize(512);
    InputViewer.setRepaintRate(30);
    InputViewer.setBufferSize(512);
    CurveViewer.setRepaintRate(30);
    CurveViewer.setBufferSize(512);
}

DrumDecapitatorAudioProcessor::~DrumDecapitatorAudioProcessor()
{
}

//==============================================================================
const juce::String DrumDecapitatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DrumDecapitatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DrumDecapitatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DrumDecapitatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DrumDecapitatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DrumDecapitatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DrumDecapitatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DrumDecapitatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DrumDecapitatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void DrumDecapitatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DrumDecapitatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	waveViewer.clear();
    for (int i = 0; i < 2; ++i) {
        mEnv[i].init(static_cast<float>(sampleRate));
        mEnvSlow[i].init(static_cast<float>(sampleRate));
        mEnvLow[i].init(static_cast<float>(sampleRate));
        mEnvLowSlow[i].init(static_cast<float>(sampleRate));
        mEnvHigh[i].init(static_cast<float>(sampleRate));
        mEnvHighSlow[i].init(static_cast<float>(sampleRate));
    }

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 2;

    LP.reset();
    HP.reset();
    LP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    LP.prepare(spec);
    HP.prepare(spec);
}

void DrumDecapitatorAudioProcessor::releaseResources()
{
	waveViewer.clear();
	InputViewer.clear();
	CurveViewer.clear();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DrumDecapitatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DrumDecapitatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const float att1 = *parameters.getRawParameterValue("att1");
    const float deltaAtk = *parameters.getRawParameterValue("deltaAtk");
    const float rel1 = *parameters.getRawParameterValue("rel1");
    const float offset = *parameters.getRawParameterValue("offset");
    const float TLow  = *parameters.getRawParameterValue("transientLow");
    const float SLow  = *parameters.getRawParameterValue("sustainLow");
    const float THigh = *parameters.getRawParameterValue("transientHigh");
    const float SHigh = *parameters.getRawParameterValue("sustainHigh");
    const float mix = *parameters.getRawParameterValue("mix");
    const bool lowMute  = *parameters.getRawParameterValue("lowMute");
    const bool lowSolo  = *parameters.getRawParameterValue("lowSolo");
    const bool highMute = *parameters.getRawParameterValue("highMute");
    const bool highSolo = *parameters.getRawParameterValue("highSolo");

    const float clippingcurve = *parameters.getRawParameterValue("clipperCurve");
    const float thresholdDB = *parameters.getRawParameterValue("clipperThresholdDB");
    const float crossover = *parameters.getRawParameterValue("CrossoverFreq");

    //const bool monitor = *parameters.getRawParameterValue("delta");

    for (int ch = 0; ch < 2; ++ch) {
        // full-band envelopes (kept initialised, not used below)
        mEnv[ch].setAttackMs(att1);
        mEnv[ch].setReleaseMs(rel1);
        mEnvSlow[ch].setAttackMs(att1 + deltaAtk);
        mEnvSlow[ch].setReleaseMs(rel1);

        // low-band envelopes
        mEnvLow[ch].setAttackMs(att1);
        mEnvLow[ch].setReleaseMs(rel1);
        mEnvLowSlow[ch].setAttackMs(att1 + deltaAtk);
        mEnvLowSlow[ch].setReleaseMs(rel1);

        // high-band envelopes
        mEnvHigh[ch].setAttackMs(att1);
        mEnvHigh[ch].setReleaseMs(rel1);
        mEnvHighSlow[ch].setAttackMs(att1 + deltaAtk);
        mEnvHighSlow[ch].setReleaseMs(rel1);

        clippers[ch].setParameters(clippingcurve, thresholdDB);
    }

    // update crossover
    LP.setCutoffFrequency(crossover);
    HP.setCutoffFrequency(crossover);


	juce::AudioBuffer<float> inputBuffer(buffer.getNumChannels(), buffer.getNumSamples());
	juce::AudioBuffer<float> curveBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    juce::AudioBuffer<float> lowBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    juce::AudioBuffer<float> highBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        inputBuffer.copyFrom(channel, 0, buffer, channel, 0, buffer.getNumSamples());
        lowBuffer.copyFrom(channel, 0, buffer, channel, 0, buffer.getNumSamples());
        highBuffer.copyFrom(channel, 0, buffer, channel, 0, buffer.getNumSamples());
        // scale input display buffer to 2/3 for visualization only
        auto* inPtr = inputBuffer.getWritePointer(channel);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            inPtr[i] *= (2.0f / 3.0f);
    }

    // Split bands
    {
        juce::dsp::AudioBlock<float> lowBlock (lowBuffer);
        juce::dsp::AudioBlock<float> highBlock (highBuffer);
        juce::dsp::ProcessContextReplacing<float> ctxLow (lowBlock);
        juce::dsp::ProcessContextReplacing<float> ctxHigh (highBlock);
        LP.process (ctxLow);
        HP.process (ctxHigh);
    }

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
		auto* curveData = curveBuffer.getWritePointer(channel);
        auto* inputData = inputBuffer.getWritePointer(channel);
        auto* lowData = lowBuffer.getWritePointer(channel);
        auto* highData = highBuffer.getWritePointer(channel);


        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            const float in = inputData[sample];

            // Low band magnitude in dB domain for envelope driving
            const float lowEnvDb  = 20.0f * std::log10(std::max(std::abs(lowData[sample]),  1e-5f));
            const float highEnvDb = 20.0f * std::log10(std::max(std::abs(highData[sample]), 1e-5f));
            const float lowClamp  = std::max(0.0f, lowEnvDb + 100.0f);
            const float highClamp = std::max(0.0f, highEnvDb + 100.0f);

            // Low band delta
            const float lowFast = mEnvLow[channel].process(lowClamp);
            const float lowSlow = mEnvLowSlow[channel].process(lowClamp);
            const float lowDiff = lowFast - lowSlow + offset;
            const bool lowTransient = (lowDiff > 0.0f);
            const float lowTrans = std::max(0.0f, lowDiff);
            const float lowSus   = std::max(0.0f, -lowDiff);
            const float lowAdj = lowTransient ? (TLow < 0 ? lowTrans * TLow : lowTrans)
                                              : (SLow < 0 ? lowSus   * SLow : lowSus);
            const float lowCurve = std::pow(10.0f, lowAdj / 20.0f);
            const float lowOut = (lowCurve >= 1.0f)
                               ? (1.0f + (lowTransient ? TLow : SLow) * std::tanh(lowCurve - 1.0f))
                               : lowCurve;

            // High band delta
            const float highFast = mEnvHigh[channel].process(highClamp);
            const float highSlow = mEnvHighSlow[channel].process(highClamp);
            const float highDiff = highFast - highSlow + offset;
            const bool highTransient = (highDiff > 0.0f);
            const float highTrans = std::max(0.0f, highDiff);
            const float highSus   = std::max(0.0f, -highDiff);
            const float highAdj = highTransient ? (THigh < 0 ? highTrans * THigh : highTrans)
                                                : (SHigh < 0 ? highSus   * SHigh : highSus);
            const float highCurve = std::pow(10.0f, highAdj / 20.0f);
            const float highOut = (highCurve >= 1.0f)
                                ? (1.0f + (highTransient ? THigh : SHigh) * std::tanh(highCurve - 1.0f))
                                : highCurve;

            // Apply gains to bands
            const float lowShaped  = lowOut  * lowData[sample];
            const float highShaped = highOut * highData[sample];
            const bool anySolo = (lowSolo || highSolo);
            const bool passLow  = anySolo ? lowSolo  : !lowMute;
            const bool passHigh = anySolo ? highSolo : !highMute;
            const float shaped = (passLow ? lowShaped : 0.0f) + (passHigh ? highShaped : 0.0f);

            // Effective gain for visualisation (pre-clip)
            const float effGain = std::abs(in) > 1e-6f ? std::abs(shaped) / std::max(std::abs(in), 1e-6f) : 1.0f;
            curveData[sample] = std::log(std::max(effGain, 1e-5f));

            // Optional dry/wet mix
            const float mixed = mix * shaped + (1.0f - mix) * in;

            // Clip and write output
            channelData[sample] = clippers[channel].processSample(mixed);
            
        }
    }

	InputViewer.pushBuffer(inputBuffer);
    //CurveViewer.pushBuffer(curveBuffer);
    //waveViewer will show final, clipped output buffer

    /*
    for (int ch = 0; ch < std::min(2, buffer.getNumChannels()); ++ch) {
        clippers[ch].setParameters(clippingcurve, thresholdDB);
        auto* channelData = buffer.getWritePointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            channelData[i] = clippers[ch].processSample(channelData[i]);
        }
    }
    */

    // push scaled (2/3) final output to WaveViewer for visualization
    juce::AudioBuffer<float> outDisplay(buffer.getNumChannels(), buffer.getNumSamples());
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        outDisplay.copyFrom(ch, 0, buffer, ch, 0, buffer.getNumSamples());
        auto* p = outDisplay.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            p[i] *= (2.0f / 3.0f);
    }
    waveViewer.pushBuffer(outDisplay);
}

//==============================================================================
bool DrumDecapitatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DrumDecapitatorAudioProcessor::createEditor()
{
    return new DrumDecapitatorAudioProcessorEditor (*this);
}

//==============================================================================
void DrumDecapitatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DrumDecapitatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrumDecapitatorAudioProcessor();
}
