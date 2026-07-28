#include "PluginProcessor.h"
#include "PluginEditor.h"

SpectrumAudioProcessorEditor::SpectrumAudioProcessorEditor(SpectrumAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLook);

    setupBinSizeMenu();
    setupChannelButtons();
    setupScaleButtons();
    setupDisplayButtons();
    setupDbRangeControls();

    addAndMakeVisible(meter);
    setSize(1200, 300);
}

SpectrumAudioProcessorEditor::~SpectrumAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void SpectrumAudioProcessorEditor::setupBinSizeMenu()
{
    binSizeLabel.setText("FFT Size:", juce::dontSendNotification);
    binSizeLabel.setJustificationType(juce::Justification::centredLeft);
    binSizeLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    binSizeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(binSizeLabel);

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

    addAndMakeVisible(binSizeMenu);
}

void SpectrumAudioProcessorEditor::setupChannelButtons()
{
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

    addAndMakeVisible(btnLeft);
    addAndMakeVisible(btnRight);
    addAndMakeVisible(btnBoth);
}

void SpectrumAudioProcessorEditor::setupScaleButtons()
{
    btnLog.setRadioGroupId(2);
    btnLin.setRadioGroupId(2);
    btnST.setRadioGroupId(2);

    btnLog.setClickingTogglesState(true);
    btnLin.setClickingTogglesState(true);
    btnST.setClickingTogglesState(true);

    btnLog.setToggleState(true, juce::dontSendNotification);

    btnLog.setConnectedEdges(juce::Button::ConnectedOnRight);
    btnLin.setConnectedEdges(juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    btnST.setConnectedEdges(juce::Button::ConnectedOnLeft);

    btnLog.onClick = [this]() { meter.currentScaleMode.store(0); };
    btnLin.onClick = [this]() { meter.currentScaleMode.store(1); };
    btnST.onClick = [this]() { meter.currentScaleMode.store(2); };

    addAndMakeVisible(btnLog);
    addAndMakeVisible(btnLin);
    addAndMakeVisible(btnST);
}

void SpectrumAudioProcessorEditor::setupDisplayButtons()
{
    btnMax.setClickingTogglesState(true);

    btnMax.onClick = [this]
        {
            meter.max.store(btnMax.getToggleState());
        };

    btnLine.setClickingTogglesState(true);
    btnLine.setToggleState(true, juce::dontSendNotification);

    btnLine.onClick = [this]
        {
            if (btnLine.getToggleState())
            {
                meter.currentMeterMode.store(0);
                btnLine.setButtonText("Line");
            }
            else
            {
                meter.currentMeterMode.store(1);
                btnLine.setButtonText("Bins");
            }
        };

    addAndMakeVisible(btnMax);
    addAndMakeVisible(btnLine);
}

void SpectrumAudioProcessorEditor::setupDbRangeControls()
{
    btnDbRange.setClickingTogglesState(true);
    btnDbRange.setToggleState(true, juce::dontSendNotification);

    sliderDbRangeMin.setRange(meter.minDb, meter.maxDb, 1.0);
    sliderDbRangeMax.setRange(meter.minDb, meter.maxDb, 1.0);
    sliderDbRangeMin.setValue(meter.minDb, juce::dontSendNotification);
    sliderDbRangeMax.setValue(meter.maxDb, juce::dontSendNotification);

    sliderDbRangeMin.setNumDecimalPlacesToDisplay(0);
    sliderDbRangeMax.setNumDecimalPlacesToDisplay(0);
    sliderDbRangeMin.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    sliderDbRangeMax.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    sliderDbRangeMin.setSliderStyle(juce::Slider::LinearHorizontal);
    sliderDbRangeMax.setSliderStyle(juce::Slider::LinearHorizontal);
    sliderDbRangeMin.setEnabled(false);
    sliderDbRangeMax.setEnabled(false);

    btnDbRange.onClick = [this]
        {
            if (btnDbRange.getToggleState())
            {
                meter.isDbRangeAuto.store(true);
                sliderDbRangeMin.setEnabled(false);
                sliderDbRangeMax.setEnabled(false);
            }
            else
            {
                sliderDbRangeMin.setValue(static_cast<int>(meter.displayRangeMin.load()), juce::dontSendNotification);
                sliderDbRangeMax.setValue(static_cast<int>(meter.displayRangeMax.load()), juce::dontSendNotification);
                meter.isDbRangeAuto.store(false);
                sliderDbRangeMin.setEnabled(true);
                sliderDbRangeMax.setEnabled(true);
            }
        };

    sliderDbRangeMin.onValueChange = [this]
        {
            float newMin = static_cast<float>(sliderDbRangeMin.getValue());
            float maxAllowedMin = meter.maxDb - 60.0f;

            if (newMin > maxAllowedMin)
            {
                newMin = maxAllowedMin;
                sliderDbRangeMin.setValue(newMin, juce::dontSendNotification);
            }

            float currentMax = static_cast<float>(sliderDbRangeMax.getValue());
            if (currentMax - newMin < 60.0f)
            {
                float requiredMax = newMin + 60.0f;
                sliderDbRangeMax.setValue(requiredMax, juce::dontSendNotification);
                meter.displayRangeMax.store(requiredMax);
            }
            else
            {
                meter.displayRangeMax.store(currentMax);
            }

            meter.displayRangeMin.store(newMin);
        };

    sliderDbRangeMax.onValueChange = [this]
        {
            float newMax = static_cast<float>(sliderDbRangeMax.getValue());
            float minAllowedMax = meter.minDb + 60.0f;

            if (newMax < minAllowedMax)
            {
                newMax = minAllowedMax;
                sliderDbRangeMax.setValue(newMax, juce::dontSendNotification);
            }

            float currentMin = static_cast<float>(sliderDbRangeMin.getValue());
            if (newMax - currentMin < 60.0f)
            {
                float requiredMin = newMax - 60.0f;
                sliderDbRangeMin.setValue(requiredMin, juce::dontSendNotification);
                meter.displayRangeMin.store(requiredMin);
            }
            else
            {
                meter.displayRangeMin.store(currentMin);
            }

            meter.displayRangeMax.store(newMax);
        };

    addAndMakeVisible(btnDbRange);
    addAndMakeVisible(sliderDbRangeMin);
    addAndMakeVisible(sliderDbRangeMax);
}

void SpectrumAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void SpectrumAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto controlPanel = bounds.removeFromLeft(220);

    meter.setBounds(bounds);
    controlPanel.reduce(15, 15);

    const int rowHeight = 30;
    const int gap = 24;

    auto topRow = controlPanel.removeFromTop(rowHeight);
    binSizeLabel.setBounds(topRow.removeFromLeft(60));
    binSizeMenu.setBounds(topRow);
    controlPanel.removeFromTop(gap);

    auto channelRow = controlPanel.removeFromTop(rowHeight);
    btnLeft.setBounds(channelRow.removeFromLeft(channelRow.getWidth() / 3));
    btnRight.setBounds(channelRow.removeFromLeft(channelRow.getWidth() / 2));
    btnBoth.setBounds(channelRow);
    controlPanel.removeFromTop(gap);

    auto scaleRow = controlPanel.removeFromTop(rowHeight);
    btnLog.setBounds(scaleRow.removeFromLeft(scaleRow.getWidth() / 3));
    btnLin.setBounds(scaleRow.removeFromLeft(scaleRow.getWidth() / 2));
    btnST.setBounds(scaleRow);
    controlPanel.removeFromTop(gap);

    auto displayRow = controlPanel.removeFromTop(rowHeight);
    btnMax.setBounds(displayRow.removeFromLeft(displayRow.getWidth() / 2));
    displayRow.removeFromLeft(4);
    btnLine.setBounds(displayRow);
    controlPanel.removeFromTop(gap);

    auto dbRow = controlPanel.removeFromTop(rowHeight);
    btnDbRange.setBounds(dbRow.removeFromLeft(60));
    dbRow.removeFromLeft(10);

    sliderDbRangeMin.setBounds(dbRow.removeFromLeft(dbRow.getWidth() / 2));
    sliderDbRangeMax.setBounds(dbRow);
}