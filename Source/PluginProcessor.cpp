/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
template <typename Type, size_t size>
size_t Fifo<Type, size>::getSize() noexcept { return size; }

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
    auto readIndex = fifo.read(1);
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
    auto writeIndex = fifo.write(1);
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
        : AudioProcessor(BusesProperties()
    #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
                .withInput("Input", juce::AudioChannelSet::stereo(), true)
        #endif
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    #endif
        )
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
    fifo.prepare(samplesPerBlock, getNumInputChannels());
        juce::dsp::ProcessSpec spec;
        spec.maximumBlockSize = samplesPerBlock;
        spec.numChannels = getNumInputChannels();
        spec.sampleRate = sampleRate;
    #if OSC
        osc.initialise([](float x) { return std::sin(x); });
        osc.prepare(spec);
        osc.setFrequency(220.0f);
    #endif
        gain.prepare(spec);
        gain.setGainDecibels(-18.0f);
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
    #if OSC
        auto numSamples = buffer.getNumSamples();
        buffer.clear();
        for (int i = 0; i < numSamples; ++i)
        {
            auto sample = osc.processSample(0);
            auto sample2 = osc.processSample(0);
            buffer.setSample(0, i, sample);
            buffer.setSample(1, i, sample2);
        }
    #endif
    gain.setGainDecibels(controllerLayout.getGainLevelInDecibels());
    auto audioBlock{ juce::dsp::AudioBlock<float>(buffer) };
    auto gainContext{ juce::dsp::ProcessContextReplacing<float>(audioBlock) };
    gain.process(gainContext);
    fifo.push(buffer);

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
void ControllerLayout::setGainLevelInDecibels(const double& value) { gainLevelInDecibels = value; }


double ControllerLayout::getGainLevelInDecibels() const { return gainLevelInDecibels; }
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistortionTestAudioProcessor();
}
