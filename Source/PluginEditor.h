/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
struct SliderParamInfo {
    juce::Slider* slider;
    const char* paramID;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class DrumDecapitatorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DrumDecapitatorAudioProcessorEditor(DrumDecapitatorAudioProcessor&);
    ~DrumDecapitatorAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    juce::Slider attackSlider, deltaAttackSlider, releaseSlider;
    juce::Slider offsetSlider, transientSlider, sustainSlider;
    juce::Slider mixSlider;
    juce::Slider bufferSizeSlider;
    juce::Slider clipperCurveSlider, clipperThresholdSlider;
    juce::ToggleButton deltaButton;

    std::vector<SliderParamInfo> sliders;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> attackAttachment, deltaAttackAttachment, releaseAttachment;
    std::unique_ptr<SliderAttachment> offsetAttachment, transientAttachment, sustainAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<ButtonAttachment> deltaAttachment;
    std::unique_ptr<SliderAttachment> clipperCurveAttachment, clipperThresholdAttachment;


    DrumDecapitatorAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumDecapitatorAudioProcessorEditor)
};
