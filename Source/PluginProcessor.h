/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <atomic>
#include <JuceHeader.h>
#include <complex>

//==============================================================================
/**
*/
class SpectrumAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SpectrumAudioProcessor();
        ~SpectrumAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    static constexpr int maxFftSize = 16384;
    static constexpr int maxBins = maxFftSize / 2;
    std::array<std::atomic<float>, maxBins> magnitudes;
    std::atomic<int> activeNumBins{ 256 };
    std::atomic<int> currentFftSize{ 512 };
private:

    std::vector<std::complex<float>> complexBuffer = std::vector<std::complex<float>>(maxFftSize, { 0.0f, 0.0f });

    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAudioProcessor)
};
