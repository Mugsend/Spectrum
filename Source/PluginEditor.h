#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        setColour(juce::Slider::backgroundColourId, juce::Colour(0xff2d2d2d));
        setColour(juce::Slider::trackColourId, juce::Colour(0xff57b846));
        setColour(juce::Slider::thumbColourId, juce::Colour(0xffffffff));

        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffffffff));
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        float alpha = slider.isEnabled() ? 1.0f : 0.4f;
        juce::Rectangle<float> trackBounds(x, y, width, height);

        g.setColour(slider.findColour(juce::Slider::backgroundColourId).withMultipliedAlpha(alpha));
        g.fillRect(trackBounds);

        if (slider.isHorizontal())
        {
            juce::Rectangle<float> fillRect(trackBounds.getX(), trackBounds.getY(),
                sliderPos - trackBounds.getX(), trackBounds.getHeight());
            g.setColour(slider.findColour(juce::Slider::trackColourId).withMultipliedAlpha(alpha));
            g.fillRect(fillRect);

        }
        else 
        {
            juce::Rectangle<float> fillRect(trackBounds.getX(), sliderPos,
                trackBounds.getWidth(), trackBounds.getBottom() - sliderPos);
            g.setColour(slider.findColour(juce::Slider::trackColourId).withMultipliedAlpha(alpha));
            g.fillRect(fillRect);
        }

        juce::String text = slider.getTextFromValue(slider.getValue());

        g.setColour(slider.findColour(juce::Slider::textBoxTextColourId).withMultipliedAlpha(alpha));
        g.setFont(10.0f);

        g.drawText(text, trackBounds.toNearestInt(), juce::Justification::centred, false);
    }
};
class MagnitudeMeter : public juce::Component, public juce::Timer
{
public:
    MagnitudeMeter(SpectrumAudioProcessor& p) : audioProcessor(p)
    {
        std::fill(smoothedData.begin(), smoothedData.end(), minDb);
        std::fill(maxData.begin(), maxData.end(), minDb);
        startTimerHz(60);
    }

    ~MagnitudeMeter() override
    {
        stopTimer();
    }
    
    void mouseMove(const juce::MouseEvent& event) override
    {
        mousePosition = event.position;
    }

    void mouseEnter(const juce::MouseEvent& event) override
    {
        isMouseOverGraph = true;
    }

    void mouseExit(const juce::MouseEvent& event) override
    {
        isMouseOverGraph = false;
    }

    void timerCallback() override
    {        
        int order = audioProcessor.currentFftOrder.load();
        int size = 1 << order;
        if (audioProcessor.nextFFTBlockReady.load())
        {
            if (order == 11)
            {
                audioProcessor.window2048.multiplyWithWindowingTable(audioProcessor.fftData.data(), size);
                audioProcessor.fft2048.performFrequencyOnlyForwardTransform(audioProcessor.fftData.data());
            }
            else if (order == 12)
            {
                audioProcessor.window4096.multiplyWithWindowingTable(audioProcessor.fftData.data(), size);
                audioProcessor.fft4096.performFrequencyOnlyForwardTransform(audioProcessor.fftData.data());
            }
            else if (order == 13)
            {
                audioProcessor.window8192.multiplyWithWindowingTable(audioProcessor.fftData.data(), size);
                audioProcessor.fft8192.performFrequencyOnlyForwardTransform(audioProcessor.fftData.data());
            }
            else if (order == 14)
            {
                audioProcessor.window16384.multiplyWithWindowingTable(audioProcessor.fftData.data(), size);
                audioProcessor.fft16384.performFrequencyOnlyForwardTransform(audioProcessor.fftData.data());
            }

            audioProcessor.nextFFTBlockReady.store(false);
        }

        int binsToDraw = size / 2;

        const float decayFactor = 0.85f;

        float frameMax = minDb;
        float frameMin = maxDb;

        for (int i = 1; i < binsToDraw; ++i)
        {            
            float rawMagnitude = audioProcessor.fftData[i];
            float targetDb = juce::Decibels::gainToDecibels(rawMagnitude) - juce::Decibels::gainToDecibels(static_cast<float>(size));

            targetDb = juce::jmax(targetDb, minDb);

            smoothedData[i] = (decayFactor * smoothedData[i]) + ((1.0f - decayFactor) * targetDb);

            float val = smoothedData[i];

            if (val < frameMin) frameMin = val;

            if (maxData[i] > frameMax) frameMax = maxData[i];
        }

        frameMax += 10.0f;
        frameMin -= 10.0f;

        frameMax = juce::jlimit(-40.0f, 12.0f, frameMax);
        frameMin = juce::jlimit(minDb, -60.0f, frameMin);

        if ((frameMax - frameMin) < 60.0f)
        {
            frameMin = frameMax - 60.0f;
        }

        if (isDbRangeAuto.load()) 
        {
            currentMaxDb += (frameMax - currentMaxDb) * 0.1f;
            currentMinDb += (frameMin - currentMinDb) * 0.1f;
        } 

        repaint();
    }

