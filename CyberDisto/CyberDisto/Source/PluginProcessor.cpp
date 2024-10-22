/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PresetData.h"
#include <JuceHeader.h>
#include <array>

//==============================================================================
CyberDistoAudioProcessor::CyberDistoAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "Parameters", createParameterLayout())
{
    juce::Logger::writeToLog("CyberDistoAudioProcessor constructor started");
    driveParameter = parameters.getRawParameterValue("drive");
    mixParameter = parameters.getRawParameterValue("mix");
    filterFreqParameter = parameters.getRawParameterValue("filterFreq");
    filterQParameter = parameters.getRawParameterValue("filterQ");
    thresholdParameter = parameters.getRawParameterValue("threshold");
    ratioParameter = parameters.getRawParameterValue("ratio");
    extraTerraParameter = parameters.getRawParameterValue("extraTerra");
    lofiXParameter = parameters.getRawParameterValue("lofiX");
    lofiYParameter = parameters.getRawParameterValue("lofiY");
    lofiDryWetParameter = parameters.getRawParameterValue("lofiDryWet");
    distortionTypeParameter = parameters.getRawParameterValue("distortionType");
    preGainParameter = parameters.getRawParameterValue("preGain");
    postGainParameter = parameters.getRawParameterValue("postGain");
    attackParameter = parameters.getRawParameterValue("attack");
    releaseParameter = parameters.getRawParameterValue("release");
    distMixParameter = parameters.getRawParameterValue("distMix"); // Added Dist Mix parameter

    // Initialize DSP objects here
    initializePresetDirectory();
    loadFactoryPresets();  // This line should now work
    initializeFFT();

    // Add this at the end of the constructor
    DBG("CyberDistoAudioProcessor initialized");
    juce::Logger::writeToLog("CyberDistoAudioProcessor initialized");
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
    spec.maximumBlockSize = juce::jmin(samplesPerBlock, 2048);
    spec.numChannels = getTotalNumOutputChannels();

    lowPassFilter.prepare(spec);
    compressor.prepare(spec);
    lofiLowPassFilters[0].prepare(spec);
    lofiLowPassFilters[1].prepare(spec);
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

void CyberDistoAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), filterFreqParameter->load(), filterQParameter->load());
    compressor.setThreshold(thresholdParameter->load());
    compressor.setRatio(ratioParameter->load());
    compressor.setAttack(attackParameter->load());
    compressor.setRelease(releaseParameter->load());

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    lowPassFilter.process(context);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            processSample(channelData[sample], channel);
        }
    }

    updateSpectrum(buffer);
}

void CyberDistoAudioProcessor::processSample(float& sample, int channel)
{
    const float drive = driveParameter->load();
    const float mix = mixParameter->load();
    const float extraTerra = extraTerraParameter->load();
    const float lofiX = lofiXParameter->load();
    const float lofiY = lofiYParameter->load();
    const float lofiDryWet = lofiDryWetParameter->load();
    const float currentPreGain = preGainParameter->load();
    const float currentPostGain = postGainParameter->load();
    const float distMix = distMixParameter->load();

    float input = sample * currentPreGain;
    
    // Apply distortion and mix
    float distorted = applyDistortion(input, drive, currentDistortionType);
    float distortionOutput = input * (1.0f - distMix) + distorted * distMix;
    
    // Apply post-gain after distortion
    distortionOutput *= currentPostGain;
    
    // Apply other effects
    float processed = distortionOutput;
    processed = compressor.processSample(channel, processed);
    processed = applyLoFiEffect(processed, lofiX, lofiY, lofiDryWet, channel);
    float extraTerraEQ = applyExtraTerraEQ(processed, extraTerra, channel);
    
    // Final mix
    sample = input * (1.0f - mix) + extraTerraEQ * mix;
}

