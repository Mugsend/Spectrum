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

    btnLeft.onClick = [this]() { audioProcessor.currentChannelMode.store(0); };
    btnRight.onClick = [this]() { audioProcessor.currentChannelMode.store(1); };
    btnBoth.onClick = [this]() { audioProcessor.currentChannelMode.store(2); };

    
    
    addAndMakeVisible(meter);
    addAndMakeVisible(binSizeMenu);

    addAndMakeVisible(btnLeft);
    addAndMakeVisible(btnRight);
    addAndMakeVisible(btnBoth);
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

    meter.setBounds(bounds);
}
