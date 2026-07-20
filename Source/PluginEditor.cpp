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

    addAndMakeVisible(meter);
    addAndMakeVisible(binSizeMenu);
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

    binSizeMenu.setBounds(45,8,120,24);


    meter.setBounds(bounds);
}