float CyberDistoAudioProcessor::applyDistortion(float input, float drive, DistortionType type)
{
    switch (type)
    {
        case DistortionType::SoftClip:
            return softClip(input * drive);
        case DistortionType::HardClip:
            return juce::jlimit(-1.0f, 1.0f, input * drive);
        case DistortionType::Tube:
        {
            float x = input * drive * 2.0f;
            float y = x > 0.0f ? 1.0f - std::exp(-x) : -1.0f + std::exp(x);
            return y * 2.0f;
        }
        case DistortionType::Fuzz:
            return std::sin(input * drive * juce::MathConstants<float>::pi * 0.5f);
        case DistortionType::Diode:
        {
            float x = input * drive * 4.0f;
            float y = (x) / (1.0f + std::abs(x));
            y = y > 0.0f ? y * 2.0f : y * 0.3f;
            return y * 2.5f;
        }
        case DistortionType::Tape:
            return std::tanh(input * drive);
        case DistortionType::LinFold:
            return std::abs(std::fmod(input * drive + 1.0f, 4.0f) - 2.0f) - 1.0f;
        case DistortionType::SinFold:
        {
            float normalizedDrive = (drive - 1.0f) / 9.0f; // Normalize drive to 0-1 range
            float foldAmount = normalizedDrive * 0.99f + 0.01f; // Ensure we never reach exactly 1
            float phase = input * juce::MathConstants<float>::pi;
            return std::sin(phase / foldAmount) * foldAmount;
        }
        case DistortionType::ZeroSquare:
        {
            float normalizedDrive = (drive - 1.0f) / 9.0f; // Normalize drive to 0-1 range
            float effectMix = std::pow(normalizedDrive, 1.2f);

            float threshold = 0.2f * std::exp(-normalizedDrive * 2.5f);
            float softness = 0.02f + 0.1f * (1.0f - normalizedDrive);

            auto softClip = [](float x, float threshold, float softness) {
                if (std::abs(x) < 1e-6f) return 0.0f; // Return 0 for very small inputs
                if (x > threshold - softness) {
                    return threshold + softness * std::tanh((x - threshold) / softness);
                } else if (x < -threshold + softness) {
                    return -threshold - softness * std::tanh((-x - threshold) / softness);
                } else {
                    return x;
                }
            };

            float distortedOutput = softClip(input * (1.0f + normalizedDrive * 3.0f), threshold, softness);
            
            float intensity = 1.0f + normalizedDrive * 4.0f;
            return input * (1.0f - effectMix) + distortedOutput * effectMix * intensity;
        }
        default:
            return input;
    }
}

float CyberDistoAudioProcessor::applyExtraTerraEQ(float sample, float amount, int channel)
{
    if (!eqFiltersInitialized[channel])
    {
        auto eqCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), 1000.0f, 0.7f, 6.0f);
        eqFilters[channel].coefficients = eqCoeffs;
        eqFiltersInitialized[channel] = true;
    }
    
    float wetSample = eqFilters[channel].processSample(sample);
    return sample * (1.0f - amount) + wetSample * amount;
}

float CyberDistoAudioProcessor::applyLoFiEffect(float sample, float x, float y, float dryWet, int channel)
{
    float minFreq = 500.0f; // Increased minimum frequency to make the effect less pronounced
    float maxFreq = 44100.0f;
    float sampleRate = minFreq * std::pow(maxFreq / minFreq, static_cast<float>(x));
    float sampleRateReduction = static_cast<float>(getSampleRate()) / sampleRate;

    int bitDepth = 1 + static_cast<int>(y * 15);
    float bitDepthFloat = static_cast<float>((1 << bitDepth) - 1);

    float crushedSample = std::round(sample * bitDepthFloat) / bitDepthFloat;

    sampleCounters[channel] += 1.0f;
    if (sampleCounters[channel] >= sampleRateReduction)
    {
        lastSamples[channel] = crushedSample;
        sampleCounters[channel] -= sampleRateReduction;
    }

    float cutoffFrequency = juce::jmap(y, 500.0f, 20000.0f); // Increased minimum cutoff frequency
    *lofiLowPassFilters[channel].state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), cutoffFrequency, 0.5f); // Decreased Q factor

    float* sampleBuffer[] = { &lastSamples[channel] };
    juce::dsp::AudioBlock<float> block(sampleBuffer, 1, 1);
    juce::dsp::ProcessContextReplacing<float> context(block);
    lofiLowPassFilters[channel].process(context);

    return sample * (1.0f - dryWet) + lastSamples[channel] * dryWet;
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
    *extraTerraParameter = 0.5f;
    savePreset("Heavy Crunch");
    
    // Reset to default values
    *driveParameter = 1.0f;
    *mixParameter = 0.5f;
    *filterFreqParameter = 1000.0f;
    *filterQParameter = 1.0f;
    *thresholdParameter = -12.0f;
    *ratioParameter = 4.0f;
    *extraTerraParameter = 0.0f;
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
    
    // Add this line to save the current distortion type
    xml->setAttribute("DistortionType", static_cast<int>(currentDistortionType));
    
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
            
            // Add these lines to load the distortion type
            int savedDistortionType = xmlState->getIntAttribute("DistortionType", 0);
            currentDistortionType = static_cast<DistortionType>(savedDistortionType);
            setDistortionType(currentDistortionType);
        }
    }
}

