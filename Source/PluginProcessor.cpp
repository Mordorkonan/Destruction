/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
//==============================================================================
ClipHolder::ClipHolder()
{
    clippers.push_back(dynamic_cast<Clipper<float>*>(hardClipper.get()));
    clippers.push_back(dynamic_cast<Clipper<float>*>(softClipper.get()));
    clippers.push_back(dynamic_cast<Clipper<float>*>(foldbackClipper.get()));
    clippers.push_back(dynamic_cast<Clipper<float>*>(sineFoldClipper.get()));
    clippers.push_back(dynamic_cast<Clipper<float>*>(linearFoldClipper.get()));
    
    currentClipper = hard; // убрать, когда будет дерево параметров
}

void ClipHolder::setClipper(int newClipper) { currentClipper = newClipper; }

Clipper<float>* ClipHolder::getClipper() const { return clippers[currentClipper]; }
//==============================================================================
juce::File PresetManager::defaultDir{ juce::File::getSpecialLocation(
    juce::File::SpecialLocationType::commonDocumentsDirectory)
    .getChildFile(ProjectInfo::companyName)
    .getChildFile(ProjectInfo::projectName) };

const juce::String PresetManager::extention{ "prexet" };

PresetManager::PresetManager(APVTS& _apvts, juce::ValueTree& _defaultTree) : apvts(_apvts), defaultTree(_defaultTree)
{
    if (!defaultDir.exists())
    {
        const juce::Result result{ defaultDir.createDirectory() };
        if (result.failed())
        {
            DBG("Failed to create default directory");
            jassertfalse;
        }
    }
    apvts.state.addListener(this);
    currentPreset.referTo(apvts.state.getPropertyAsValue(juce::Identifier("presetName"), nullptr));
}

PresetManager::~PresetManager() { apvts.state.removeListener(this); }

void PresetManager::newPreset()
{
    apvts.replaceState(defaultTree.createCopy());
    currentPreset.setValue("-init-");
}

void PresetManager::savePreset(const juce::String& presetName)
{
    if (presetName.isEmpty()) { return; }
    currentPreset.setValue(presetName);
    const juce::File presetToSave{ defaultDir.getChildFile(presetName + "." + extention) };
    const auto xml{ apvts.state.createXml() };
    if (xml == nullptr)
    {
        DBG("Failed to create XML from current value tree");
        jassertfalse;
        return;
    }
    if (!xml->writeTo(presetToSave))
    {
        DBG("Failed to write XML value tree to preset file");
        jassertfalse;
        return;
    }
    updatePresetList();
}

void PresetManager::loadPreset(const juce::String& presetName)
{
    if (presetName.isEmpty()) { return; }
    const juce::File presetToLoad{ defaultDir.getChildFile(presetName + "." + extention) };
    if (!presetToLoad.exists())
    {
        DBG("Failed to load preset file");
        jassertfalse;
        return;
    }
    const auto xml{ juce::XmlDocument(presetToLoad).getDocumentElement() };
    if (xml == nullptr)
    {
        DBG("Failed to create XML value tree from loaded preset");
        jassertfalse;
        return;
    }
    apvts.replaceState(juce::ValueTree::fromXml(*xml));
    currentPreset.setValue(presetName);
}

void PresetManager::deletePreset(const juce::String& presetName)
{
    if (presetName.isEmpty()) { return; }
    const juce::File presetToDelete{ defaultDir.getChildFile(presetName + "." + extention) };
    if (currentPreset.toString() == "-init-") { return; }
    if (!presetToDelete.exists())
    {
        DBG("Preset is not saved to be deleted");
        jassertfalse;
        return;
    }
    if (!presetToDelete.deleteFile())
    {
        DBG("Failed to delete current preset file");
        jassertfalse;
        return;
    }
    updatePresetList();
    currentPreset.setValue("-init-");
    apvts.replaceState(defaultTree.createCopy());
}

void PresetManager::updatePresetList()
{
    presetList.clear(); // нужно чистить перед каждым добавлением элементов
    const auto fileList{ defaultDir.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false, "*." + extention)};
    for (const auto& file : fileList) { presetList.add(file.getFileNameWithoutExtension()); }
}

int PresetManager::nextPreset()
{
    if (presetList.isEmpty()) { return -1; }
    const int currentIndex{ presetList.indexOf(currentPreset.toString()) };
    int nextIndex{ currentIndex >= presetList.size() - 1 ? 0 : currentIndex + 1 };
    loadPreset(presetList.getReference(nextIndex));
    return nextIndex + presetListIdOffset; // смещение для обхода строк New, Load, Save, Delete в комбобоксе
}

int PresetManager::previousPreset()
{
    if (presetList.isEmpty()) { return -1; }
    const int currentIndex{ presetList.indexOf(currentPreset.toString()) };
    int prevIndex{ currentIndex <= 0 ? presetList.size() - 1 : currentIndex - 1 };
    loadPreset(presetList.getReference(prevIndex));
    return prevIndex + presetListIdOffset;
}

