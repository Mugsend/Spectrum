/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MagnitudeMeter : public juce::Component, public juce::Timer
{
public:
    // Pass in a reference to your atomic magnitude variable
    MagnitudeMeter(std::array<std::atomic<float>,SpectrumAudioProcessor::maxBins>& magValues, std::atomic<int>& activeBins) : magnitudeValues(magValues), activeNumBins(activeBins)
    {
        startTimerHz(60);
    }

    ~MagnitudeMeter() override
    {
        stopTimer();
    }

    void timerCallback() override
    {
        
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        int binsToDraw = activeNumBins.load();
        if (binsToDraw == 0) return;

        auto width = static_cast<float>(getWidth());
        auto height = static_cast<float>(getHeight());

        float binWidth = width / static_cast<float>(binsToDraw);
        
        const float minDb = -100.0f;
        const float maxDb = 0.0f;
        const float dbRange = maxDb - minDb;

        g.setColour(juce::Colours::blue);

        for (int i = 0; i < binsToDraw; ++i) {
            float currentDb = magnitudeValues[i].load();

            float normalisedDb = (currentDb - minDb) / dbRange;

            float mappedY = height - (normalisedDb * height);
            float xPos = i * binWidth;
            float barHeight = height - mappedY;

            //g.fillRect(xPos, mappedY, binWidth - 1.0f, barHeight);
            g.drawLine(xPos, height, xPos, barHeight);
                
        }
    }

private:
    std::array<std::atomic<float>, SpectrumAudioProcessor::maxBins>&magnitudeValues;
    std::atomic<int>& activeNumBins;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MagnitudeMeter)
};
//==============================================================================
/**
*/
class SpectrumAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SpectrumAudioProcessorEditor (SpectrumAudioProcessor&);
    ~SpectrumAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SpectrumAudioProcessor& audioProcessor;

    MagnitudeMeter meter{ audioProcessor.magnitudes, audioProcessor.activeNumBins };

    juce::ComboBox binSizeMenu;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAudioProcessorEditor)
};


