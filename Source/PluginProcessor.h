/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define OSC false
//========================================
// Clipper correction coefficients
#define HARDCLIP_COEF 0.25
#define SOFTCLIP_COEF 1.25
#define FOLDBACK_COEF 0.5
#define SINEFOLD_COEF 0.75
#define LINEARFOLD_COEF 0.75
// Sensitivities
#define SLOW_SENS 125
#define NORM_SENS 250
//========================================
enum ClipperType { hard = 1, soft, foldback, sinefold, linearfold };
//==============================================================================
template <typename Type, size_t size>
class Fifo
{
public:
    size_t getSize() noexcept { return size; }

    void prepare(int numSamples, int numChannels)
    {
        for (auto& buffer : buffers)
        {
            buffer.setSize(numChannels, numSamples, false, true, false);
            buffer.clear();
        }
    }

    int getNumAvailableForReading() const { return fifo.getNumReady(); }

    int getAvailableSpace() const { return fifo.getFreeSpace(); }

    bool pull(Type& t)
    {
        auto readIndex = fifo.read(1);
        if (readIndex.blockSize1 > 0)
        {
            t = buffers[readIndex.startIndex1];
            return true;
        }
        else { return false; }
    }

    bool push(const Type& t)
    {
        auto writeIndex = fifo.write(1);
        if (writeIndex.blockSize1 > 0)
        {
            buffers[writeIndex.startIndex1] = t;
            return true;
        }
        else { return false; }
    }

private:
    juce::AbstractFifo fifo{ size };
    std::array<Type, size> buffers;
};
//==============================================================================
template <typename SampleType>
class Clipper
    /* Базовый класс, предназначенный для модернизации различными
    типами клипперов, такими как Hard, Soft, SineFold, LinearFold, Foldback.
    Имеет чистую виртуальную функцию process, требующую переопределения
    наследованным классом. */
{
public:
    Clipper(double&& corrCoef = 1.0) : correctionCoefficient(corrCoef) { }
    virtual ~Clipper() { }
    virtual SampleType process(SampleType& sample) = 0;
    virtual void updateMultiplier(double newValue) { multiplier = correctionCoefficient * newValue - getOffset(); }
protected:
    virtual const double& getOffset() const { return correctionOffset; }

    double multiplier{ 1.0 };
    /* Корректирующие коэффициенты задают интенсивность влияния
    параметра multiplier на обработку. При этом значение вывода
    функции correctMulti() должно быть равно 0.5 при multiplier = 1,
    для соблюдения линейности участка передаточной функции.
    Исключение - HardClipper. */
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
        return static_cast<SampleType>(juce::jlimit<double>(-1.0, 1.0, static_cast<double>(sample) * multiplier));
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
    SoftClipper(double&& corrCoef = 1.0) : Clipper<SampleType>(std::move(corrCoef)) { }
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
    FoldbackClipper(double&& corrCoef = 1.0) : Clipper<SampleType>(std::move(corrCoef)) { }
    SampleType process(SampleType& sample) override
    {
        auto updatedSample = [this](SampleType sample) -> double
            { return (std::atan(static_cast<double>(sample) * multiplier)); };
        double newSample{ updatedSample(sample) };
        double foldbackMultiplier{ 1.0 };
        if (std::fabs(newSample) >= kneeThreshold)
        {
            foldbackMultiplier = juce::jmap<double>(std::fabs(newSample),
                                                    kneeThreshold,
                                                    1.0,
                                                    1.0,
                                                    std::fabs(static_cast<double>(sample)) * multiplier);
        }
        return static_cast<SampleType>(juce::jmap<double>(newSample / foldbackMultiplier,
                                                          updatedSample(-1.0),
                                                          updatedSample(1.0),
                                                          -1.0,
                                                          1.0));
    }
private:
    double kneeThreshold{ 0.5 }; // влияет на резкость звучания. Должен быть от 0,2 до 0,7 (найдено эмпирически)
};
//==============================================================================
template <typename SampleType>
class SineFoldClipper : public Clipper<SampleType>
{
public:
    SineFoldClipper(double&& corrCoef = 1.0) : Clipper<SampleType>(std::move(corrCoef)) { }
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
template <typename SampleType>
class LinearFoldClipper : public Clipper<SampleType>
{
public:
    LinearFoldClipper(double&& corrCoef = 1.0) : Clipper<SampleType>(std::move(corrCoef)) { }
    SampleType process(SampleType& sample) override
    {
        auto newSample{ static_cast<double>(sample) * multiplier };
        recursiveInversion(newSample);
        if (multiplier < 1) { newSample = juce::jmap<double>(newSample, -multiplier, multiplier, -1.0, 1.0); }
        return static_cast<SampleType>(newSample);
    }
private:
    void recursiveInversion(double& sample)
    {
        bool negativeSign{ false };
        if (std::abs(sample) >= 1)
        {
            if (sample < 0) { negativeSign = true; }
            else { negativeSign = false; }
            sample = (1.0 - (std::abs(sample) - 1)) * (negativeSign ? -1.0 : 1.0);
            recursiveInversion(sample);
        }
    }
};
//==============================================================================
class ClipHolder
{
public:
    ClipHolder();
    void setClipper(int newClipper);
    Clipper<float>* getClipper() const;
private:
    std::vector<Clipper<float>*> clippers;
    int currentClipper;
    std::shared_ptr<HardClipper<float>> hardClipper{ new HardClipper<float>(HARDCLIP_COEF) };
    std::shared_ptr<SoftClipper<float>> softClipper{ new SoftClipper<float>(SOFTCLIP_COEF) };
    std::shared_ptr<FoldbackClipper<float>> foldbackClipper{ new FoldbackClipper<float>(FOLDBACK_COEF) };
    std::shared_ptr<SineFoldClipper<float>> sineFoldClipper{ new SineFoldClipper<float>(SINEFOLD_COEF) };
    std::shared_ptr<LinearFoldClipper<float>> linearFoldClipper{ new LinearFoldClipper<float>(LINEARFOLD_COEF) };
};
//==============================================================================
class GainController
{
public:
    void setInputGainLevelInDb(const double& value);
    double getInputGainLevelInDb() const;

    void setOutputGainLevelInDb(const double& value);
    double getOutputGainLevelInDb() const;
private:
    double inputGainInDb{ 0.0 };
    double outputGainInDb{ 0.0 };
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
    GainController gainController;
    ClipHolder clipHolder;
private:
#if OSC
    juce::dsp::Oscillator<float> osc;
#endif // OSC
    juce::dsp::Gain<float> inputGain;
    juce::dsp::Gain<float> outputGain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionTestAudioProcessor)
};
