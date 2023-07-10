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
    /*Базовый класс, предназначенный для модернизации различными
    типами клипперов, такими как Hard, Soft, Sine, Triangle, Foldback.
    Имеет чистую виртуальную функцию process, требующую переопределения
    наследованным классом.*/
{
public:
    Clipper(double&& corrCoef = 1.0) : correctionCoefficient(corrCoef) { }
    virtual ~Clipper() { }
    virtual SampleType process(SampleType& sample) = 0;
    virtual void updateMultiplier(double newValue) { multiplier = correctionCoefficient * newValue - getOffset(); }
protected:
    virtual const double& getOffset() const { return correctionOffset; }

    double multiplier;
    /*корректирующие коэффициенты задают интенсивность влияния
    параметра multiplier на обработку. При этом значение вывода
    функции correctMulti() должно быть равно 0.5 при multiplier = 1,
    для соблюдения линейности участка передаточной функции.*/    
    double correctionCoefficient;
    double correctionOffset{ correctionCoefficient - 0.5 };
};
//==============================================================================
template <typename SampleType>
class HardClipper : public Clipper<SampleType>
{
public:
    HardClipper(double&& corrCoef = 1.0) : Clipper<SampleType>(std::move(corrCoef)) { }
    SampleType process(SampleType& sample) override
    {
        return juce::jlimit<double>(-1.0, 1.0, static_cast<double>(sample) * multiplier);
    }
private:
    virtual const double& getOffset() const override { return correctionOffset; }

    double correctionOffset{ correctionCoefficient - 1.0 };
};
//==============================================================================
template <typename SampleType>
class SoftClipper : public Clipper<SampleType>
{
public:
    SoftClipper(double&& corrCoef = 1.0) : Clipper(std::move(corrCoef)) { }
    SampleType process(SampleType& sample) override
    {
        auto updatedSample = [this](SampleType sample) -> double
            { return (std::atan(static_cast<double>(sample) * multiplier)); };
        return static_cast<SampleType>(juce::jmap<double>(updatedSample(sample), updatedSample(-1.0), updatedSample(1.0), -1.0, 1.0));
    }
};
//==============================================================================
template <typename SampleType>
class FoldbackClipper : public Clipper<SampleType>
{
public:
    FoldbackClipper(double&& corrCoef = 1.0) : Clipper(std::move(corrCoef)) { }
    SampleType process(SampleType& sample) override
    {
        auto updatedSample = [this](SampleType sample) -> double
            { return (std::atan(static_cast<double>(sample) * multiplier)); };
        if (std::abs(updatedSample(sample)) >= kneeThreshold)
        {
            foldbackMultiplier.reset(new double{ juce::jmap<double>(std::abs(updatedSample(sample)),
                                                                    kneeThreshold,
                                                                    1.0,
                                                                    1.0,
                                                                    static_cast<double>(std::abs(sample)) * multiplier) });
        }
        else foldbackMultiplier.reset(new double{ 1.0 });
        return static_cast<SampleType>(juce::jmap<double>(updatedSample(sample) / *foldbackMultiplier,
                                                          updatedSample(-1.0),
                                                          updatedSample(1.0),
                                                          -1.0,
                                                          1.0));
    }
private:
    double kneeThreshold{ 0.5 }; // влияет на резкость звучания. Должен быть от 0,2 до 0,7 (найдено эмпирически)
    std::shared_ptr<double> foldbackMultiplier;
};
//==============================================================================
template <typename SampleType>
class SineClipper : public Clipper<SampleType>
{
public:
    SineClipper(double&& corrCoef = 1.0) : Clipper(std::move(corrCoef)) { }
    SampleType process(SampleType& sample) override
    {
        auto updateSample = [this](double sample) -> double
            { return std::sin(sample * multiplier * juce::MathConstants<double>::halfPi); };
        auto newSample{ updateSample(static_cast<double>(sample)) };
        if (multiplier < 1)
        {
            newSample = juce::jmap<double>(newSample,
                                           updateSample(-1.0),
                                           updateSample(1.0),
                                           -1.0,
                                           1.0);
        }
        return static_cast<SampleType>(newSample);
    }
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
    //HardClipper<float> clipper{ 0.25 };
    //SoftClipper<float> clipper{ 1.25 };
    //FoldbackClipper<float> clipper{ 0.5 };
    SineClipper<float> clipper{ 0.75 };

private:
#if OSC
    juce::dsp::Oscillator<float> osc;
#endif // OSC
    juce::dsp::Gain<float> gain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionTestAudioProcessor)
};
