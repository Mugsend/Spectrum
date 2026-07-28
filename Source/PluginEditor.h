#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MagnitudeMeter.h"
#include "CustomLookAndFeel.h"

class SpectrumAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SpectrumAudioProcessorEditor(SpectrumAudioProcessor&);
    ~SpectrumAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void setupBinSizeMenu();
    void setupChannelButtons();
    void setupDisplayButtons();
    void setupScaleButtons();
    void setupDbRangeControls();

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAudioProcessorEditor)
};