/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Envelope/dsp1d_mod_Envelope.h"
#include "Envelope/dsp1d_mod_fastMath.h"
#include "SoftClipper.h"

//==============================================================================
/**
*/
class DrumDecapitatorAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DrumDecapitatorAudioProcessor();
    ~DrumDecapitatorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	juce::AudioProcessorValueTreeState parameters;
    juce::AudioVisualiserComponent waveViewer, InputViewer, CurveViewer;

private:
    //==============================================================================
    std::array<dsp1d::mod::Envelope, 2> mEnv;      // Fast envelope per channel
    std::array<dsp1d::mod::Envelope, 2> mEnvSlow;
    // Separate envelopes for band-split processing
    std::array<dsp1d::mod::Envelope, 2> mEnvLow;
    std::array<dsp1d::mod::Envelope, 2> mEnvLowSlow;
    std::array<dsp1d::mod::Envelope, 2> mEnvHigh;
    std::array<dsp1d::mod::Envelope, 2> mEnvHighSlow;
    std::array<SoftClipper, 2> clippers;

	using Filter = juce::dsp::LinkwitzRileyFilter<float>;
    Filter LP, HP;

	std::array<juce::AudioBuffer<float>, 2> filterBuffers;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumDecapitatorAudioProcessor)
};
