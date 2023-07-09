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
class Clipper
    /*������� �����, ��������������� ��� ������������ ����������
    ������ ���������, ������ ��� Hard, Soft, Sine, Triangle, Foldback.
    ����� ������ ����������� ������� process, ��������� ���������������
    ������������� �������.*/
{
public:
    virtual ~Clipper() { }
    virtual SampleType process(SampleType& sample) = 0;

    void updateMultiplier(SampleType newValue) { multiplier = newValue; }
protected:
    SampleType multiplier;
};
//==============================================================================
template <typename SampleType>
class HardClipper : public Clipper<SampleType>
{
public:
    SampleType process(SampleType& sample) override
    {
        return juce::jlimit<float>(-1.0f, 1.0f, sample * multiplier);
    }
};
//==============================================================================
template <typename SampleType>
class SoftClipper : public Clipper<SampleType>
{
public:
    SampleType process(SampleType& sample) override
    {
        auto updatedSample = [this](SampleType sample) -> double
            { return (std::atan(static_cast<double>(sample) * correctMultiplier())); };
        return static_cast<SampleType>(juce::jmap<double>(updatedSample(sample), updatedSample(-1.0), updatedSample(1.0), -1.0, 1.0));
    }
private:    
    double correctMultiplier() { return (correctionCoefficient * static_cast<double>(multiplier) - 0.75); } // �������� ��� multiplier = 1.0 ����� 0,5, ��������� ��������
    double correctionCoefficient{ 1.25 };
};
//==============================================================================
template <typename SampleType>
class FoldbackClipper : public Clipper<SampleType>
{
public:
    SampleType process(SampleType& sample) override
    {
        auto updatedSample = [this](SampleType sample) -> double
        { return (std::atan(static_cast<double>(sample) * correctMultiplier())); };
        if (std::abs(updatedSample(sample)) >= kneeThreshold)
        {
            foldbackMultiplier.reset(new double{ juce::jmap<double>(std::abs(updatedSample(sample)),
                                                                    kneeThreshold,
                                                                    1.0,
                                                                    1.0,
                                                                    //static_cast<double>(std::abs(sample)) * static_cast<double>(multiplier)) });
                                                                    static_cast<double>(std::abs(sample)) * correctMultiplier()) });
        }
        else foldbackMultiplier.reset(new double{ 1.0 });
        return static_cast<SampleType>(juce::jmap<double>(updatedSample(sample) / *foldbackMultiplier, updatedSample(-1.0), updatedSample(1.0), -1.0, 1.0));
    }
    double correctMultiplier()
    {
        double corrector{ static_cast<double>(JUCE_LIVE_CONSTANT(10)) / 10.0 }; // �������� 0.2 �� 0.7, ������ �� �����
        return corrector * static_cast<double>(multiplier) - (corrector - 0.5);
    }
private:
    double kneeThreshold{ 0.5 };
    std::shared_ptr<double> foldbackMultiplier;
};
//==============================================================================
class ControllerLayout
{
public:
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
    //HardClipper<float> clipper;
    FoldbackClipper<float> clipper;

private:
#if OSC
    juce::dsp::Oscillator<float> osc;
#endif // OSC
    juce::dsp::Gain<float> gain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionTestAudioProcessor)
};
