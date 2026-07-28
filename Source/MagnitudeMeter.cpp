#include "MagnitudeMeter.h"

MagnitudeMeter::MagnitudeMeter(SpectrumAudioProcessor& p) : audioProcessor(p)
{
    displayRangeMin = minDb;
    displayRangeMax = maxDb;
    std::fill(smoothedData.begin(), smoothedData.end(), minDb);
    std::fill(maxData.begin(), maxData.end(), minDb);
    startTimerHz(60);
}

MagnitudeMeter::~MagnitudeMeter()
{
    stopTimer();
}

void MagnitudeMeter::mouseMove(const juce::MouseEvent& event)
{
    mousePosition = event.position;
}

void MagnitudeMeter::mouseEnter(const juce::MouseEvent& event)
{
    isMouseOverGraph = true;
}

void MagnitudeMeter::mouseExit(const juce::MouseEvent& event)
{
    isMouseOverGraph = false;
}

void MagnitudeMeter::timerCallback()
{
    int order = audioProcessor.currentFftOrder.load();
    int size = 1 << order;

    executeFftProcessing(order, size);

    float frameMax = minDb;
    float frameMin = maxDb;

    updateSmoothingAndBounds(size, frameMin, frameMax);
    updateAutoDbRanges(frameMin, frameMax);

    repaint();
}

void MagnitudeMeter::executeFftProcessing(int order, int size)
{
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
}

void MagnitudeMeter::updateSmoothingAndBounds(int size, float& frameMin, float& frameMax)
{
    int binsToDraw = size / 2;
    const float decayFactor = 0.85f;
    float sizeNormalization = 1.0f / static_cast<float>(size);

    for (int i = 1; i < binsToDraw; ++i)
    {
        float rawMagnitude = audioProcessor.fftData[i];
        float targetDb = juce::Decibels::gainToDecibels(rawMagnitude * sizeNormalization, minDb);
        targetDb = juce::jmax(targetDb, minDb);

        smoothedData[i] = (decayFactor * smoothedData[i]) + ((1.0f - decayFactor) * targetDb);
        float val = smoothedData[i];

        if (val > frameMax) frameMax = val;
        if (val < frameMin) frameMin = val;
        if (val > maxData[i]) maxData[i] = val;
    }
}

void MagnitudeMeter::updateAutoDbRanges(float frameMin, float frameMax)
{
    frameMax += 10.0f;
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
        displayRangeMax.store(currentMaxDb);
        displayRangeMin.store(currentMinDb);
    }
    else
    {
        currentMaxDb = displayRangeMax.load();
        currentMinDb = displayRangeMin.load();
    }
}

void MagnitudeMeter::paint(juce::Graphics& g)
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

    drawSpectrum(g, binsToDraw, width, height, binToFreqFactor, minLog, maxLog);
    drawGrid(g, width, height, binToFreqFactor, minLog, maxLog);
    drawDbScale(g, width, height);

    if (isMouseOverGraph)
    {
        drawTooltip(g, width, height, binToFreqFactor, minLog, maxLog);
    }

    drawBranding(g);
}

float MagnitudeMeter::getXPositionForBin(int i, float width, float binToFreqFactor, float minLog, float maxLog) const
{
    int scaleMode = currentScaleMode.load();

    switch (scaleMode)
    {
    case 0:
    {
        float currentLog = std::log(static_cast<float>(i));
        return juce::jmap(currentLog, minLog, maxLog, 0.0f, width);
    }
    case 1:
        return juce::jmap(static_cast<float>(i), 1.0f, static_cast<float>(std::exp(maxLog)), 0.0f, width);
    case 2:
    {
        float freq = i * binToFreqFactor;
        float pitch = getPitchFromFreq(freq);
        return juce::jmap(pitch, minMidi, maxMidi, 0.0f, width);
    }
    }
    return 0.0f;
}

float MagnitudeMeter::getPitchFromFreq(float freq) const
{
    return 69.0f + 12.0f * std::log2(freq / 440.0f);
}

