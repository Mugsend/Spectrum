#include "PluginProcessor.h"
#include "PluginEditor.h"

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

    btnLeft.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnLeft.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);

    btnLeft.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnLeft.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    btnRight.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnRight.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);
      
    btnRight.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnRight.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    btnBoth.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnBoth.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);

    btnBoth.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnBoth.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    btnLeft.onClick = [this]() { audioProcessor.currentChannelMode.store(0); };
    btnRight.onClick = [this]() { audioProcessor.currentChannelMode.store(1); };
    btnBoth.onClick = [this]() { audioProcessor.currentChannelMode.store(2); };

    btnMax.setClickingTogglesState(true);

    btnMax.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);    
    btnMax.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);
    
    btnMax.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    btnMax.setColour(juce::TextButton::textColourOnId, juce::Colours::black);

    btnMax.onClick = [this]
        {
            if (btnMax.getToggleState())
                meter.max.store(true);
            else
                meter.max.store(false);
        };
    
    btnLine.setClickingTogglesState(true);

    btnLine.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnLine.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);

    btnLine.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnLine.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

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
                btnLine.setButtonText("Path");
            }
                
        };

    btnLine.setToggleState(true, juce::dontSendNotification);

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

    btnLog.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnLog.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);

    btnLog.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnLog.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    btnLin.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnLin.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);

    btnLin.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnLin.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    btnST.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnST.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);

    btnST.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnST.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    btnLog.onClick = [this]() { meter.currentScaleMode.store(0); };
    btnLin.onClick = [this]() { meter.currentScaleMode.store(1); };
    btnST.onClick = [this]() { meter.currentScaleMode.store(2); };

    btnDbRange.setColour(juce::TextButton::buttonOnColourId, juce::Colours::limegreen);
    btnDbRange.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    btnDbRange.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    btnDbRange.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    btnDbRange.setClickingTogglesState(true);

    btnDbRange.setToggleState(true, juce::dontSendNotification);

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
                sliderDbRangeMin.setValue(static_cast<int>(meter.currentMinDb));
                sliderDbRangeMax.setValue(static_cast<int>(meter.currentMaxDb));
                meter.isDbRangeAuto.store(false);
                sliderDbRangeMin.setEnabled(true);
                sliderDbRangeMax.setEnabled(true);
            }
        };

    sliderDbRangeMin.setRange(meter.minDb, meter.maxDb, 1.0);
    sliderDbRangeMax.setRange(meter.minDb, meter.maxDb, 1.0);

    sliderDbRangeMin.setValue(meter.minDb, juce::dontSendNotification);
    sliderDbRangeMax.setValue(meter.maxDb, juce::dontSendNotification);

    sliderDbRangeMin.setNumDecimalPlacesToDisplay(0);
    sliderDbRangeMax.setNumDecimalPlacesToDisplay(0);

    sliderDbRangeMin.onValueChange = [this]
        {
            float newMin = static_cast<float>(sliderDbRangeMin.getValue());
            float currentMax = static_cast<float>(sliderDbRangeMax.getValue());

            if (currentMax - newMin < 60.0f)
            {
                float requiredMax = newMin + 60.0f;

                if (requiredMax <= meter.maxDb)
                {
                    sliderDbRangeMax.setValue(requiredMax, juce::dontSendNotification);
                    meter.currentMaxDb = requiredMax;
                    meter.currentMinDb = newMin;
                }
                else
                {
                    float blockedMin = meter.maxDb - 60.0f;
                    sliderDbRangeMin.setValue(blockedMin, juce::dontSendNotification);
                    meter.currentMinDb = blockedMin;
                    meter.currentMaxDb = meter.maxDb;
                }
            }
            else
            {
                meter.currentMinDb = newMin;
            }
        };

    sliderDbRangeMax.onValueChange = [this]
        {
            float newMax = static_cast<float>(sliderDbRangeMax.getValue());
            float currentMin = static_cast<float>(sliderDbRangeMin.getValue());

            if (newMax - currentMin < 60.0f)
            {
                float requiredMin = newMax - 60.0f;

                if (requiredMin >= meter.minDb)
                {
                    sliderDbRangeMin.setValue(requiredMin, juce::dontSendNotification);
                    meter.currentMinDb = requiredMin;
                    meter.currentMaxDb = newMax;
                }
                else
                {
                    float blockedMax = meter.minDb + 60.0f;
                    sliderDbRangeMax.setValue(blockedMax, juce::dontSendNotification);
                    meter.currentMaxDb = blockedMax;
                    meter.currentMinDb = meter.minDb;
                }
            }
            else
            {
                meter.currentMaxDb = newMax;
            }
        };

    sliderDbRangeMin.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    sliderDbRangeMax.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);

    sliderDbRangeMin.setEnabled(false);
    sliderDbRangeMax.setEnabled(false);

    addAndMakeVisible(meter);
    addAndMakeVisible(binSizeMenu);

    addAndMakeVisible(btnLeft);
    addAndMakeVisible(btnRight);
    addAndMakeVisible(btnBoth);

    addAndMakeVisible(btnMax);

    addAndMakeVisible(btnLine);

    addAndMakeVisible(btnLog);
    addAndMakeVisible(btnLin);
    addAndMakeVisible(btnST);

    addAndMakeVisible(btnDbRange);

    sliderDbRangeMin.setSliderStyle(juce::Slider::LinearHorizontal);
    sliderDbRangeMax.setSliderStyle(juce::Slider::LinearHorizontal);

    sliderDbRangeMin.setLookAndFeel(&customLook);
    sliderDbRangeMax.setLookAndFeel(&customLook);

    addAndMakeVisible(sliderDbRangeMin);
    addAndMakeVisible(sliderDbRangeMax);

    setSize(1200,300);
}

SpectrumAudioProcessorEditor::~SpectrumAudioProcessorEditor()
{
    sliderDbRangeMin.setLookAndFeel(nullptr);
    sliderDbRangeMax.setLookAndFeel(nullptr);
}

void SpectrumAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void SpectrumAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    binSizeMenu.setBounds(45,8,120,24);

    btnLeft.setBounds(45, 42, 35, 24);
    btnRight.setBounds(80, 42, 35, 24);
    btnBoth.setBounds(115, 42, 45, 24);

    btnMax.setBounds(45, 76, 45, 24);
    btnLine.setBounds(95, 76, 45, 24);

    btnLog.setBounds(45, 110, 35, 24);
    btnLin.setBounds(80, 110, 35, 24);
    btnST.setBounds(115, 110, 45, 24);

    btnDbRange.setBounds(45, 144, 45, 24);

    sliderDbRangeMin.setBounds(100, 144, 40, 15);
    sliderDbRangeMax.setBounds(150, 144, 40, 15);

    meter.setBounds(bounds);
}
