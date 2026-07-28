#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MagnitudeMeter : public juce::Component, public juce::Timer
{

public:
    MagnitudeMeter(SpectrumAudioProcessor& p);
    ~MagnitudeMeter() override;

    void mouseMove(const juce::MouseEvent& event) override;
    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void timerCallback() override;
    void paint(juce::Graphics& g) override;

    std::atomic<bool> max = false;
    std::atomic<int> currentMeterMode = { 0 };
    std::atomic<int> currentScaleMode = { 0 };
    std::atomic<bool> isDbRangeAuto = true;
    const float minDb = -100.0f;
    const float maxDb = 0.0f;
    std::atomic<float> displayRangeMin;
    std::atomic<float> displayRangeMax;

private:
    void executeFftProcessing(int order, int size);
    void updateSmoothingAndBounds(int size, float& frameMin, float& frameMax);
    void updateAutoDbRanges(float frameMin, float frameMax);

    void drawSpectrum(juce::Graphics& g, int binsToDraw, float width, float height, float binToFreqFactor, float minLog, float maxLog);
    void drawGrid(juce::Graphics& g, float width, float height, float binToFreqFactor, float minLog, float maxLog);
    void drawDbScale(juce::Graphics& g, float width, float height);
    void drawTooltip(juce::Graphics& g, float width, float height, float binToFreqFactor, float minLog, float maxLog);
    void drawBranding(juce::Graphics& g);

    float getXPositionForBin(int i, float width, float binToFreqFactor, float minLog, float maxLog) const;
    float getPitchFromFreq(float freq) const;
    float getFreqFromPitch(float pitch) const;

    SpectrumAudioProcessor& audioProcessor;
    std::array<float, SpectrumAudioProcessor::maxFftSize / 2> smoothedData;
    std::array<float, SpectrumAudioProcessor::maxFftSize / 2> maxData;

    bool isMouseOverGraph = false;
    juce::Point<float> mousePosition;

    const float minMidi = 15.0f;
    const float maxMidi = 135.0f;

    float currentMaxDb = maxDb;
    float currentMinDb = minDb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MagnitudeMeter)
};