void PresetManager::valueTreeRedirected(juce::ValueTree& changedTree)
{
    currentPreset.referTo(changedTree.getPropertyAsValue(juce::Identifier("presetName"), nullptr));
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
        ),
    apvts(*this, nullptr, ProjectInfo::projectName, createParameterLayout())
#endif
{
    apvts.state.setProperty(juce::Identifier("presetName"), "-init-", nullptr);
    apvts.state.setProperty(juce::Identifier("version"), ProjectInfo::versionString, nullptr);
    defaultTree = apvts.copyState(); // сохранение дефолтного дерева для функции создания нового пресета
    manager = std::make_unique<PresetManager>(apvts, defaultTree);
    manager->updatePresetList();
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
        inputGain.prepare(spec);
        inputGain.setGainDecibels(static_cast<float>(gainController.getInputGainLevelInDb()));
        outputGain.prepare(spec);
        outputGain.setGainDecibels(static_cast<float>(gainController.getOutputGainLevelInDb()));
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
    // указываем условный порог магнитуды в 0,05 чтобы снизить нагрузку на процессор на холостом ходе
    if (!gainController.getBypassState() && buffer.getMagnitude(0, buffer.getNumSamples()) >= 0.00001)
    {
        // input gain
        auto audioBlock{ juce::dsp::AudioBlock<float>(buffer) };
        auto gainContext{ juce::dsp::ProcessContextReplacing<float>(audioBlock) };
        inputGain.setGainDecibels(static_cast<float>(gainController.getInputGainLevelInDb()));
        inputGain.process(gainContext);

        // clipping process
        auto numOfSamples = buffer.getNumSamples();
        auto numOfChannels = buffer.getNumChannels();
        float* sample = new float(0.0f);
        for (int i = 0; i < numOfChannels; ++i)
        {
            for (int j = 0; j < numOfSamples; ++j)
            {
                *sample = buffer.getSample(i, j);
                buffer.setSample(i, j, clipHolder.getClipper()->process(*sample));
            }
        }
        delete sample;
        sample = nullptr;

        // output gain
        outputGain.setGainDecibels(static_cast<float>(gainController.getOutputGainLevelInDb()));
        outputGain.process(gainContext);
    }
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
    // Метод 1 - Запись в стрим
    /*
    juce::MemoryOutputStream mos{ destData, false };
    if (apvts.state.isValid()) { apvts.state.writeToStream(mos); }
    */

    // Метод 2 - Запись xml в бинарник
    if (apvts.state.isValid())
    {
        const auto xml{ apvts.copyState().createXml() };
        copyXmlToBinary(*xml, destData);
    }
}

void DistortionTestAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Метод 1 - Чтение из стрима или напрямую из данных
    /*
    juce::ValueTree tempTree{ juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes)) };
    if (tempTree.isValid() && tempTree == apvts.state) { apvts.replaceState(tempTree); }
    else { jassertfalse; }
    */

    // Метод 2 - Чтение из бинарника в xml
    const auto xml{ getXmlFromBinary(data, sizeInBytes) };
    if (xml != nullptr) // проверка обязательна, иначе вылезает jassert
    {
        juce::ValueTree tempTree{ juce::ValueTree::fromXml(*xml) };
        if (tempTree.isValid()) { apvts.replaceState(tempTree); }
    }
    else { return; }
}
//==============================================================================
void GainController::setInputGainLevelInDb(const double& value) { inputGainInDb = value; }

double GainController::getInputGainLevelInDb() const { return inputGainInDb; }

void GainController::setOutputGainLevelInDb(const double& value) { outputGainInDb = value; }

double GainController::getOutputGainLevelInDb() const { return outputGainInDb; }

void GainController::setBypassState(const bool& newState) { bypassed = newState; }

bool GainController::getBypassState() const { return bypassed; }
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistortionTestAudioProcessor();
}

APVTS::ParameterLayout DistortionTestAudioProcessor::createParameterLayout()
{
    juce::StringArray clipTypes{ "Hard Clip", "Soft Clip", "Fold Back", "Sine Fold", "Linear Fold" };
    return APVTS::ParameterLayout
    {
        std::make_unique<juce::AudioParameterFloat>("Input Gain", "Input Gain", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterFloat>("Clip", "Clip", 1.0f, 10.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("Output Gain", "Output Gain", -12.0f, 12.0f, 0.0f),
        std::make_unique<juce::AudioParameterChoice>("Clipper Type", "Clipper Type", clipTypes, hard),
        std::make_unique<juce::AudioParameterBool>("Bypass", "Bypass", false),
        std::make_unique<juce::AudioParameterBool>("Link", "Link", true)
    };
}

PresetManager& DistortionTestAudioProcessor::getPresetManager() { return *manager; }