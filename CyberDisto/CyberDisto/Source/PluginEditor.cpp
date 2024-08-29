/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CyberpunkLookAndFeel.h"

CyberDistoAudioProcessorEditor::CyberDistoAudioProcessorEditor (CyberDistoAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&cyberpunkLookAndFeel);

    // Configure sliders
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& labelText) {
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.attachToComponent(&slider, false);
    };

    setupSlider(driveSlider, driveLabel, "Drive");
    setupSlider(mixSlider, mixLabel, "Mix");
    setupSlider(filterFreqSlider, filterFreqLabel, "Filter Freq");
    setupSlider(filterQSlider, filterQLabel, "Filter Q");
    setupSlider(thresholdSlider, thresholdLabel, "Threshold");
    setupSlider(ratioSlider, ratioLabel, "Ratio");

    // Configure Extra Terra button
    addAndMakeVisible(extraTerraButton);
    extraTerraButton.setButtonText("Extra Terra EQ");

    // Configure preset management components
    addAndMakeVisible(presetComboBox);
    presetComboBox.setTextWhenNothingSelected("Select Preset");
    updatePresetList();

    addAndMakeVisible(savePresetButton);
    savePresetButton.setButtonText("Save Preset");
    savePresetButton.onClick = [this] {
        auto dialogOptions = juce::AlertWindow::QuestionIcon;
        auto* window = new juce::AlertWindow("Save Preset", "Enter a name for your preset:", dialogOptions);
        
        window->addTextEditor("presetName", "My Preset");
        window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey, 0, 0));
        window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey, 0, 0));

        window->enterModalState(true, juce::ModalCallbackFunction::create(
            [this, window](int result) {
                if (result == 1)
                {
                    juce::String presetName = window->getTextEditorContents("presetName");
                    audioProcessor.savePreset(presetName);
                    updatePresetList();
                }
                delete window;
            }));
    };

    addAndMakeVisible(deletePresetButton);
    deletePresetButton.setButtonText("Delete Preset");
    deletePresetButton.onClick = [this] { deletePreset(); };

    // Create parameter attachments
    auto& params = audioProcessor.getParameters();
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "drive", driveSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "mix", mixSlider);
    filterFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "filterFreq", filterFreqSlider);
    filterQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "filterQ", filterQSlider);
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "threshold", thresholdSlider);
    ratioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(params, "ratio", ratioSlider);
    extraTerraAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(params, "extraTerra", extraTerraButton);

    presetComboBox.onChange = [this] { loadPreset(); };

    setResizable(true, true);
    setResizeLimits(400, 300, 800, 600);
    setSize (600, 400);
}

CyberDistoAudioProcessorEditor::~CyberDistoAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void CyberDistoAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(cyberpunkLookAndFeel.getBackgroundColor());

    // Draw cyberpunk-style grid
    g.setColour(cyberpunkLookAndFeel.getGridColor());
    for (float i = 0.0f; i < static_cast<float>(getWidth()); i += 20.0f) {
        g.drawLine(i, 0.0f, i, static_cast<float>(getHeight()), 1.0f);
    }
    for (float i = 0.0f; i < static_cast<float>(getHeight()); i += 20.0f) {
        g.drawLine(0.0f, i, static_cast<float>(getWidth()), i, 1.0f);
    }

    // Draw plugin name
    g.setFont(cyberpunkLookAndFeel.getTitleFont());
    g.setColour(cyberpunkLookAndFeel.getTitleColor());
    g.drawText("CyberDisto", getLocalBounds(), juce::Justification::centredTop, true);
}

void CyberDistoAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    auto topRow = bounds.removeFromTop(bounds.getHeight() / 2);
    auto bottomRow = bounds;

    int sliderSize = juce::jmin(topRow.getWidth() / 3, topRow.getHeight()) - 20;

    driveSlider.setBounds(topRow.removeFromLeft(sliderSize).reduced(10));
    mixSlider.setBounds(topRow.removeFromLeft(sliderSize).reduced(10));
    filterFreqSlider.setBounds(topRow.removeFromLeft(sliderSize).reduced(10));

    filterQSlider.setBounds(bottomRow.removeFromLeft(sliderSize).reduced(10));
    thresholdSlider.setBounds(bottomRow.removeFromLeft(sliderSize).reduced(10));
    ratioSlider.setBounds(bottomRow.removeFromLeft(sliderSize).reduced(10));

    auto presetArea = bottomRow.reduced(10);
    extraTerraButton.setBounds(presetArea.removeFromTop(30));
    presetComboBox.setBounds(presetArea.removeFromTop(30).withTrimmedRight(100));
    savePresetButton.setBounds(presetArea.removeFromTop(30).withTrimmedRight(100));
    deletePresetButton.setBounds(presetArea.removeFromTop(30).withTrimmedRight(100));
}

void CyberDistoAudioProcessorEditor::updatePresetList()
{
    presetComboBox.clear();
    auto presetList = audioProcessor.getPresetList();
    int index = 1;
    for (auto& presetName : presetList)
    {
        presetComboBox.addItem(presetName, index++);
    }
}

void CyberDistoAudioProcessorEditor::loadPreset()
{
    int selectedId = presetComboBox.getSelectedId();
    if (selectedId > 0)
    {
        juce::String presetName = presetComboBox.getItemText(selectedId - 1);
        audioProcessor.loadPreset(presetName);
    }
}

void CyberDistoAudioProcessorEditor::deletePreset()
{
    int selectedId = presetComboBox.getSelectedId();
    if (selectedId > 0)
    {
        juce::String presetName = presetComboBox.getItemText(selectedId - 1);
        audioProcessor.deletePreset(presetName);
        updatePresetList();
    }
}
