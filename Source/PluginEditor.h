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
    MagnitudeMeter(SpectrumAudioProcessor& p) : processor(p)
    {
        std::fill(smoothedData.begin(), smoothedData.end(), minDb);
        std::fill(maxData.begin(), maxData.end(), minDb);
        startTimerHz(60);
    }

    ~MagnitudeMeter() override
    {
        stopTimer();
    }
    
    void timerCallback() override
    {        
        int order = processor.currentFftOrder.load();
        int size = 1 << order;
        if (processor.nextFFTBlockReady.load())
        {
            if (order == 11)
            {
                processor.window2048.multiplyWithWindowingTable(processor.fftData.data(), size);
                processor.fft2048.performFrequencyOnlyForwardTransform(processor.fftData.data());
            }
            else if (order == 12)
            {
                processor.window4096.multiplyWithWindowingTable(processor.fftData.data(), size);
                processor.fft4096.performFrequencyOnlyForwardTransform(processor.fftData.data());
            }
            else if (order == 13)
            {
                processor.window8192.multiplyWithWindowingTable(processor.fftData.data(), size);
                processor.fft8192.performFrequencyOnlyForwardTransform(processor.fftData.data());
            }
            else if (order == 14)
            {
                processor.window16384.multiplyWithWindowingTable(processor.fftData.data(), size);
                processor.fft16384.performFrequencyOnlyForwardTransform(processor.fftData.data());
            }

            processor.nextFFTBlockReady.store(false);

            
        }

        int binsToDraw = size / 2;

        const float decayFactor = 0.85f;

        

        for (int i = 0; i < binsToDraw; ++i)
        {
            
            float rawMagnitude = processor.fftData[i];
            float targetDb = juce::Decibels::gainToDecibels(rawMagnitude) - juce::Decibels::gainToDecibels(static_cast<float>(size));

            
            targetDb = juce::jmax(targetDb, minDb);

            
            smoothedData[i] = (decayFactor * smoothedData[i]) + ((1.0f - decayFactor) * targetDb);
        }
        repaint();
    }

    void paint(juce::Graphics& g) override
    {


        int order = processor.currentFftOrder.load();
        int activeFftSize = 1 << order;
        int binsToDraw = activeFftSize / 2;
        if (binsToDraw == 0) return;

        float minLog = std::log(1.0f);
        float maxLog = std::log(static_cast<float>(binsToDraw - 1));

        auto width = static_cast<float>(getWidth());
        auto height = static_cast<float>(getHeight());

        float binWidth = width / static_cast<float>(binsToDraw);
        
        
        for (float db = minDb;db < maxDb;db += 12) 
        {
            float yPos = juce::jmap(db, -120.0f, 0.0f, height, 0.0f);
            float snapped_Y = static_cast<float>(juce::roundToInt(yPos));

            g.drawLine(0, snapped_Y, width, snapped_Y);
            g.setColour(juce::Colours::darkgrey);
            g.drawText(juce::String(db) + "db", 5, yPos - 12, 50, 10, juce::Justification::left);
            g.setColour(juce::Colours::lightgrey.withAlpha(0.1f));
        }
        float sampleRate = static_cast<float>(processor.getSampleRate());
        std::array<float, 10> displayFreqs = { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f };

        if (sampleRate>0.0f)
        {
            for (float freq : displayFreqs)
            {
                float binIndex = freq * (static_cast<float>(activeFftSize) / sampleRate);
                float currentLog = std::log(binIndex);
                float normalisedX = (currentLog - minLog) / (maxLog - minLog);
                float xPos = normalisedX * width;
                float snapped_X = static_cast<float>(juce::roundToInt(xPos));
                
                g.drawLine(snapped_X, height, snapped_X, 0);
                g.setColour(juce::Colours::darkgrey);
                juce::String freqText = freq >= 1000.0f ? juce::String(freq / 1000.0f) + "k" : juce::String(freq);
                g.drawText(freqText, snapped_X + 4, static_cast<int>(height) - 15, 30, 10, juce::Justification::left);
                g.setColour(juce::Colours::lightgrey.withAlpha(0.1f));


            }
        }
        
        const float dbRange = maxDb - minDb;

        g.setColour(juce::Colours::blue);
        juce::Path spectrumPath;
        juce::Path maxPath;

        for (int i = 1; i < binsToDraw; ++i) 
        {
            float currentLog = std::log(static_cast<float>(i));
            float normalisedX = (currentLog - minLog) / (maxLog - minLog);
            float xPos = normalisedX * width;
            float snapped_X = static_cast<float>(juce::roundToInt(xPos));

            float currentDb = smoothedData[i];
            float yPos = juce::jmap(currentDb, minDb, maxDb, height, 0.0f);

            float normalisedDb = (currentDb - minDb) / dbRange;

            float mappedY = height - (normalisedDb * height);
            
            if (i == 1)
            {
                spectrumPath.startNewSubPath(xPos, yPos);
            }
            else
            {
                spectrumPath.lineTo(xPos, yPos);
            }
                
        }
        g.strokePath(spectrumPath, juce::PathStrokeType(0.5f));
    }

private:
    const float minDb = -120.0f;
    const float maxDb = 0.0f;
    SpectrumAudioProcessor& processor;
    std::array<float, SpectrumAudioProcessor::maxFftSize / 2> smoothedData;
    std::array<float, SpectrumAudioProcessor::maxFftSize / 2> maxData;

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

    MagnitudeMeter meter{ audioProcessor };

    juce::ComboBox binSizeMenu;

    juce::TextButton btnLeft{ "L" };
    juce::TextButton btnRight{ "R" };
    juce::TextButton btnBoth{ "L + R" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAudioProcessorEditor)
};


