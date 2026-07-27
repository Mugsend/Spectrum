#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        setDefaultSansSerifTypefaceName("Helvetica");
    }

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override
    {
        return juce::LookAndFeel_V4::getTypefaceForFont(font);
    }

    // --- BUTTONS ---
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        bool isOn = button.getToggleState();

        juce::Colour bgBase = juce::Colour(0xff333333); 
        juce::Colour bgHover = juce::Colour(0xff444444); 
        juce::Colour bgActive = juce::Colour(0xffffac00);

        juce::Colour currentBg = isOn ? bgActive : (shouldDrawButtonAsHighlighted ? bgHover : bgBase);


        g.setColour(currentBg);
        g.fillRect(bounds);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(bounds, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::Font font(12.0f);
        g.setFont(font);

        g.setColour(button.getToggleState() ? juce::Colour(0xff111111) : juce::Colours::lightgrey);
        g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, true);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
        int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float>(0, 0, width, height);

        g.setColour(juce::Colour(0xff333333));
        g.fillRect(bounds);
        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(bounds, 1.0f);

        juce::Path triangle;
        float centerX = buttonX + buttonW * 0.5f;
        float centerY = buttonY + buttonH * 0.5f;
        triangle.addTriangle(centerX - 4.0f, centerY - 2.0f,
            centerX + 4.0f, centerY - 2.0f,
            centerX, centerY + 3.0f);
        g.setColour(juce::Colours::grey);
        g.fillPath(triangle);
    }


    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        g.setColour(juce::Colour(0xff2a2a2a));
        g.fillRect(0, 0, width, height);

        g.setColour(juce::Colour(0xff1a1a1a));
        g.drawRect(0, 0, width, height, 1);
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
        bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
        const juce::String& text, const juce::String& shortcutKeyText,
        const juce::Drawable* icon, const juce::Colour* textColour) override
    {
        if (isSeparator)
        {
            auto r = area.reduced(5, 0);
            r.removeFromTop(juce::roundToInt((r.getHeight() * 0.5f) - 0.5f));
            g.setColour(juce::Colour(0xff1a1a1a));
            g.fillRect(r.withHeight(1));
            return;
        }

        if (isHighlighted && isActive)
        {
            g.setColour(juce::Colour(0xffffac00)); 
            g.fillRect(area);
        }

        g.setColour(isHighlighted && isActive ? juce::Colour(0xff111111) : juce::Colours::lightgrey);
        g.setFont(juce::Font(12.0f, juce::Font::bold));

        auto r = area.reduced(10, 0);

        if (isTicked)
        {
            auto tickArea = r.removeFromLeft(15).withSizeKeepingCentre(6, 6).toFloat();

            g.setColour(isHighlighted && isActive ? juce::Colour(0xff111111) : juce::Colour(0xffffac00));
            g.fillRect(tickArea);
        }
        else
        {
            r.removeFromLeft(15);
        }

        g.drawText(text, r, juce::Justification::centredLeft, true);
    }

    void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
        int standardMenuItemHeight, int& idealWidth, int& idealHeight) override
    {
        juce::LookAndFeel_V4::getIdealPopupMenuItemSize(text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight);

        idealHeight = isSeparator ? 8 : 24;

        idealWidth += 20;
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        auto trackRect = juce::Rectangle<float>(static_cast<float>(x), y + height * 0.5f - 2.0f, static_cast<float>(width), 4.0f);
        g.setColour(juce::Colour(0xff1a1a1a)); 
        g.fillRect(trackRect);

        
        auto thumbWidth = 8.0f;
        auto thumbRect = juce::Rectangle<float>(sliderPos - thumbWidth * 0.5f, static_cast<float>(y + 2), thumbWidth, static_cast<float>(height - 4));

        g.setColour(slider.isMouseOverOrDragging() ? juce::Colour(0xffcccccc) : juce::Colour(0xff999999));
        g.fillRect(thumbRect);
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

            if (val > frameMax) frameMax = val;
            if (val < frameMin) frameMin = val;

            if (val > maxData[i]) maxData[i] = val;
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
        g.fillAll(juce::Colour(0xff212121));

        int order = audioProcessor.currentFftOrder.load();
        int activeFftSize = 1 << order;
        int binsToDraw = activeFftSize / 2;
        if (binsToDraw == 0) return;

        float sampleRate = static_cast<float>(audioProcessor.getSampleRate());
        if (sampleRate <= 0.0f) return;

        auto width = static_cast<float>(getWidth());
        auto height = static_cast<float>(getHeight());

        float minLog = std::log(1.0f);
        float maxLog = std::log(static_cast<float>(binsToDraw - 1));
        float binToFreqFactor = sampleRate / static_cast<float>(activeFftSize);

        const float minMidi = 15.0f;
        const float maxMidi = 135.0f;

        int scaleMode = currentScaleMode.load();
        int meterMode = currentMeterMode.load();


        juce::Path spectrumPath;
        juce::Path maxPath;
        bool pathStarted = false;

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
            {
                float freq = i * binToFreqFactor;
                if (freq < 10.0f) continue;
                float pitch = 69.0f + 12.0f * std::log2(freq / 440.0f);
                xPos = juce::jmap(pitch, minMidi, maxMidi, 0.0f, width);
                break;
            }
            }

            float yPos = juce::jmap(smoothedData[i], currentMinDb, currentMaxDb, height, 0.0f);
            float maxYPos = juce::jmap(maxData[i], currentMinDb, currentMaxDb, height, 0.0f);
            g.setColour(juce::Colour(0xffffac00));
            if (meterMode == 0)
            {
                g.drawVerticalLine(juce::roundToInt(xPos), yPos, height);
            }

            if (!pathStarted)
            {
                
                spectrumPath.startNewSubPath(xPos, yPos);
                maxPath.startNewSubPath(xPos, maxYPos);
                pathStarted = true;
            }
            else
            {
                spectrumPath.lineTo(xPos, yPos);
                maxPath.lineTo(xPos, maxYPos);
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

        const float dashPattern[2] = { 2.0f, 4.0f };

        juce::Colour gridLineColor = juce::Colour(0xffffffff).withAlpha(0.12f); 
        juce::Colour gridTextColor = juce::Colour(0xffffffff).withAlpha(0.85f); 
        juce::Font gridFont(10.0f); 

        g.setFont(gridFont);


        if (scaleMode == 2)
        {
            for (int midiNote = static_cast<int>(minMidi); midiNote <= static_cast<int>(maxMidi); midiNote++)
            {
                float xPos = juce::jmap(static_cast<float>(midiNote), minMidi, maxMidi, 0.0f, width);
                float snappedX = std::round(xPos); // Use float for drawDashedLine
                bool isC = (midiNote % 12 == 0);

                if (isC)
                {
                    g.setColour(gridLineColor);
                    g.drawDashedLine(juce::Line<float>(snappedX, 0.0f, snappedX, height), dashPattern, 2, 0.5f);

                    juce::String noteName = "C" + juce::String(midiNote / 12);
                    g.setColour(gridTextColor);
                    g.drawText(noteName, static_cast<int>(snappedX) + 4, static_cast<int>(height) - 18, 30, 12, juce::Justification::left);
                }
            }
        }
        else
        {
            std::vector<float> displayFreqs = (scaleMode == 0)
                ? std::vector<float>{ 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f }
            : std::vector<float>{ 2000.0f, 4000.0f, 6000.0f, 8000.0f, 10000.0f, 12000.0f, 14000.0f, 16000.0f, 18000.0f, 20000.0f };

            for (float freq : displayFreqs)
            {
                float binIndex = freq / binToFreqFactor;
                float xPos = (scaleMode == 0) ? juce::jmap(std::log(binIndex), minLog, maxLog, 0.0f, width)
                    : juce::jmap(binIndex, 1.0f, static_cast<float>(binsToDraw - 1), 0.0f, width);

                float snapped_X = std::round(xPos);

                g.setColour(gridLineColor);
                g.drawDashedLine(juce::Line<float>(snapped_X, 0.0f, snapped_X, height), dashPattern, 2, 0.5f);

                juce::String freqText = freq >= 1000.0f ? juce::String(freq / 1000.0f, 0) + "k" : juce::String(freq, 0);
                g.setColour(gridTextColor);
                g.drawText(freqText, static_cast<int>(snapped_X) + 4, static_cast<int>(height) - 18, 30, 12, juce::Justification::left);
            }
        }


        const int dbStep = 12;
        float startDb = dbStep * std::floor(currentMinDb / dbStep);

        for (float db = startDb; db <= currentMaxDb; db += dbStep)
        {
            float yPos = std::round(juce::jmap(db, currentMinDb, currentMaxDb, height, 0.0f));

            if (yPos > 15.0f && yPos < height - 15.0f) 
            {
                g.setColour(gridLineColor);
                g.drawDashedLine(juce::Line<float>(0.0f, yPos, width, yPos), dashPattern, 2, 0.5f);

                g.setColour(gridTextColor);
                g.drawText(juce::String(db, 0) + " dB", 5, static_cast<int>(yPos) - 14, 40, 12, juce::Justification::left);
            }
        }

        if (isMouseOverGraph)
        {
            float currentDb = juce::jmap(mousePosition.y, height, 0.0f, currentMinDb, currentMaxDb);
            float normalisedX = mousePosition.x / width;
            float currentFreq = 0.0f;
            float currentPitch = 0.0f;

            if (scaleMode == 0)
            {
                float currentLog = normalisedX * (maxLog - minLog) + minLog;
                currentFreq = std::exp(currentLog) * binToFreqFactor;
                currentPitch = 69.0f + 12.0f * std::log2(currentFreq / 440.0f);
            }
            else if (scaleMode == 1)
            {
                float binIndex = normalisedX * (binsToDraw - 1);
                currentFreq = binIndex * binToFreqFactor;
                currentPitch = 69.0f + 12.0f * std::log2(currentFreq / 440.0f);
            }
            else if (scaleMode == 2)
            {
                currentPitch = normalisedX * (maxMidi - minMidi) + minMidi;
                currentFreq = 440.0f * std::pow(2.0f, (currentPitch - 69.0f) / 12.0f);
            }

            int midiRound = juce::roundToInt(currentPitch);
            int noteIndex = midiRound % 12;
            if (noteIndex < 0) noteIndex += 12;
            int octave = (midiRound / 12);

            juce::StringArray notes = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
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
            int boxY = static_cast<int>(height) - boxHeight - padding;

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

        juce::AttributedString branding;

        juce::Font brandFont("Helvetica Neue", 16.0f, juce::Font::bold);
        branding.append("MUGSEND", brandFont, juce::Colour(0xffffffff).withAlpha(0.9f));

        juce::Font pluginFont("Helvetica Neue", 16.0f, juce::Font::plain);
        branding.append(" SPECTRUM", pluginFont, juce::Colour(0xffffac00));

        branding.setJustification(juce::Justification::bottomLeft);

        float textHeight = 20.0f;
        float textY = 10.0f;
        float textWidth = 200.0f;
        float textX = 10.0f;

        branding.draw(g, juce::Rectangle<float>(textX, textY, textWidth, textHeight));

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
    juce::TextButton btnBoth{ "L+R" };

    juce::TextButton btnMax{ "Peak" };
    juce::TextButton btnLine{ "Line" };

    juce::TextButton btnLog{ "Log" };
    juce::TextButton btnLin{ "Lin" };
    juce::TextButton btnST{ "ST" };

    juce::TextButton btnDbRange{ "Auto" };

    juce::Slider sliderDbRangeMin;
    juce::Slider sliderDbRangeMax;

    CustomLookAndFeel customLook;
    juce::Label binSizeLabel;   

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAudioProcessorEditor)
};


