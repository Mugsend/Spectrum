/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <complex>
#include <cmath>

std::complex<float> calculateComplexAverage(const float* audioData, int numSamples, float phaseDelta, float* currentPhase)
{   
    if (numSamples == 0) return { 0.0f, 0.0f };
   
    std::complex<float> sum(0.0f, 0.0f);

    for (int t = 0; t < numSamples; ++t)
    {   
        float sineValue = std::sin(juce::MathConstants<float>::pi * static_cast<float>(t) / static_cast<float>(numSamples));

        float windowMultiplier = sineValue * sineValue;

        float windowedSample = audioData[t] * windowMultiplier;

        std::complex<float> exponent(0.0f, -*currentPhase);

        sum += windowedSample * std::exp(exponent);

        *currentPhase += phaseDelta;

        if (*currentPhase >= juce::MathConstants<float>::twoPi) {
            *currentPhase -= juce::MathConstants<float>::twoPi;
        }
    }

    std::complex<float> average = sum / static_cast<float>(numSamples);
    
    return average;
}
//==============================================================================
SpectrumAudioProcessor::SpectrumAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SpectrumAudioProcessor::~SpectrumAudioProcessor()
{
}

//==============================================================================
const juce::String SpectrumAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectrumAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpectrumAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpectrumAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectrumAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpectrumAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpectrumAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpectrumAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpectrumAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpectrumAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    targetFrequencies.resize(numBins);
    currentPhases.resize(numBins, 0.0f);
    magnitudes = std::vector <std::atomic<float>>(numBins);
    for (int i = 0; i < numBins; ++i)
    {
        targetFrequencies[i] = 10.0f * (i + 1);
    }
}

void SpectrumAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectrumAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif
int c = 0;
void SpectrumAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    int numSamples = buffer.getNumSamples();

    auto* channelData = buffer.getReadPointer(0);

    for (int f = 0; f < numBins; ++f) {
        float phaseDelta = (juce::MathConstants<float>::twoPi * targetFrequencies[f]) / static_cast<float>(currentSampleRate);

        std::complex<float> average = calculateComplexAverage(channelData, numSamples, phaseDelta, &currentPhases[f]);

        magnitudes[f].store(std::abs(average));
    }
}

//==============================================================================
bool SpectrumAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpectrumAudioProcessor::createEditor()
{
    return new SpectrumAudioProcessorEditor (*this);
}

//==============================================================================
void SpectrumAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpectrumAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectrumAudioProcessor();
}
