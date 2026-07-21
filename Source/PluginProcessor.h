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
class SpectrumAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    SpectrumAudioProcessor();
    ~SpectrumAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static constexpr int maxFftOrder = 14;
    static constexpr int maxFftSize = 1 << maxFftOrder;

    std::atomic<bool> nextFFTBlockReady{ false };
    std::atomic<int> currentFftOrder{ 11 };

    std::array<float, maxFftSize> fifo;
    std::array<float, maxFftSize * 2> fftData;
    int fifoIndex = 0;

    std::atomic<int> currentChannelMode = { 2 };

    juce::dsp::FFT fft2048{ 11 };
    juce::dsp::FFT fft4096{ 12 };
    juce::dsp::FFT fft8192{ 13 };
    juce::dsp::FFT fft16384{ 14 };

    juce::dsp::WindowingFunction<float> window2048{ 2048, juce::dsp::WindowingFunction<float>::hann };
    juce::dsp::WindowingFunction<float> window4096{ 4096, juce::dsp::WindowingFunction<float>::hann };
    juce::dsp::WindowingFunction<float> window8192{ 8192, juce::dsp::WindowingFunction<float>::hann };
    juce::dsp::WindowingFunction<float> window16384{ 16384, juce::dsp::WindowingFunction<float>::hann };
    

    void pushNextSampleIntoFifo(float sample) noexcept;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAudioProcessor)
};
