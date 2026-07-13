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
    MagnitudeMeter(std::vector<std::atomic<float>>& magValues) : magnitudeValues(magValues)
    {
        currentLevels.resize(magValues.size());
        startTimerHz(30);
    }

    ~MagnitudeMeter() override
    {
        stopTimer();
    }

    void timerCallback() override
    {
        for (size_t i = 0; i < currentLevels.size(); ++i)
        {
            currentLevels[i] = magnitudeValues[i].load();
        }
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        auto width = static_cast<float>(getWidth());
        auto height = static_cast<float>(getHeight());

        if (currentLevels.empty()) return;

        float binWidth = width / static_cast<float>(currentLevels.size());
        
        g.setColour(juce::Colours::cyan);
        for (size_t i = 0; i < currentLevels.size(); ++i) {
            float mappedY = height - (currentLevels[i] * height);
            mappedY = juce::jlimit(0.0f, height, mappedY);

            float xPos = i * binWidth;
            float barHeight = height - mappedY;

            g.fillRect(xPos, mappedY, binWidth - 1.0f, barHeight);
        }
    }

private:
    std::vector<std::atomic<float>>&magnitudeValues;
    std::vector<float> currentLevels;

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
    MagnitudeMeter meter{ audioProcessor.magnitudes };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAudioProcessorEditor)
};


