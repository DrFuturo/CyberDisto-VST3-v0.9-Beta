/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
// Ajoutez cette ligne en haut du fichier, avec les autres inclusions
#include "PresetData.h"

// Déplacez cette déclaration d'enum avant la classe CyberDistoAudioProcessor
enum class DistortionType
{
    SoftClip,
    HardClip,
    Tube,
    Fuzz,
    Diode,       // New distortion type
    Tape,        // New distortion type
    LinFold,     // New distortion type
    SinFold,     // New distortion type
    ZeroSquare   // New distortion type
};

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

    // Add this method
    const std::vector<float>& getSpectrumData() const;

    // Add these methods
    void pushNextSampleIntoFifo(float sample) noexcept;
    float getNextFFTResult() noexcept;

    void updateSpectrum(const juce::AudioBuffer<float>& buffer);

    // Ajoutez ces méthodes pour les nouveaux paramètres
    void setDistortionType(DistortionType type);
    DistortionType getDistortionType() const;
    void setPreGain(float gain);
    float getPreGain() const;
    void setPostGain(float gain);
    float getPostGain() const;

    // Add these lines in the public section
    void setAttack(float attackTime);
    void setRelease(float releaseTime);

    void loadFactoryPresets();
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

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
    std::atomic<float>* lofiXParameter = nullptr;
    std::atomic<float>* lofiYParameter = nullptr;
    std::atomic<float>* lofiDryWetParameter = nullptr;

    // Replace the existing FFT-related members with these:
    std::unique_ptr<juce::dsp::FFT> fft;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    juce::HeapBlock<float> fifo;
    std::vector<float> fftData;
    std::vector<float> spectrumData;
    std::vector<float> oldSpectrumData; // Add this line
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    // Add this method:
    void initializeFFT();

    // Add DSP objects here (e.g., filter, compressor)
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowPassFilter;
    juce::dsp::Compressor<float> compressor;
    
    // Soft clipping function for distortion
    static float softClip(float sample) { return std::tanh(sample); }
    float applyExtraTerraEQ(float sample, float amount, int channel);
    float applyLoFiEffect(float sample, float x, float y, float dryWet, int channel);
    void createDefaultPresets();

    // Add these members for preset management
    juce::File presetDirectory;
    juce::String currentPresetName;

    void initializePresetDirectory();

    // Add these members
    void updateSpectrum();
    
    std::unique_ptr<juce::FileLogger> fileLogger;

    // Ajoutez ces membres pour les nouveaux paramètres
    std::atomic<int> distortionType{0};
    std::atomic<bool> testSignalEnabled{false};

    // Ajoutez ces pointeurs pour les nouveaux paramètres
    std::atomic<float>* distortionTypeParameter = nullptr;
    std::atomic<float>* preGainParameter = nullptr;
    std::atomic<float>* postGainParameter = nullptr;

    // Add these function declarations
    float applyDistortion(float input, float drive, DistortionType type);

    // Ajoutez ces variables membres pour l'effet Lo-Fi
    float lastSample = 0.0f;
    float sampleCounter = 0.0f;

    // Ajoutez cette fonction helper
    void processSample(float& sample, int channel);

    // Add this member
    juce::dsp::IIR::Filter<float> eqFilter;
    bool eqFilterInitialized = false;

    DistortionType currentDistortionType = DistortionType::SoftClip;

    // Add these lines in the private section
    std::atomic<float>* attackParameter = nullptr;
    std::atomic<float>* releaseParameter = nullptr;

    std::atomic<float>* distMixParameter = nullptr;

    // Add this member for the low-pass filter
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lofiLowPassFilter;

    // Add these members for stereo processing
    std::array<float, 2> lastSamples = {0.0f, 0.0f};
    std::array<float, 2> sampleCounters = {0.0f, 0.0f};
    std::array<juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>, 2> lofiLowPassFilters;

    std::array<juce::dsp::IIR::Filter<float>, 2> eqFilters;
    std::array<bool, 2> eqFiltersInitialized = {false, false};
};
