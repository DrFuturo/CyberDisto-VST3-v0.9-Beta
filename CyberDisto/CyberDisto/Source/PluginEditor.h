/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "JUCE_hints.h"
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CyberpunkLookAndFeel.h"

class XYPad : public juce::Component,
              private juce::Timer
{
public:
    XYPad(CyberDistoAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts, const juce::String& xParamId, const juce::String& yParamId);
    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void updateFromSliders();
    void resized() override;

private:
    CyberDistoAudioProcessor& audioProcessor;
    juce::AudioProcessorValueTreeState& apvts;
    juce::String xParamId, yParamId;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> xAttachment, yAttachment;
    juce::Point<float> currentPosition;

    void updatePosition(juce::Point<float> newPosition);
    void timerCallback() override;

    juce::Path spectrumPath;
    std::vector<float> spectrumData;
    std::vector<float> smoothedSpectrumData;
    float smoothingFactor = 0.8f; // Adjust this value between 0 (no smoothing) and 1 (max smoothing)
};

class TooltipComponent : public juce::Component
{
public:
    TooltipComponent();
    void paint(juce::Graphics& g) override;
    void showTooltip(const juce::String& text, juce::Point<int> position);
    void hideTooltip();

private:
    juce::String tooltipText;
    bool isVisible = false;
};

class CyberDistoAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       public juce::Timer,
                                       private juce::Slider::Listener,
                                       private juce::Button::Listener,
                                       private juce::ComboBox::Listener
{
public:
    CyberDistoAudioProcessorEditor (CyberDistoAudioProcessor&);
    ~CyberDistoAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

    void mouseDown(const juce::MouseEvent& e) override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;

private:
    CyberDistoAudioProcessor& audioProcessor;
    CyberpunkLookAndFeel cyberpunkLookAndFeel;

    juce::Slider driveSlider, mixSlider, filterFreqSlider, filterQSlider, thresholdSlider, ratioSlider, extraTerraSlider;
    juce::Label driveLabel, mixLabel, filterFreqLabel, filterQLabel, thresholdLabel, ratioLabel, extraTerraLabel;

    juce::ComboBox presetComboBox;
    juce::ImageButton savePresetButton;  // Change this from TextButton to ImageButton
    juce::ImageButton deletePresetButton;  // Change from TextButton to ImageButton

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> extraTerraAttachment;

    juce::Slider lofiXSlider, lofiYSlider, lofiDryWetSlider;
    juce::Label lofiXLabel, lofiYLabel, lofiDryWetLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lofiXAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lofiYAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lofiDryWetAttachment;

    XYPad lofiXYPad;

    juce::ComboBox distortionTypeSelector;
    juce::TextButton prevDistortionButton{"<"};
    juce::TextButton nextDistortionButton{">"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> distortionTypeAttachment;

    juce::Rectangle<int> distortionArea;
    juce::Rectangle<int> filterArea;
    juce::Rectangle<int> compressorArea;
    juce::Rectangle<int> lofiArea;
    juce::Rectangle<int> presetArea;
    juce::Rectangle<int> extraTerraArea;
    juce::Rectangle<int> logoArea;
    juce::Rectangle<int> dryWetArea; // Add this line

    juce::Image logo; // Add this line
    juce::Image extraTerraLogo; // Add this line

    // Add these near the other Image declarations
    juce::Image originalBackgroundImage;
    juce::Image scaledBackgroundImage;

    void updatePresetList();
    void savePreset();
    void loadPreset();
    void deletePreset();

    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    void drawRoundedBlock(juce::Graphics& g, const juce::Rectangle<int>& bounds, const juce::String& title, const juce::Colour& colour);

    void updateDistortionType(int change);

    // Add the declaration of getConstrainedBounds
    juce::Rectangle<int> getConstrainedBounds(int width, int height) const;

    public:
        CyberDistoAudioProcessorEditor(const CyberDistoAudioProcessorEditor&) = delete;
        CyberDistoAudioProcessorEditor& operator=(const CyberDistoAudioProcessorEditor&) = delete;

    private:
        const char* leakID;  // Remove the second 'const'

        // Add these to the private section of CyberDistoAudioProcessorEditor
        juce::Slider preGainSlider, postGainSlider;
        juce::Label preGainLabel, postGainLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preGainAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> postGainAttachment;

        juce::Slider distMixSlider;
        juce::Label distMixLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distMixAttachment;

        // Add these lines in the private section
        juce::Slider attackSlider, releaseSlider;
        juce::Label attackLabel, releaseLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

        // Add these to the private section of CyberDistoAudioProcessorEditor
        juce::TextButton prevPresetButton{"<"};
        juce::TextButton nextPresetButton{">"};

        // Add these member variables
        const int buttonHeight = 25;
        const int padding = 5;

        // Add this method declaration
        void changePreset(int direction);

        juce::Image diskImage;

        juce::Image crossImage;

        juce::Rectangle<int> presetBarBounds;

        // Add this line in the private section of the CyberDistoAudioProcessorEditor class
        void createSliderAttachment(const juce::String& parameterID, juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment);

        // Add these to the private section of CyberDistoAudioProcessorEditor
        void showAboutDialog();
        void createAboutDialogOverlay();
        void removeAboutDialogOverlay();
        bool isAboutDialogVisible = false;
        std::unique_ptr<juce::Component> aboutDialogOverlay;
        juce::Rectangle<int> logoClickArea;

        // In the private section of the CyberDistoAudioProcessorEditor class, add:
        juce::Rectangle<int> extraTerraClickArea;

        // Add these near the top of the file, after other includes
        private:
            bool isTooltipVisible = false;
            juce::String tooltipText = "Apply on all effects modules except Filter.";
            juce::Rectangle<int> tooltipBounds;

            void showTooltip(const juce::MouseEvent& e);
            void hideTooltip();

        // Add this near the other private member variables
        private:
            bool isMixSliderHovered = false;

    private:
        TooltipComponent tooltipComponent;
};
