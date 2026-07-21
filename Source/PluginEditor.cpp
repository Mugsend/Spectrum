/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectrumAudioProcessorEditor::SpectrumAudioProcessorEditor (SpectrumAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{   
    binSizeMenu.addItem("2048", 1);
    binSizeMenu.addItem("4096", 2);
    binSizeMenu.addItem("8192", 3);
    binSizeMenu.addItem("16384", 4);

    binSizeMenu.setSelectedId(1);

    binSizeMenu.onChange = [this]()
        {
            int selectedId = binSizeMenu.getSelectedId();

            switch (selectedId)
            {
            case 1: audioProcessor.currentFftOrder.store(11); break;
            case 2: audioProcessor.currentFftOrder.store(12); break;
            case 3: audioProcessor.currentFftOrder.store(13); break;
            case 4: audioProcessor.currentFftOrder.store(14); break;
            }
        };

    btnLeft.setRadioGroupId(1);
    btnRight.setRadioGroupId(1);
    btnBoth.setRadioGroupId(1);

    btnLeft.setClickingTogglesState(true);
    btnRight.setClickingTogglesState(true);
    btnBoth.setClickingTogglesState(true);

    btnBoth.setToggleState(true, juce::dontSendNotification);

    btnLeft.setConnectedEdges(juce::Button::ConnectedOnRight);
    btnRight.setConnectedEdges(juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    btnBoth.setConnectedEdges(juce::Button::ConnectedOnLeft);

    btnLeft.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnLeft.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);

    btnLeft.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnLeft.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    btnRight.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnRight.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);
      
    btnRight.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnRight.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    btnBoth.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnBoth.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);

    btnBoth.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnBoth.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    btnLeft.onClick = [this]() { audioProcessor.currentChannelMode.store(0); };
    btnRight.onClick = [this]() { audioProcessor.currentChannelMode.store(1); };
    btnBoth.onClick = [this]() { audioProcessor.currentChannelMode.store(2); };

    btnMax.setClickingTogglesState(true);

    btnMax.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);    
    btnMax.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);
    
    btnMax.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    btnMax.setColour(juce::TextButton::textColourOnId, juce::Colours::black);

    btnMax.onClick = [this]
        {
            if (btnMax.getToggleState())
                meter.max.store(true);
            else
                meter.max.store(false);
        };
    
    btnLine.setClickingTogglesState(true);

    btnLine.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnLine.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);

    btnLine.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnLine.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    btnLine.onClick = [this]
        {
            if (btnLine.getToggleState()) 
            {   
                meter.meterMode.store(0);
                btnLine.setButtonText("Line");
            }
                
            else
            {
                meter.meterMode.store(1);
                btnLine.setButtonText("Path");
            }
                
        };
    btnLine.setToggleState(true, juce::dontSendNotification);

    addAndMakeVisible(meter);
    addAndMakeVisible(binSizeMenu);

    addAndMakeVisible(btnLeft);
    addAndMakeVisible(btnRight);
    addAndMakeVisible(btnBoth);

    addAndMakeVisible(btnMax);

    addAndMakeVisible(btnLine);

    setSize(1200,300);
}

SpectrumAudioProcessorEditor::~SpectrumAudioProcessorEditor()
{
}

//==============================================================================
void SpectrumAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void SpectrumAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto headerBounds = bounds.removeFromTop(40);

    binSizeMenu.setBounds(45,8,120,24);

    btnLeft.setBounds(45, 42, 35, 24);
    btnRight.setBounds(80, 42, 35, 24);
    btnBoth.setBounds(115, 42, 45, 24);

    btnMax.setBounds(45, 76, 45, 24);
    btnLine.setBounds(95, 76, 45, 24);

    meter.setBounds(bounds);
}
