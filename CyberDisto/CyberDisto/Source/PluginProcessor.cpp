/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_dsp/juce_dsp.h>

//==============================================================================
CyberDistoAudioProcessor::CyberDistoAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "Parameters", {
          std::make_unique<juce::AudioParameterFloat> ("drive", "Drive", 1.0f, 10.0f, 1.0f),
          std::make_unique<juce::AudioParameterFloat> ("mix", "Mix", 0.0f, 1.0f, 0.5f),
          std::make_unique<juce::AudioParameterFloat> ("filterFreq", "Filter Freq", 20.0f, 20000.0f, 1000.0f),
          std::make_unique<juce::AudioParameterFloat> ("filterQ", "Filter Q", 0.1f, 10.0f, 1.0f),
          std::make_unique<juce::AudioParameterFloat> ("threshold", "Threshold", -60.0f, 0.0f, -12.0f),
          std::make_unique<juce::AudioParameterFloat> ("ratio", "Ratio", 1.0f, 20.0f, 4.0f),
          std::make_unique<juce::AudioParameterBool> ("extraTerra", "Extra Terra EQ", false)
      })
{
    driveParameter = parameters.getRawParameterValue("drive");
    mixParameter = parameters.getRawParameterValue("mix");
    filterFreqParameter = parameters.getRawParameterValue("filterFreq");
    filterQParameter = parameters.getRawParameterValue("filterQ");
    thresholdParameter = parameters.getRawParameterValue("threshold");
    ratioParameter = parameters.getRawParameterValue("ratio");
    extraTerraParameter = parameters.getRawParameterValue("extraTerra");

    // Initialize DSP objects here
    initializePresetDirectory();
    createDefaultPresets();
}

CyberDistoAudioProcessor::~CyberDistoAudioProcessor()
{
}

//==============================================================================
const juce::String CyberDistoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CyberDistoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CyberDistoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CyberDistoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CyberDistoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CyberDistoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CyberDistoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CyberDistoAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String CyberDistoAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void CyberDistoAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void CyberDistoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    lowPassFilter.prepare(spec);
    compressor.prepare(spec);
}

void CyberDistoAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CyberDistoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void CyberDistoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);  // Ajoutez cette ligne ici
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get current parameter values
    float drive = driveParameter->load();
    float mix = mixParameter->load();
    float filterFreq = filterFreqParameter->load();
    float filterQ = filterQParameter->load();
    float threshold = thresholdParameter->load();
    float ratio = ratioParameter->load();
    bool extraTerra = extraTerraParameter->load() > 0.5f;

    // Update filter coefficients
    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), filterFreq, filterQ);

    // Update compressor settings
    compressor.setThreshold(threshold);
    compressor.setRatio(ratio);

    // Create an AudioBlock object from the input buffer
    juce::dsp::AudioBlock<float> block(buffer);

    // Create a ProcessContextReplacing with the AudioBlock
    juce::dsp::ProcessContextReplacing<float> context(block);

    // Process the entire buffer with the lowPassFilter
    lowPassFilter.process(context);

    // Implement distortion and compression here
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Apply distortion
            float input = channelData[sample];
            float distorted = softClip(input * drive);

            // Apply compression
            distorted = compressor.processSample(channel, distorted);

            // Apply Extra Terra EQ if enabled
            if (extraTerra)
            {
                distorted = applyExtraTerraEQ(distorted);
            }

            // Mix dry and wet signals
            channelData[sample] = input * (1 - mix) + distorted * mix;
        }
    }
}

float CyberDistoAudioProcessor::applyExtraTerraEQ(float sample)
{
    static juce::dsp::IIR::Filter<float> eqFilter;
    static bool coefficientsInitialized = false;

    if (!coefficientsInitialized)
    {
        auto eqCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), 1000.0f, 0.7f, 6.0f);
        eqFilter.coefficients = eqCoeffs;
        coefficientsInitialized = true;
    }
    
    return eqFilter.processSample(sample);
}

void CyberDistoAudioProcessor::createDefaultPresets()
{
    // Warm Distortion preset
    savePreset("Warm Distortion");
    
    // Heavy Crunch preset
    *driveParameter = 8.0f;
    *mixParameter = 0.8f;
    *filterFreqParameter = 2000.0f;
    *filterQParameter = 0.7f;
    *thresholdParameter = -24.0f;
    *ratioParameter = 4.0f;
    *extraTerraParameter = false;
    savePreset("Heavy Crunch");
    
    // Reset to default values
    *driveParameter = 1.0f;
    *mixParameter = 0.5f;
    *filterFreqParameter = 1000.0f;
    *filterQParameter = 1.0f;
    *thresholdParameter = -12.0f;
    *ratioParameter = 4.0f;
    *extraTerraParameter = false;
}

//==============================================================================
bool CyberDistoAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CyberDistoAudioProcessor::createEditor()
{
    return new CyberDistoAudioProcessorEditor (*this);
}

//==============================================================================
void CyberDistoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->setAttribute("PresetName", currentPresetName);
    copyXmlToBinary(*xml, destData);
}

void CyberDistoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(parameters.state.getType()))
        {
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
            currentPresetName = xmlState->getStringAttribute("PresetName", "");
        }
    }
}

void CyberDistoAudioProcessor::initializePresetDirectory()
{
    presetDirectory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(JucePlugin_Name).getChildFile("Presets");

    if (!presetDirectory.exists())
        presetDirectory.createDirectory();
}

void CyberDistoAudioProcessor::savePreset(const juce::String& name)
{
    if (name.isEmpty())
        return;

    juce::File presetFile = presetDirectory.getChildFile(name + ".xml");
    juce::MemoryBlock data;
    getStateInformation(data);

    if (presetFile.replaceWithData(data.getData(), data.getSize()))
    {
        currentPresetName = name;
    }
}

void CyberDistoAudioProcessor::loadPreset(const juce::String& name)
{
    juce::File presetFile = presetDirectory.getChildFile(name + ".xml");

    if (presetFile.existsAsFile())
    {
        juce::MemoryBlock data;
        if (presetFile.loadFileAsData(data))
        {
            setStateInformation(data.getData(), (int)data.getSize());
            currentPresetName = name;
        }
    }
}

void CyberDistoAudioProcessor::deletePreset(const juce::String& name)
{
    juce::File presetFile = presetDirectory.getChildFile(name + ".xml");

    if (presetFile.existsAsFile())
    {
        presetFile.deleteFile();
        if (currentPresetName == name)
            currentPresetName = "";
    }
}

juce::StringArray CyberDistoAudioProcessor::getPresetList()
{
    juce::StringArray presetList;
    juce::Array<juce::File> presetFiles;
    presetDirectory.findChildFiles(presetFiles, juce::File::findFiles, false, "*.xml");

    for (auto& file : presetFiles)
    {
        presetList.add(file.getFileNameWithoutExtension());
    }

    return presetList;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CyberDistoAudioProcessor();
}
