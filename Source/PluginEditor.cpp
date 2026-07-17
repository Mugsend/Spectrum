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
    
    addAndMakeVisible(meter);
    addAndMakeVisible(binSizeMenu);

    binSizeMenu.addItem("128 Bins", 1);
    binSizeMenu.addItem("256 Bins", 2);
    binSizeMenu.addItem("512 Bins", 3);
    binSizeMenu.addItem("1024 Bins", 4);
    binSizeMenu.addItem("2048 Bins", 5);
    binSizeMenu.addItem("4096 Bins", 6);
    binSizeMenu.addItem("8192 Bins", 7);
     
    binSizeMenu.onChange = [this]()
        {
            int selectedId = binSizeMenu.getSelectedId();

            int newFftSize = 512;

            switch (selectedId)
            {
            case 1: newFftSize = 256;  break;
            case 2: newFftSize = 512;  break;
            case 3: newFftSize = 1024; break;
            case 4: newFftSize = 2048; break;
            case 5: newFftSize = 4096; break;
            case 6: newFftSize = 8192; break;
            case 7: newFftSize = 16384; break;
            }

            audioProcessor.currentFftSize.store(newFftSize);
        };

    binSizeMenu.setSelectedId(2);

    setSize(1000,300);
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
    auto bounds = getLocalBounds().reduced(10);

    binSizeMenu.setBounds(bounds.removeFromTop(30));

    bounds.removeFromTop(10);

    meter.setBounds(bounds);
}
