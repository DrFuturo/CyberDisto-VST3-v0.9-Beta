/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/**
*/
class CyberDistoAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    CyberDistoAudioProcessor();
    ~CyberDistoAudioProcessor() override;

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

    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

    // Add these methods for preset management
    void savePreset(const juce::String& name);
    void loadPreset(const juce::String& name);
    void deletePreset(const juce::String& name);
    juce::StringArray getPresetList();

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CyberDistoAudioProcessor)

    juce::AudioProcessorValueTreeState parameters;
    
    std::atomic<float>* driveParameter = nullptr;
    std::atomic<float>* mixParameter = nullptr;
    std::atomic<float>* filterFreqParameter = nullptr;
    std::atomic<float>* filterQParameter = nullptr;
    std::atomic<float>* thresholdParameter = nullptr;
    std::atomic<float>* ratioParameter = nullptr;
    std::atomic<float>* extraTerraParameter = nullptr;

    // Add these members for preset management
    juce::File presetDirectory;
    juce::String currentPresetName;

    void initializePresetDirectory();

    // Add DSP objects here (e.g., filter, compressor)
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowPassFilter;
    juce::dsp::Compressor<float> compressor;
    
    // Soft clipping function for distortion
    float softClip(float sample)
    {
        return std::tanh(sample);
    }

    float applyExtraTerraEQ(float sample);
    void createDefaultPresets();
};