float MagnitudeMeter::getFreqFromPitch(float pitch) const
{
    return 440.0f * std::pow(2.0f, (pitch - 69.0f) / 12.0f);
}

void MagnitudeMeter::drawSpectrum(juce::Graphics& g, int binsToDraw, float width, float height, float binToFreqFactor, float minLog, float maxLog)
{
    juce::Path spectrumPath;
    juce::Path maxPath;
    bool pathStarted = false;
    int meterMode = currentMeterMode.load();

    g.setColour(juce::Colour(0xffffac00));

    for (int i = 1; i < binsToDraw; ++i)
    {
        float xPos = getXPositionForBin(i, width, binToFreqFactor, minLog, maxLog);
        float yPos = juce::jmap(smoothedData[i], currentMinDb, currentMaxDb, height, 0.0f);
        float maxYPos = juce::jmap(maxData[i], currentMinDb, currentMaxDb, height, 0.0f);

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
}

void MagnitudeMeter::drawGrid(juce::Graphics& g, float width, float height, float binToFreqFactor, float minLog, float maxLog)
{
    const float dashPattern[2] = { 2.0f, 4.0f };
    juce::Colour gridLineColor = juce::Colour(0xffffffff).withAlpha(0.12f);
    juce::Colour gridTextColor = juce::Colour(0xffffffff).withAlpha(0.85f);
    juce::Font gridFont(10.0f);

    g.setFont(gridFont);
    int scaleMode = currentScaleMode.load();

    if (scaleMode == 2)
    {
        for (int midiNote = static_cast<int>(minMidi); midiNote <= static_cast<int>(maxMidi); midiNote++)
        {
            float xPos = juce::jmap(static_cast<float>(midiNote), minMidi, maxMidi, 0.0f, width);
            float snappedX = std::round(xPos);
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
                : juce::jmap(binIndex, 1.0f, static_cast<float>(std::exp(maxLog)), 0.0f, width);

            float snapped_X = std::round(xPos);

            g.setColour(gridLineColor);
            g.drawDashedLine(juce::Line<float>(snapped_X, 0.0f, snapped_X, height), dashPattern, 2, 0.5f);

            juce::String freqText = freq >= 1000.0f ? juce::String(freq / 1000.0f, 0) + "k" : juce::String(freq, 0);
            g.setColour(gridTextColor);
            g.drawText(freqText, static_cast<int>(snapped_X) + 4, static_cast<int>(height) - 18, 30, 12, juce::Justification::left);
        }
    }
}

void MagnitudeMeter::drawDbScale(juce::Graphics& g, float width, float height)
{
    const float dashPattern[2] = { 2.0f, 4.0f };
    juce::Colour gridLineColor = juce::Colour(0xffffffff).withAlpha(0.12f);
    juce::Colour gridTextColor = juce::Colour(0xffffffff).withAlpha(0.85f);

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
}

void MagnitudeMeter::drawTooltip(juce::Graphics& g, float width, float height, float binToFreqFactor, float minLog, float maxLog)
{
    float currentDb = juce::jmap(mousePosition.y, height, 0.0f, currentMinDb, currentMaxDb);
    float normalisedX = mousePosition.x / width;
    float currentFreq = 0.0f;
    float currentPitch = 0.0f;
    int scaleMode = currentScaleMode.load();

    if (scaleMode == 0)
    {
        float currentLog = normalisedX * (maxLog - minLog) + minLog;
        currentFreq = std::exp(currentLog) * binToFreqFactor;
        currentPitch = getPitchFromFreq(currentFreq);
    }
    else if (scaleMode == 1)
    {
        float binIndex = normalisedX * std::exp(maxLog);
        currentFreq = binIndex * binToFreqFactor;
        currentPitch = getPitchFromFreq(currentFreq);
    }
    else if (scaleMode == 2)
    {
        currentPitch = normalisedX * (maxMidi - minMidi) + minMidi;
        currentFreq = getFreqFromPitch(currentPitch);
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

void MagnitudeMeter::drawBranding(juce::Graphics& g)
{
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