    void paint(juce::Graphics& g) override
    {

        int order = audioProcessor.currentFftOrder.load();
        int activeFftSize = 1 << order;
        int binsToDraw = activeFftSize / 2;
        if (binsToDraw == 0) return;

        float sampleRate = static_cast<float>(audioProcessor.getSampleRate());

        float minLog = std::log(1.0f);
        float maxLog = std::log(static_cast<float>(binsToDraw - 1));

        auto width = static_cast<float>(getWidth());
        auto height = static_cast<float>(getHeight());

        float binWidth = width / static_cast<float>(binsToDraw);
        const float minMidi = 15.0f;
        const float maxMidi = 135.0f;
        
        int scaleMode = currentScaleMode.load();
        int meterMode = currentMeterMode.load();

        g.setColour(juce::Colours::blue);

        juce::Path spectrumPath;
        juce::Path maxPath;

        bool pathStarted = false;


        const int dbStep = 12;
        float startDb = dbStep * std::floor(currentMinDb / dbStep);
        for (int i = 1; i < binsToDraw; ++i) 
        {
            float xPos = 0;

            switch (scaleMode) 
            {
                case 0:
                {
                    float currentLog = std::log(static_cast<float>(i));
                    xPos = juce::jmap(currentLog, minLog, maxLog, 0.0f, width);
                    break;
                }
                case 1:
                    xPos = juce::jmap(static_cast<float>(i), 1.0f, static_cast<float>(binsToDraw - 1), 0.0f, width);
                    break;
                case 2:
                    float freq = i * (sampleRate / static_cast<float>(activeFftSize));

                    if (freq < 10.0f) continue;

                    float pitch = 69.0f + 12.0f * std::log2(freq / 440.0f);

                    xPos = juce::jmap(pitch, minMidi, maxMidi, 0.0f, width);
                    break;
            }

            float snapped_X = static_cast<float>(juce::roundToInt(xPos));

            float currentDb = smoothedData[i];

            float yPos = juce::jmap(currentDb, currentMinDb, currentMaxDb, height, 0.0f);

            
            if (currentDb > maxData[i]) 
            {
                maxData[i] = currentDb;
            }

            if (currentMeterMode.load() == 0)
            {
                g.drawVerticalLine(snapped_X, yPos , height);
            }

            if (!pathStarted)
            {
                
                spectrumPath.startNewSubPath(xPos, yPos);
                maxPath.startNewSubPath(xPos,juce::jmap(maxData[i], currentMinDb, currentMaxDb, height, 0.0f));
                pathStarted = true;
            }
            else
            {
                spectrumPath.lineTo(xPos, yPos);
                maxPath.lineTo(xPos, juce::jmap(maxData[i], currentMinDb, currentMaxDb, height, 0.0f));

            }
                
        }
        if (meterMode == 1)
        {
            g.strokePath(spectrumPath, juce::PathStrokeType(0.5f));

        }

        if (max.load())
        {
            g.strokePath(maxPath, juce::PathStrokeType(0.5f));
        }

        g.setFont(10.0f);

        if (scaleMode == 2)
        {
            for (int midiNote = static_cast<int> (minMidi); midiNote <= static_cast<int>(maxMidi); midiNote++)
            {
                float xPos = juce::jmap(static_cast<float> (midiNote), minMidi, maxMidi, 0.0f, width);
                int snappedX = juce::roundToInt(xPos);

                bool isC = (midiNote % 12 == 0);

                if (isC)
                {
                    g.setColour(juce::Colours::darkgrey.withAlpha(0.8f));
                    g.drawVerticalLine(snappedX, 0.0f, height);

                    int octave = midiNote / 12;
                    juce::String noteName = "C" + juce::String(octave);

                    g.setColour(juce::Colours::lightgrey);
                    g.drawText(noteName, snappedX + 4, static_cast<int>(height) - 15, 30, 10, juce::Justification::left);
                }
                else
                {
                    g.setColour(juce::Colours::darkgrey.withAlpha(0.15f));
                    g.drawVerticalLine(snappedX, 0.0f, height);
                }
            }
        }

        else
        {
            if (sampleRate > 0.0f)
            {   
                std::vector<float> displayFreqs;
                
                if (scaleMode == 0) 
                    displayFreqs = { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f };
                
                else
                    displayFreqs = { 2000.0f, 4000.0f, 6000.0f, 8000.0f, 10000.0f, 12000.0f, 14000.0f, 16000.0f, 18000.0f, 20000.0f };

                for (float freq : displayFreqs)
                {
                    float binIndex = freq * (static_cast<float>(activeFftSize) / sampleRate);
                    float xPos = 0;
                    if (scaleMode == 0) {
                        float currentLog = std::log(binIndex);
                        xPos = juce::jmap(currentLog, minLog, maxLog, 0.0f, width);
                    }
                    else {
                        xPos = juce::jmap(binIndex, 1.0f, static_cast<float>(binsToDraw - 1), 0.0f, width);

                    }
                    int snapped_X = juce::roundToInt(xPos);

                    g.setColour(juce::Colours::darkgrey.withAlpha(0.8f));
                    g.drawVerticalLine(snapped_X, 0, height);
                    juce::String freqText = freq >= 1000.0f ? juce::String(freq / 1000.0f) + "k" : juce::String(freq);
                    g.setColour(juce::Colours::lightgrey);
                    g.drawText(freqText, snapped_X + 4, static_cast<int>(height) - 15, 30, 10, juce::Justification::left);
                }
            }
        }

        for (float db = startDb; db <= currentMaxDb; db += dbStep)
        {
            float yPos = juce::jmap(db, currentMinDb, currentMaxDb, height, 0.0f);
            int snapped_Y = juce::roundToInt(yPos);

            g.setColour(juce::Colours::darkgrey.withAlpha(0.8f));
            g.drawHorizontalLine(snapped_Y, 0, width);
            g.setColour(juce::Colours::lightgrey);
            g.drawText(juce::String(db) + "db", 5, yPos - 12, 50, 10, juce::Justification::left);
        }

        if (isMouseOverGraph) 
        {
            if (sampleRate > 0.0f)
            {
                float currentDb = juce::jmap(mousePosition.y, height, 0.0f, currentMinDb, currentMaxDb);

                float normalisedX = mousePosition.x / width;
                float currentFreq = 0.0f;
                float currentPitch = 0.0f;

                if (scaleMode == 0)
                {
                    float currentLog = normalisedX * (maxLog - minLog) + minLog;
                    float binIndex = std::exp(currentLog);

                    currentFreq = binIndex * (sampleRate / static_cast<float>(activeFftSize));
                    currentPitch = 69.0f + 12.0f * std::log2(currentFreq / 440.0f);
                }
                else if (scaleMode == 1)
                {
                    float binIndex = normalisedX * (binsToDraw - 1);

                    currentFreq = binIndex * (sampleRate / static_cast<float>(activeFftSize));
                    currentPitch = 69.0f + 12.0f * std::log2(currentFreq / 440.0f);
                }
                else if (scaleMode == 2)
                {
                    currentPitch = normalisedX * (maxMidi - minMidi) + minMidi;
                    currentFreq = 440.0f * std::pow(2.0f, (currentPitch - 69.0f) / 12.0f);
                }

                int midiRound = juce::roundToInt(currentPitch);
                juce::StringArray notes = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

                int noteIndex = midiRound % 12;
                if (noteIndex < 0) noteIndex += 12;
                int octave = (midiRound / 12);

                juce::String musicalNote = notes[noteIndex] + juce::String(octave);

                juce::String freqString = (currentFreq >= 1000.0f)
                    ? juce::String(currentFreq / 1000.0f, 2) + " kHz"
                    : juce::String(currentFreq, 1) + " Hz";

                juce::String dbString = juce::String(currentDb, 1) + " dB";

                juce::String tooltipText = musicalNote + " (" + freqString + ") | " + dbString;

                int boxWidth = 160;
                int boxHeight = 24;
                int padding = 10;

                int boxX = 45;
                int boxY = height-boxHeight - padding;

                g.setColour(juce::Colours::black.withAlpha(0.8f));
                g.fillRoundedRectangle(static_cast<float>(boxX), static_cast<float>(boxY),
                    static_cast<float>(boxWidth), static_cast<float>(boxHeight), 4.0f);

                g.setColour(juce::Colours::grey);
                g.drawRoundedRectangle(static_cast<float>(boxX), static_cast<float>(boxY),
                    static_cast<float>(boxWidth), static_cast<float>(boxHeight), 4.0f, 1.0f);

                g.setColour(juce::Colours::white);
                g.setFont(12.0f);
                g.drawText(tooltipText, boxX, boxY, boxWidth, boxHeight, juce::Justification::centred, true);
            }
        }

    }
    std::atomic<bool> max = false;

