/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DrumDecapitatorAudioProcessorEditor::DrumDecapitatorAudioProcessorEditor (DrumDecapitatorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    sliders.emplace_back(SliderParamInfo{ &attackSlider,      "att1" });
    sliders.emplace_back(SliderParamInfo{ &deltaAttackSlider, "deltaAtk" });
    sliders.emplace_back(SliderParamInfo{ &releaseSlider,     "rel1" });
    sliders.emplace_back(SliderParamInfo{ &offsetSlider,      "offset" });
    sliders.emplace_back(SliderParamInfo{ &transientSlider,   "transient" });
    sliders.emplace_back(SliderParamInfo{ &sustainSlider,     "sustain" });
    sliders.emplace_back(SliderParamInfo{ &mixSlider,         "mix" });

    for (auto& info : sliders)
    {
        addAndMakeVisible(*info.slider);
        info.slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
        info.slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        info.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, info.paramID, *info.slider);
    }

    // Set component names for accessibility
    attackSlider.setName("Attack");
    deltaAttackSlider.setName("Delta Attack");
    releaseSlider.setName("Release");
    offsetSlider.setName("Offset");
    transientSlider.setName("Transient");
    sustainSlider.setName("Sustain");
    mixSlider.setName("Mix");


	addAndMakeVisible(audioProcessor.waveViewer);
    audioProcessor.waveViewer.setColours(juce::Colours::black, juce::Colours::white);
    bufferSizeSlider.setRange(256, 4096, 1);
    bufferSizeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bufferSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    addAndMakeVisible(bufferSizeSlider);

    bufferSizeSlider.onValueChange = [this]() {
        audioProcessor.waveViewer.setBufferSize((int)bufferSizeSlider.getValue());
        };

    addAndMakeVisible(clipperCurveSlider);
    clipperCurveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    clipperCurveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    clipperCurveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "clipperCurve", clipperCurveSlider
    );

    addAndMakeVisible(clipperThresholdSlider);
    clipperThresholdSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    clipperThresholdSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    clipperThresholdSlider.setTextValueSuffix(" dB");
    clipperThresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "clipperThresholdDB", clipperThresholdSlider
    );

    clipperCurveSlider.setName("Clipper Curve");
    clipperThresholdSlider.setName("Clipper Threshold");

    setSize (700, 700);
}

DrumDecapitatorAudioProcessorEditor::~DrumDecapitatorAudioProcessorEditor()
{
}

//==============================================================================
void DrumDecapitatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black.brighter(0.1));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Drum Decapitator", getLocalBounds().removeFromTop(30), juce::Justification::centred, 1);

    g.drawText("Attack", attackSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
    g.drawText("Delta Atk", deltaAttackSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
    g.drawText("Release", releaseSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
    g.drawText("Offset", offsetSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
    g.drawText("Transient", transientSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
    g.drawText("Sustain", sustainSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
    g.drawText("Mix", mixSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
    g.drawText("Hard/Soft", clipperCurveSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
    g.drawText("Ceiling", clipperThresholdSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
}

void DrumDecapitatorAudioProcessorEditor::resized()
{
    const int sliderSize = 80;
    const int sliderSpacing = 10;
    const int topMargin = 50;
    const int waveViewerHeight = 200;
    const int row2Y = topMargin + sliderSize + 40;

    attackSlider.setBounds(10, topMargin, sliderSize, sliderSize);
    deltaAttackSlider.setBounds(100, topMargin, sliderSize, sliderSize);
    releaseSlider.setBounds(190, topMargin, sliderSize, sliderSize);
    offsetSlider.setBounds(280, topMargin, sliderSize, sliderSize);
    transientSlider.setBounds(370, topMargin, sliderSize, sliderSize);
    sustainSlider.setBounds(460, topMargin, sliderSize, sliderSize);
    mixSlider.setBounds(550, topMargin, sliderSize, sliderSize);
    clipperCurveSlider.setBounds(10, row2Y, sliderSize, sliderSize);
    clipperThresholdSlider.setBounds(100, row2Y, sliderSize, sliderSize);
    int waveViewerTop = topMargin + sliderSize + sliderSpacing;
    audioProcessor.waveViewer.setBounds(10, waveViewerTop + 200, getWidth() - 40, waveViewerHeight);
    bufferSizeSlider.setBounds(10, 10, getWidth() - 20, 40);

}
