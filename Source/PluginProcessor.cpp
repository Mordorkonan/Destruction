/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
template <typename Type, size_t size>
size_t Fifo<Type, size>::getSize() { return size; }

template <typename Type, size_t size>
void Fifo<Type, size>::prepare(int numSamples, int numChannels)
{
    for (auto& buffer : buffers)
    {
        buffer.setSize(numChannels, numSamples, false, true, false);
        buffer.clear();
    }    
}

template <typename Type, size_t size>
bool Fifo<Type, size>::pull(Type& t)
{
    auto readIndex = fifo.read();
    if (readIndex.blockSize1 > 0)
    {
        t = buffers[readIndex.startIndex1];
        return true;
    }
    else { return false; }
}

template <typename Type, size_t size>
bool Fifo<Type, size>::push(const Type& t)
{
    auto writeIndex = fifo.write();
    if (writeIndex.blockSize1 > 0)
    {
        buffers[writeIndex.startIndex1] = t;
        return true;
    }
    else { return false; }
}

template <typename Type, size_t size>
int Fifo<Type, size>::getNumAvailableForReading() const
{
    return fifo.getNumReady();
}

template <typename Type, size_t size>
int Fifo<Type, size>::getAvailableSpace() const
{
    return fifo.getFreeSpace();
}
//==============================================================================
DistortionTestAudioProcessor::DistortionTestAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties())
#endif
{
}

DistortionTestAudioProcessor::~DistortionTestAudioProcessor()
{
}

//==============================================================================
const juce::String DistortionTestAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DistortionTestAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DistortionTestAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DistortionTestAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DistortionTestAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DistortionTestAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DistortionTestAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DistortionTestAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DistortionTestAudioProcessor::getProgramName (int index)
{
    return {};
}

void DistortionTestAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DistortionTestAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void DistortionTestAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DistortionTestAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DistortionTestAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int i = 0; i < buffer.getNumChannels(); ++i)
    {
        
    }
}

//==============================================================================
bool DistortionTestAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DistortionTestAudioProcessor::createEditor()
{
    return new DistortionTestAudioProcessorEditor (*this);
}

//==============================================================================
void DistortionTestAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DistortionTestAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistortionTestAudioProcessor();
}
