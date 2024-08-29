/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CyberpunkLookAndFeel.h"

class CyberDistoAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    CyberDistoAudioProcessorEditor (CyberDistoAudioProcessor&);
    ~CyberDistoAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CyberDistoAudioProcessor& audioProcessor;
    CyberpunkLookAndFeel cyberpunkLookAndFeel;

    juce::Slider driveSlider, mixSlider, filterFreqSlider, filterQSlider, thresholdSlider, ratioSlider;
    juce::Label driveLabel, mixLabel, filterFreqLabel, filterQLabel, thresholdLabel, ratioLabel;
    juce::ToggleButton extraTerraButton;

    juce::ComboBox presetComboBox;
    juce::TextButton savePresetButton;
    juce::TextButton deletePresetButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> extraTerraAttachment;

    void updatePresetList();
    void savePreset();
    void loadPreset();
    void deletePreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CyberDistoAudioProcessorEditor)
};
