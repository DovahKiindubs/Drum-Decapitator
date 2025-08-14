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
        std::make_unique<juce::AudioParameterFloat>(
            "mix", "Mix",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
            1.0f
        ),
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
            waveViewer(1)
{
    waveViewer.setRepaintRate(30);
    waveViewer.setBufferSize(256);
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
    }
}

void DrumDecapitatorAudioProcessor::releaseResources()
{
	waveViewer.clear();
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
    const float T = *parameters.getRawParameterValue("transient");
    const float S = *parameters.getRawParameterValue("sustain");
    const float mix = *parameters.getRawParameterValue("mix");
    const bool monitor = *parameters.getRawParameterValue("delta");

    for (int ch = 0; ch < 2; ++ch) {
        mEnv[ch].setAttackMs(att1);
        mEnv[ch].setReleaseMs(rel1);
        mEnvSlow[ch].setAttackMs(att1 + deltaAtk);
        mEnvSlow[ch].setReleaseMs(rel1);
    }

    juce::AudioBuffer<float> processedBuffer(buffer.getNumChannels(), buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto* processedData = processedBuffer.getWritePointer(channel);
        bool transientState = true;

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
			const float in = channelData[sample];
            const float envDb = 20.0f * std::log10(std::max(std::abs(in), 1e-5f));
            const float clampedEnv = std::max(0.0f, envDb + 100.0f);

			const float deltaFast = mEnv[channel].process(clampedEnv);
			const float deltaSlow = mEnvSlow[channel].process(clampedEnv);
			const float deltaDiff = deltaFast - deltaSlow + offset;

			transientState = (deltaDiff > 0.0f);

            const float deltaTrans = std::max(0.0f, deltaDiff);
			const float deltaSustain = std::max(0.0f, -deltaDiff);

            float adjustedTrans = transientState ?
                (T < 0 ? deltaTrans * T : deltaTrans) :
                (S < 0 ? deltaSustain * S : deltaSustain);

            // Calculate gain curve
            float curve = std::pow(10.0f, adjustedTrans / 20.0f);
            float out;

            if (curve >= 1.0f) {
                const float modifier = transientState ? T : S;
                out = 1.0f + modifier * std::tanh(curve - 1.0f);
            }
            else {
                out = curve;
            }

            if (monitor) {
				channelData[sample] = out / 3.0f; // Monitor output
			}
            else {
                channelData[sample] = out * in;
            }

            processedData[sample] = channelData[sample] * (2.0f / 3.0f);

        }
    }

    waveViewer.pushBuffer(processedBuffer);
    const float curve = *parameters.getRawParameterValue("clipperCurve");
    const float thresholdDB = *parameters.getRawParameterValue("clipperThresholdDB");

    for (int ch = 0; ch < std::min(2, buffer.getNumChannels()); ++ch) {
        clippers[ch].setParameters(curve, thresholdDB);
        auto* channelData = buffer.getWritePointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            channelData[i] = clippers[ch].processSample(channelData[i]);
        }
    }
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
