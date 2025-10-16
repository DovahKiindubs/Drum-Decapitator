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

    // Per-band sliders and buttons
    addAndMakeVisible(transientLowSlider);
    addAndMakeVisible(sustainLowSlider);
    addAndMakeVisible(transientHighSlider);
    addAndMakeVisible(sustainHighSlider);
    transientLowAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "transientLow", transientLowSlider);
    sustainLowAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "sustainLow", sustainLowSlider);
    transientHighAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "transientHigh", transientHighSlider);
    sustainHighAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "sustainHigh", sustainHighSlider);

    for (auto* s : { &transientLowSlider, &sustainLowSlider, &transientHighSlider, &sustainHighSlider })
    {
        s->setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    }

    addAndMakeVisible(lowMuteButton);
    addAndMakeVisible(lowSoloButton);
    addAndMakeVisible(highMuteButton);
    addAndMakeVisible(highSoloButton);
    lowMuteAttachment = std::make_unique<ButtonAttachment>(audioProcessor.parameters, "lowMute", lowMuteButton);
    lowSoloAttachment = std::make_unique<ButtonAttachment>(audioProcessor.parameters, "lowSolo", lowSoloButton);
    highMuteAttachment = std::make_unique<ButtonAttachment>(audioProcessor.parameters, "highMute", highMuteButton);
    highSoloAttachment = std::make_unique<ButtonAttachment>(audioProcessor.parameters, "highSolo", highSoloButton);

    transientLowSlider.setName("Trans Low");
    sustainLowSlider.setName("Sus Low");
    transientHighSlider.setName("Trans High");
    sustainHighSlider.setName("Sus High");
    lowMuteButton.setButtonText("Low Mute");
    lowSoloButton.setButtonText("Low Solo");
    highMuteButton.setButtonText("High Mute");
    highSoloButton.setButtonText("High Solo");

    addAndMakeVisible(audioProcessor.waveViewer);
    audioProcessor.waveViewer.setColours(juce::Colours::black, juce::Colours::orange);

    addAndMakeVisible(audioProcessor.InputViewer);
    audioProcessor.InputViewer.setColours(juce::Colours::transparentBlack, juce::Colours::white);
    audioProcessor.InputViewer.setOpaque(false);

    addAndMakeVisible(audioProcessor.CurveViewer);
    audioProcessor.CurveViewer.setColours(juce::Colours::transparentBlack, juce::Colours::blue);
	audioProcessor.CurveViewer.setOpaque(false);

    addAndMakeVisible(bufferSizeSlider);
    bufferSizeSlider.setRange(256, 4096, 1);
    bufferSizeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bufferSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);


    bufferSizeSlider.onValueChange = [this]() {
        audioProcessor.waveViewer.setBufferSize((int)bufferSizeSlider.getValue());
        audioProcessor.InputViewer.setBufferSize((int)bufferSizeSlider.getValue());
        audioProcessor.CurveViewer.setBufferSize((int)bufferSizeSlider.getValue());
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

    addAndMakeVisible(crossoverSlider);
    crossoverSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    crossoverSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    crossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "CrossoverFreq", crossoverSlider);

    clipperCurveSlider.setName("Clipper Curve");
    clipperThresholdSlider.setName("Clipper Threshold");
    crossoverSlider.setName("Crossover");

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
    g.drawText("Trans Low", transientLowSlider.getBounds().translated(0, -40), juce::Justification::centred, false);
    g.drawText("Sus Low", sustainLowSlider.getBounds().translated(0, -40), juce::Justification::centred, false);
    g.drawText("Trans High", transientHighSlider.getBounds().translated(0, -40), juce::Justification::centred, false);
    g.drawText("Sus High", sustainHighSlider.getBounds().translated(0, -40), juce::Justification::centred, false);
    g.drawText("Hard/Soft", clipperCurveSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
    g.drawText("Ceiling", clipperThresholdSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
    g.drawText("X-Over", crossoverSlider.getBounds().translated(0, -40),
        juce::Justification::centred, false);
}

void DrumDecapitatorAudioProcessorEditor::resized()
{
    const int sliderSize = 80;
    const int sliderSpacing = 10;
    const int topMargin = 50;
    const int waveViewerHeight = 200;
    const int row2Y = topMargin + sliderSize + 40;
    const int row3Y = row2Y + sliderSize + 60;

    attackSlider.setBounds(10, topMargin, sliderSize, sliderSize);
    deltaAttackSlider.setBounds(100, topMargin, sliderSize, sliderSize);
    releaseSlider.setBounds(190, topMargin, sliderSize, sliderSize);
    offsetSlider.setBounds(280, topMargin, sliderSize, sliderSize);
    transientSlider.setBounds(370, topMargin, sliderSize, sliderSize);
    sustainSlider.setBounds(460, topMargin, sliderSize, sliderSize);
    mixSlider.setBounds(550, topMargin, sliderSize, sliderSize);
    clipperCurveSlider.setBounds(10, row2Y, sliderSize, sliderSize);
    clipperThresholdSlider.setBounds(100, row2Y, sliderSize, sliderSize);
    crossoverSlider.setBounds(550, row2Y, sliderSize, sliderSize);
    // Per-band controls layout
    transientLowSlider.setBounds(190, row2Y, sliderSize, sliderSize);
    sustainLowSlider.setBounds(280, row2Y, sliderSize, sliderSize);
    transientHighSlider.setBounds(370, row2Y, sliderSize, sliderSize);
    sustainHighSlider.setBounds(460, row2Y, sliderSize, sliderSize);
    lowMuteButton.setBounds(10, row3Y, 100, 24);
    lowSoloButton.setBounds(120, row3Y, 100, 24);
    highMuteButton.setBounds(230, row3Y, 100, 24);
    highSoloButton.setBounds(340, row3Y, 100, 24);
    int waveViewerTop = topMargin + sliderSize + sliderSpacing;
    audioProcessor.waveViewer.setBounds(10, waveViewerTop + 240, getWidth() - 40, waveViewerHeight);
    audioProcessor.InputViewer.setBounds(10, waveViewerTop + 240, getWidth() - 40, waveViewerHeight);
    audioProcessor.CurveViewer.setBounds(10, waveViewerTop + 240, getWidth() - 40, waveViewerHeight);
    bufferSizeSlider.setBounds(10, 10, getWidth() - 20, 40);

}