void CyberDistoAudioProcessor::initializePresetDirectory()
{
    presetDirectory = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(JucePlugin_Name).getChildFile("Presets");

    if (!presetDirectory.exists())
    {
        juce::Result result = presetDirectory.createDirectory();
        if (result.wasOk())
        {
            juce::Logger::writeToLog("Created preset directory: " + presetDirectory.getFullPathName());
        }
        else
        {
            juce::Logger::writeToLog("Failed to create preset directory: " + presetDirectory.getFullPathName());
        }
    }
    else
    {
        juce::Logger::writeToLog("Preset directory already exists: " + presetDirectory.getFullPathName());
    }
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

void CyberDistoAudioProcessor::initializeFFT()
{
    fft = std::make_unique<juce::dsp::FFT>(10);
    window = std::make_unique<juce::dsp::WindowingFunction<float>>(static_cast<size_t>(1 << 10), juce::dsp::WindowingFunction<float>::hann);
    fifo.allocate(fft->getSize() * 2, true);
    fftData.resize(fft->getSize() * 2);
    spectrumData.resize(fft->getSize() / 2);
    oldSpectrumData.resize(fft->getSize() / 2);
}

void CyberDistoAudioProcessor::updateSpectrum(const juce::AudioBuffer<float>& buffer)
{
    if (fft == nullptr || window == nullptr) return;

    const auto* channelData = buffer.getReadPointer(0);
    const int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        pushNextSampleIntoFifo(channelData[i]);
    }

    if (nextFFTBlockReady)
    {
        window->multiplyWithWindowingTable(fftData.data(), fft->getSize());
        fft->performFrequencyOnlyForwardTransform(fftData.data());

        const int numBins = fft->getSize() / 2;
        for (int i = 0; i < numBins; ++i)
        {
            spectrumData[i] = juce::Decibels::gainToDecibels(fftData[i]);
        }

        nextFFTBlockReady = false;
    }
}

void CyberDistoAudioProcessor::pushNextSampleIntoFifo(float sample) noexcept
{
    if (fifoIndex == static_cast<int>(fft->getSize()))
    {
        if (!nextFFTBlockReady)
        {
            std::fill(fftData.begin(), fftData.end(), 0.0f);
            std::copy(fifo.getData(), fifo.getData() + fft->getSize(), fftData.begin());
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
    }
    fifo[fifoIndex++] = sample;
}

float CyberDistoAudioProcessor::getNextFFTResult() noexcept
{
    if (nextFFTBlockReady && !fftData.empty())
    {
        nextFFTBlockReady = false;
        return juce::jmap(juce::Decibels::gainToDecibels(fftData[0], -100.0f), -100.0f, 0.0f, 0.0f, 1.0f);
    }

    return 0.0f;
}

const std::vector<float>& CyberDistoAudioProcessor::getSpectrumData() const
{
    return spectrumData;
}

void CyberDistoAudioProcessor::setDistortionType(DistortionType type)
{
    currentDistortionType = type;
    if (distortionTypeParameter != nullptr)
    {
        *distortionTypeParameter = static_cast<float>(type);
    }
    // Add this line to update the parameter directly
    parameters.getParameter("distortionType")->setValueNotifyingHost(static_cast<float>(type) / 8.0f);
}

DistortionType CyberDistoAudioProcessor::getDistortionType() const
{
    return currentDistortionType;
}

void CyberDistoAudioProcessor::loadFactoryPresets()
{
    juce::Logger::writeToLog("Loading factory presets...");
    juce::Logger::writeToLog("Preset directory: " + presetDirectory.getFullPathName());

    juce::StringArray presetNames = PresetData::getPresetNames();
    
    for (const auto& presetName : presetNames)
    {
        const char* xmlData = PresetData::getPresetXml(presetName);
        
        if (xmlData != nullptr)
        {
            juce::String xmlString(xmlData);
            std::unique_ptr<juce::XmlElement> xmlState(juce::XmlDocument::parse(xmlString));
            
            if (xmlState != nullptr && xmlState->hasTagName("Parameters"))
            {
                juce::String presetFileName = presetName + ".xml";
                juce::File presetFile = presetDirectory.getChildFile(presetFileName);
                
                if (!presetFile.existsAsFile())
                {
                    if (xmlState->writeTo(presetFile))
                    {
                        juce::Logger::writeToLog("Created preset: " + presetFile.getFullPathName());
                    }
                    else
                    {
                        juce::Logger::writeToLog("Failed to create preset: " + presetFile.getFullPathName());
                    }
                }
                else
                {
                    juce::Logger::writeToLog("Preset already exists: " + presetFile.getFullPathName());
                }
            }
        }
        else
        {
            juce::Logger::writeToLog("XML data is null for preset: " + presetName);
        }
    }
}

// Add this function definition
juce::AudioProcessorValueTreeState::ParameterLayout CyberDistoAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 1.0f, 10.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("distMix", "Dist Mix", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filterFreq", "Filter Freq", 20.0f, 20000.0f, 1000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("filterQ", "Filter Q", 0.1f, 10.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", -60.0f, 0.0f, -12.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ratio", "Ratio", 1.0f, 20.0f, 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("extraTerra", "Extra Terra", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lofiX", "Lo-Fi X", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lofiY", "Lo-Fi Y", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("lofiDryWet", "Lo-Fi Dry/Wet", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("preGain", "Pre Gain", 0.0f, 2.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("postGain", "Post Gain", 0.0f, 2.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("attack", "Attack", 0.1f, 100.0f, 10.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", 10.0f, 1000.0f, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("distortionType", "Distortion Type", juce::StringArray{"Soft Clip", "Hard Clip", "Tube", "Fuzz", "Diode", "Tape", "Lin.Fold", "Sin.Fold", "Zero Square"}, 0));

    return { params.begin(), params.end() };
}
