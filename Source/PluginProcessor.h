/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define OSC false
/**
*/
template <typename Type, size_t size>
class Fifo
{
public:
    size_t getSize() noexcept;
    void prepare(int numSamples, int numChannels);
    int getNumAvailableForReading() const;
    int getAvailableSpace() const;
    bool pull(Type& t);
    bool push(const Type& t);

private:
    juce::AbstractFifo fifo{ size };
    std::array<Type, size> buffers;
};
//==============================================================================
template <typename SampleType>
class HardClipper
{
public:
    SampleType processHardClipping(SampleType& sample)
    {
        return juce::jlimit<float>(-1.0f, 1.0f, sample * multiplier);
    }
    void updateMultiplier(SampleType newValue) { multiplier = newValue; }
private:
    SampleType multiplier;
};

//==============================================================================
class ControllerLayout
{
public:
    void processHardClipping();
    void setGainLevelInDecibels(const double& value);
    double getGainLevelInDecibels() const;
private:
    double gainLevelInDecibels{ -18.0 };
};
//==============================================================================
class DistortionTestAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    DistortionTestAudioProcessor();
    ~DistortionTestAudioProcessor() override;

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
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================

    Fifo<juce::AudioBuffer<float>, 256> fifo;
    ControllerLayout controllerLayout;
    HardClipper<float> hardClipper;

private:
#if OSC
    juce::dsp::Oscillator<float> osc;
#endif // OSC
    juce::dsp::Gain<float> gain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionTestAudioProcessor)
};
