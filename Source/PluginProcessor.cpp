/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <complex>
#include <cmath>

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
void SpectrumAudioProcessor::pushNextSampleIntoFifo(float sample) noexcept
{
    int currentOrder = currentFftOrder.load();
    int currentFftSize = 1 << currentOrder;
    int currentHopSize = currentFftSize / 2;

    fifo[fifoIndex++] = sample;

    if (fifoIndex >= currentFftSize)
    {
        if (!nextFFTBlockReady.load())
        {
            std::fill(fftData.begin(), fftData.end(), 0.0f);
            std::copy(fifo.begin(), fifo.begin() + currentFftSize, fftData.begin());

            nextFFTBlockReady.store(true);
        }

        std::copy(fifo.begin() + currentHopSize, fifo.begin() + currentFftSize, fifo.begin());
        fifoIndex = currentFftSize - currentHopSize;
    }
}
void SpectrumAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    int numSamples = buffer.getNumSamples();


    int mode = currentChannelMode.load();
    int numChannels = buffer.getNumChannels();

    for (int t = 0; t < numSamples; ++t) {
        
        float mixedSample = 0.0f;
        
        if (mode == 0 && numChannels > 0) 
        {
            mixedSample = buffer.getReadPointer(0)[t];
        }
        
        else if (mode == 1 && numChannels > 1) 
        {
            mixedSample = buffer.getReadPointer(1)[t];
        }

        else if (mode == 2) 
        {
            if (numChannels > 1) 
            {
                mixedSample = (buffer.getReadPointer(0)[t] + buffer.getReadPointer(1)[t]) * 0.5f;
            }
            else if (numChannels > 0)
            {
                mixedSample = buffer.getReadPointer(0)[t];
            }
        }

        pushNextSampleIntoFifo(mixedSample);
 
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