    std::atomic<int> currentMeterMode = { 0 };
    std::atomic<int> currentScaleMode = { 0 };

    std::atomic<bool> isDbRangeAuto = true;

    bool isMouseOverGraph = false;
    juce::Point<float> mousePosition;

    const float minDb = -200.0f;
    const float maxDb = 0.0f;


    float currentMaxDb = maxDb;
    float currentMinDb = minDb;

private:
    

   

    SpectrumAudioProcessor& audioProcessor;
    std::array<float, SpectrumAudioProcessor::maxFftSize / 2> smoothedData;
    std::array<float, SpectrumAudioProcessor::maxFftSize / 2> maxData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MagnitudeMeter)
};

class SpectrumAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SpectrumAudioProcessorEditor (SpectrumAudioProcessor&);
    ~SpectrumAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    
private:
    SpectrumAudioProcessor& audioProcessor;

    MagnitudeMeter meter{ audioProcessor };

    juce::ComboBox binSizeMenu;

    juce::TextButton btnLeft{ "L" };
    juce::TextButton btnRight{ "R" };
    juce::TextButton btnBoth{ "L + R" };

    juce::TextButton btnMax{ "Max" };
    juce::TextButton btnLine{ "Line" };

    juce::TextButton btnLog{ "Log" };
    juce::TextButton btnLin{ "Lin" };
    juce::TextButton btnST{ "ST" };

    juce::TextButton btnDbRange{ "Auto" };

    juce::Slider sliderDbRangeMin;
    juce::Slider sliderDbRangeMax;

    CustomLookAndFeel customLook;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAudioProcessorEditor)
};


