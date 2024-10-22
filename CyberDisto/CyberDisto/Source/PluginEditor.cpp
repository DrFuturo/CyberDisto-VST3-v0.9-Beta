/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "CyberpunkLookAndFeel.h"
#include "BinaryData.h"
#include <memory>
#include <cmath>

static juce::Image loadImageFromBinaryData(const char* resourceName)
{
    int size;
    const char* data = BinaryData::getNamedResource(resourceName, size);
    if (data == nullptr || size == 0)
    {
        juce::Logger::writeToLog("Failed to load image: " + juce::String(resourceName));
        return juce::Image();
    }
    return juce::ImageCache::getFromMemory(data, size);
}

TooltipComponent::TooltipComponent()
{
    setAlwaysOnTop(true);
}

void TooltipComponent::paint(juce::Graphics& g)
{
    if (isVisible)
    {
        // Draw a solid background
        g.setColour(juce::Colours::black);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);

        // Draw a border
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 5.0f, 1.0f);

        // Draw the text
        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        g.drawFittedText(tooltipText, getLocalBounds().reduced(5), juce::Justification::centred, 2);
    }
}

void TooltipComponent::showTooltip(const juce::String& text, juce::Point<int> position)
{
    tooltipText = text;
    isVisible = true;
    
    const int padding = 10;
    juce::Font font(juce::FontOptions().withHeight(14.0f));  // Updated font creation
    
    int requiredWidth = font.getStringWidth(text) + padding * 2;
    int width = juce::jmax(200, requiredWidth);
    int height = 30;
    
    juce::Rectangle<int> parentBounds = getParentComponent()->getLocalBounds();
    
    position.x -= width / 2;
    position.y -= (height + 5);
    
    // Move the tooltip down by 20 pixels
    position.y += 40;
    
    position.x = juce::jlimit(0, parentBounds.getWidth() - width, position.x);
    position.y = juce::jlimit(0, parentBounds.getHeight() - height, position.y);
    
    setBounds(position.x, position.y, width, height);
    setVisible(true);
    toFront(true);
    repaint();
}

void TooltipComponent::hideTooltip()
{
    isVisible = false;
    setVisible(false);
    repaint();
}

CyberDistoAudioProcessorEditor::CyberDistoAudioProcessorEditor (CyberDistoAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      lofiXYPad(p, p.getParameters(), "lofiX", "lofiY")
{
    juce::Logger::writeToLog("CyberDistoAudioProcessorEditor constructor started");
    setLookAndFeel(&cyberpunkLookAndFeel);

    // Make the plugin non-resizable
    setResizable(false, false); // {{ change_1 }}

    // Ensure all sliders are initialized
    addAndMakeVisible(driveSlider);
    addAndMakeVisible(distMixSlider);
    addAndMakeVisible(mixSlider);
    addAndMakeVisible(filterFreqSlider);
    addAndMakeVisible(filterQSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);
    addAndMakeVisible(extraTerraSlider);
    addAndMakeVisible(lofiXSlider);
    addAndMakeVisible(lofiYSlider);
    addAndMakeVisible(lofiDryWetSlider);
    addAndMakeVisible(preGainSlider);
    addAndMakeVisible(postGainSlider);
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);

    // Configure sliders
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& labelText, bool isSmall = false) {
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.attachToComponent(&slider, false);
        
        // Use a consistent font size for all labels
        float labelFontSize = 12.0f;  // Adjust this value to match other labels in your plugin
        label.setFont(label.getFont().withHeight(labelFontSize));

        if (isSmall)
        {
            slider.setSize(30, 30);
        }
    };

    setupSlider(driveSlider, driveLabel, "Drive");
    setupSlider(distMixSlider, distMixLabel, "Dist Mix");
    setupSlider(mixSlider, mixLabel, "DRY/WET");
    setupSlider(filterFreqSlider, filterFreqLabel, "Filter Freq");
    setupSlider(filterQSlider, filterQLabel, "Filter Q");
    setupSlider(thresholdSlider, thresholdLabel, "Threshold");
    setupSlider(ratioSlider, ratioLabel, "Ratio");
    setupSlider(extraTerraSlider, extraTerraLabel, "Extra Terra EQ");

    setupSlider(lofiXSlider, lofiXLabel, "Lo-Fi X");
    lofiXSlider.setComponentID("lofiX");
    setupSlider(lofiYSlider, lofiYLabel, "Lo-Fi Y");
    lofiYSlider.setComponentID("lofiY");
    setupSlider(lofiDryWetSlider, lofiDryWetLabel, "Lo-Fi Dry/Wet");

    setupSlider(preGainSlider, preGainLabel, "Pre Gain");
    setupSlider(postGainSlider, postGainLabel, "Post Gain");

    setupSlider(attackSlider, attackLabel, "Attack");
    setupSlider(releaseSlider, releaseLabel, "Release");

    // Add listeners to the Lo-Fi sliders
    lofiXSlider.addListener(this);
    lofiYSlider.addListener(this);

    // Configure preset management components
    addAndMakeVisible(prevPresetButton);
    addAndMakeVisible(nextPresetButton);
    addAndMakeVisible(presetComboBox);
    addAndMakeVisible(savePresetButton);
    addAndMakeVisible(deletePresetButton);

    presetComboBox.setTextWhenNothingSelected("Select Preset");
    presetComboBox.setJustificationType(juce::Justification::centred);
    updatePresetList();

    savePresetButton.setButtonText(""); // Remove text
    deletePresetButton.setButtonText("Delete");

    prevPresetButton.onClick = [this] { changePreset(-1); };
    nextPresetButton.onClick = [this] { changePreset(1); };
    savePresetButton.onClick = [this] { savePreset(); };
    deletePresetButton.onClick = [this] { deletePreset(); };

    // Style the preset components
    juce::Colour presetBarColor = juce::Colour(0xFF1F262F); // New color: #1f262f
    juce::Colour textColor = juce::Colours::white;

    auto styleButton = [this, presetBarColor, textColor](juce::Button& button) {
        button.setColour(juce::TextButton::buttonColourId, presetBarColor);
        button.setColour(juce::TextButton::textColourOffId, textColor);
        button.setColour(juce::TextButton::buttonOnColourId, presetBarColor.brighter(0.1f)); // Slightly brighter when clicked
        button.setLookAndFeel(nullptr); // Remove custom look and feel for these buttons
    };

    styleButton(prevPresetButton);
    styleButton(nextPresetButton);
    styleButton(savePresetButton);
    styleButton(deletePresetButton);

    presetComboBox.setColour(juce::ComboBox::backgroundColourId, presetBarColor);
    presetComboBox.setColour(juce::ComboBox::textColourId, textColor);
    presetComboBox.setColour(juce::ComboBox::arrowColourId, textColor);
    presetComboBox.setColour(juce::ComboBox::outlineColourId, presetBarColor); // Remove outline
    presetComboBox.setLookAndFeel(nullptr); // Remove custom look and feel for the combo box

    // Ensure all attachments are created
    createSliderAttachment("drive", driveSlider, driveAttachment);
    createSliderAttachment("distMix", distMixSlider, distMixAttachment);
    createSliderAttachment("mix", mixSlider, mixAttachment);
    createSliderAttachment("filterFreq", filterFreqSlider, filterFreqAttachment);
    createSliderAttachment("filterQ", filterQSlider, filterQAttachment);
    createSliderAttachment("threshold", thresholdSlider, thresholdAttachment);
    createSliderAttachment("ratio", ratioSlider, ratioAttachment);
    createSliderAttachment("extraTerra", extraTerraSlider, extraTerraAttachment);
    createSliderAttachment("lofiX", lofiXSlider, lofiXAttachment);
    createSliderAttachment("lofiY", lofiYSlider, lofiYAttachment);
    createSliderAttachment("lofiDryWet", lofiDryWetSlider, lofiDryWetAttachment);
    createSliderAttachment("preGain", preGainSlider, preGainAttachment);
    createSliderAttachment("postGain", postGainSlider, postGainAttachment);
    createSliderAttachment("attack", attackSlider, attackAttachment);
    createSliderAttachment("release", releaseSlider, releaseAttachment);

    presetComboBox.onChange = [this] { loadPreset(); };

    addAndMakeVisible(lofiXYPad);

    // Configure distortion type selector and buttons
    addAndMakeVisible(distortionTypeSelector);
    addAndMakeVisible(prevDistortionButton);
    addAndMakeVisible(nextDistortionButton);

    distortionTypeSelector.addItem("Soft Clip", 1);
    distortionTypeSelector.addItem("Hard Clip", 2);
    distortionTypeSelector.addItem("Tube", 3);
    distortionTypeSelector.addItem("Fuzz", 4);
    distortionTypeSelector.addItem("Diode", 5);       // New distortion type
    distortionTypeSelector.addItem("Tape", 6);        // New distortion type
    distortionTypeSelector.addItem("Lin.Fold", 7);    // New distortion type
    distortionTypeSelector.addItem("Sin.Fold", 8);    // New distortion type
    distortionTypeSelector.addItem("Zero Square", 9); // New distortion type
    distortionTypeSelector.setJustificationType(juce::Justification::centred);
    distortionTypeSelector.addListener(this);

    prevDistortionButton.addListener(this);
    nextDistortionButton.addListener(this);

    distortionTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getParameters(), "distortionType", distortionTypeSelector);

    // Load the logo
    logo = loadImageFromBinaryData("logo_png");

    // Load the Extra Terra logo
    extraTerraLogo = loadImageFromBinaryData("extraterra_png");

    // Load the disk image
    diskImage = loadImageFromBinaryData("disk_png");
    
    // Create a scaled version of the image (90% of original size)
    juce::Image scaledImage = diskImage.rescaled(
        juce::roundToInt(diskImage.getWidth() * 0.9f),
        juce::roundToInt(diskImage.getHeight() * 0.9f),
        juce::Graphics::ResamplingQuality::highResamplingQuality
    );
    
    savePresetButton.setImages(true, true, true, 
                               scaledImage, 1.0f, juce::Colours::transparentBlack,
                               scaledImage, 1.0f, juce::Colours::white.withAlpha(0.5f),
                               scaledImage, 1.0f, juce::Colours::white);

    addAndMakeVisible(savePresetButton);
    savePresetButton.onClick = [this] { savePreset(); };

    // Load the cross image
    crossImage = loadImageFromBinaryData("cross_png");
    
    // Create a scaled version of the image (90% of original size)
    juce::Image scaledCrossImage = crossImage.rescaled(
        juce::roundToInt(crossImage.getWidth() * 0.9f),
        juce::roundToInt(crossImage.getHeight() * 0.9f),
        juce::Graphics::ResamplingQuality::highResamplingQuality
    );
    
    deletePresetButton.setImages(true, true, true, 
                                 scaledCrossImage, 1.0f, juce::Colours::transparentBlack,
                                 scaledCrossImage, 1.0f, juce::Colours::white.withAlpha(0.5f),
                                 scaledCrossImage, 1.0f, juce::Colours::white);

    addAndMakeVisible(deletePresetButton);
    deletePresetButton.onClick = [this] { deletePreset(); };

    // Style the distortion selector components
    juce::Colour selectorColor = juce::Colour(0xFF1F262F); // Very dark grey, almost black
    textColor = juce::Colours::white;

    auto styleSelectorButton = [this, selectorColor, textColor](juce::Button& button) {
        button.setColour(juce::TextButton::buttonColourId, selectorColor);
        button.setColour(juce::TextButton::textColourOffId, textColor);
        button.setLookAndFeel(&cyberpunkLookAndFeel);
    };

    styleSelectorButton(prevDistortionButton);
    styleSelectorButton(nextDistortionButton);

    distortionTypeSelector.setColour(juce::ComboBox::backgroundColourId, selectorColor);
    distortionTypeSelector.setColour(juce::ComboBox::textColourId, textColor);
    distortionTypeSelector.setColour(juce::ComboBox::arrowColourId, textColor);
    distortionTypeSelector.setLookAndFeel(&cyberpunkLookAndFeel);

    // Load the original background image
    originalBackgroundImage = loadImageFromBinaryData("bgcyberdisto_png");
    scaledBackgroundImage = originalBackgroundImage; // Initialize with original

    setSize(950, 576); // Set initial size to 950x524

    startTimerHz(30); // Increase update rate to 30 Hz

    juce::Colour buttonColor = juce::Colour(0xFF1F262F);

    prevPresetButton.setColour(juce::TextButton::buttonColourId, buttonColor);
    nextPresetButton.setColour(juce::TextButton::buttonColourId, buttonColor);

    // Add this line to initialize the distortion type selector
    distortionTypeSelector.setSelectedItemIndex(static_cast<int>(audioProcessor.getDistortionType()), juce::dontSendNotification);

    mixSlider.addMouseListener(this, true);
    mixSlider.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    addChildComponent(tooltipComponent);
    mixSlider.addMouseListener(this, false);
}

CyberDistoAudioProcessorEditor::~CyberDistoAudioProcessorEditor()
{
    // Remove all listeners before destroying attachments
    lofiXSlider.removeListener(this);
    lofiYSlider.removeListener(this);
    distortionTypeSelector.removeListener(this);
    prevDistortionButton.removeListener(this);
    nextDistortionButton.removeListener(this);

    // Clear all attachments
    driveAttachment.reset();
    distMixAttachment.reset();
    mixAttachment.reset();
    filterFreqAttachment.reset();
    filterQAttachment.reset();
    thresholdAttachment.reset();
    ratioAttachment.reset();
    extraTerraAttachment.reset();
    lofiXAttachment.reset();
    lofiYAttachment.reset();
    lofiDryWetAttachment.reset();
    preGainAttachment.reset();
    postGainAttachment.reset();
    attackAttachment.reset();
    releaseAttachment.reset();
    distortionTypeAttachment.reset();

    // Remove look and feel
    setLookAndFeel(nullptr);
}

//==============================================================================
void CyberDistoAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Draw the scaled background image
    if (scaledBackgroundImage.isValid())
    {
        g.drawImage(scaledBackgroundImage, getLocalBounds().toFloat(), juce::RectanglePlacement::stretchToFit);
    }
    else
    {
        // Fallback to the original background color if the image couldn't be loaded
        g.fillAll(cyberpunkLookAndFeel.getBackgroundColor());
    }

    // Set block color with 0 opacity (fully transparent)
    juce::Colour blockColor = juce::Colours::transparentBlack;

    // Draw rounded blocks for each module with 0 opacity
    drawRoundedBlock(g, logoArea, "", blockColor);
    drawRoundedBlock(g, distortionArea, "DISTORTION", blockColor);
    drawRoundedBlock(g, filterArea, "FILTER", blockColor);
    drawRoundedBlock(g, compressorArea, "COMPRESSOR", blockColor);
    drawRoundedBlock(g, lofiArea, "LO-FI", blockColor);
    drawRoundedBlock(g, extraTerraArea, "", blockColor);
    drawRoundedBlock(g, dryWetArea, "", blockColor);

    // Draw a wider bar behind the preset bar
    g.setColour(juce::Colour(0xFF1F262F)); // New color: #1f262f
    float barWidth = 320.0f;
    float barHeight = presetBarBounds.getHeight() + 10.0f; // Slightly taller than the preset bar
    float barX = (getWidth() - barWidth) / 2.0f; // Center horizontally
    float barY = presetBarBounds.getY() - 5.0f; // Slightly above the preset bar
    g.fillRoundedRectangle(barX, barY, barWidth, barHeight, 7.0f); // Slightly larger corner radius

    // Draw main logo
    if (logo.isValid())
    {
        auto logoRect = logoArea.reduced(10).removeFromLeft(juce::roundToInt(logoArea.getWidth() * 0.3f));
        [[maybe_unused]] float aspectRatio = logo.getWidth() / static_cast<float>(logo.getHeight());
        int logoHeight = juce::roundToInt(static_cast<float>(logoRect.getHeight()) * 0.8f * 0.6f * 0.4f * 1.5f * 1.65f * 1.6f * 1.15f);        
        // Align the logo to the left and center vertically
        int x = logoRect.getX();
        int y = juce::roundToInt(logoRect.getCentreY() - logoHeight / 2.0f);
        
        g.drawImage(logo, 
                    juce::roundToInt(x), 
                    juce::roundToInt(y), 
                    juce::roundToInt(logoRect.getWidth()), 
                    juce::roundToInt(logoHeight), 
                    0, 0, logo.getWidth(), logo.getHeight());
    }
    else
    {
        // Fallback if logo couldn't be loaded
        g.setColour(juce::Colours::white);
        g.setFont(24.0f * 0.6f * 0.6f * 1.5f * 1.65f * 1.6f * 1.15f);
        g.drawText("Logo", logoArea.reduced(10).removeFromLeft(juce::roundToInt(logoArea.getWidth() * 0.3f)), juce::Justification::centredLeft, true);
    }

    // Draw Extra Terra logo
    if (extraTerraLogo.isValid())
    {
        auto extraTerraRect = logoArea.reduced(10).removeFromRight(juce::roundToInt(logoArea.getWidth() * 0.3f));
        float aspectRatio = extraTerraLogo.getWidth() / static_cast<float>(extraTerraLogo.getHeight());
        int logoHeight = juce::roundToInt(static_cast<float>(extraTerraRect.getHeight()) * 0.8f * 0.45f * 0.55f * 1.5f * 1.65f * 1.6f * 0.8f);
        int logoWidth = juce::roundToInt(static_cast<float>(logoHeight) * aspectRatio);
        
        // Align the logo to the right and center vertically
        int x = extraTerraRect.getRight() - logoWidth;
        int y = juce::roundToInt(extraTerraRect.getCentreY() - logoHeight / 2.0f);
        
        g.drawImage(extraTerraLogo, 
                    juce::roundToInt(x), 
                    juce::roundToInt(y), 
                    juce::roundToInt(logoWidth), 
                    juce::roundToInt(logoHeight), 
                    0, 0, 
                    juce::roundToInt(extraTerraLogo.getWidth()), 
                    juce::roundToInt(extraTerraLogo.getHeight()));

        // Set the extraTerraClickArea
        extraTerraClickArea = juce::Rectangle<int>(x, y, logoWidth, logoHeight);
    }

    if (isAboutDialogVisible)
    {
        g.setColour(juce::Colours::black.withAlpha(0.8f));
        g.fillAll();
    }

    // Draw tooltip if visible
    if (isTooltipVisible)
    {
        g.setColour(juce::Colours::black.withAlpha(0.8f));
        g.fillRoundedRectangle(tooltipBounds.toFloat(), 5.0f);
        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        g.drawFittedText(tooltipText, tooltipBounds, juce::Justification::centred, 2);
    }
}

void CyberDistoAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    const int knobSize = 65; // Slightly reduced knob size
    const int localButtonHeight = 25;
    const int localPadding = 5;
    const int titleHeight = 25;
    const int extraPadding = 10;
    const int labelHeight = 15; // Height for labels

    // Logo area at the top (reduced by 35%)
    int originalLogoHeight = 100;
    int newLogoHeight = juce::roundToInt(originalLogoHeight * 0.65f);
    logoArea = bounds.removeFromTop(newLogoHeight).reduced(localPadding);

    // Calculate logo width (40% of total width)
    [[maybe_unused]] int logoWidth = static_cast<int>(std::round(logoArea.getWidth() * 0.4f));    
    // Position preset controls in the logo area
    auto presetControls = logoArea.reduced(localPadding);
    int presetControlWidth = juce::roundToInt(presetControls.getWidth() * 0.4f);
    int buttonWidth = juce::roundToInt(30.0f);

    // Reduce the preset bar width by 15%
    presetControlWidth = static_cast<int>(presetControlWidth * 0.85f);

    int comboBoxWidth = presetControlWidth - (buttonWidth * 4) - 20; // 20 for gaps
    
    // Center the preset bar
    presetBarBounds = presetControls.withSizeKeepingCentre(presetControlWidth, localButtonHeight);
    presetBarBounds.setCentre(logoArea.getCentreX(), logoArea.getCentreY());
    
    prevPresetButton.setBounds(presetBarBounds.removeFromLeft(buttonWidth));
    presetBarBounds.removeFromLeft(5);
    presetComboBox.setBounds(presetBarBounds.removeFromLeft(comboBoxWidth));
    presetBarBounds.removeFromLeft(5);
    nextPresetButton.setBounds(presetBarBounds.removeFromLeft(buttonWidth));
    presetBarBounds.removeFromLeft(5);

    // Make the save button 10% smaller
    int saveButtonSize = juce::roundToInt(buttonWidth * 0.8f);
    int saveButtonOffset = (buttonWidth - saveButtonSize) / 2;
    savePresetButton.setBounds(presetBarBounds.removeFromLeft(buttonWidth).reduced(saveButtonOffset));

    presetBarBounds.removeFromLeft(5);

    // Make the delete button 10% smaller
    int deleteButtonSize = juce::roundToInt(buttonWidth * 0.8f);
    int deleteButtonOffset = (buttonWidth - deleteButtonSize) / 2;
    deletePresetButton.setBounds(presetBarBounds.removeFromLeft(buttonWidth).reduced(deleteButtonOffset));

    // Add the height difference to the main content area
    int heightDifference = originalLogoHeight - newLogoHeight;
    bounds.setHeight(bounds.getHeight() + heightDifference);

    // Divide the main area into three vertical sections
    auto leftSection = bounds.removeFromLeft(bounds.getWidth() / 3);
    auto rightSection = bounds.removeFromRight(bounds.getWidth() / 2);
    auto centerSection = bounds;

    // 1. Distortion section (left top)
    distortionArea = leftSection.removeFromTop(juce::roundToInt(leftSection.getHeight() * 0.65f)).reduced(localPadding);
    auto distortionControls = distortionArea.reduced(localPadding).withTrimmedTop(titleHeight + extraPadding);
    
    auto gainControlsArea = distortionControls.removeFromTop(knobSize * 2 + labelHeight * 2 + localPadding * 2);
    auto knobWidth = gainControlsArea.getWidth() / 2;
    
    // First row: Pre Gain and Drive
    auto firstRowArea = gainControlsArea.removeFromTop(knobSize + labelHeight + localPadding);
    preGainSlider.setBounds(firstRowArea.removeFromLeft(knobWidth).withSizeKeepingCentre(knobSize, knobSize));
    driveSlider.setBounds(firstRowArea.withSizeKeepingCentre(knobSize, knobSize));
    
    // Second row: Post Gain and Dist Mix
    auto secondRowArea = gainControlsArea.removeFromTop(knobSize + labelHeight + localPadding);
    postGainSlider.setBounds(secondRowArea.removeFromLeft(knobWidth).withSizeKeepingCentre(knobSize, knobSize));
    distMixSlider.setBounds(secondRowArea.withSizeKeepingCentre(knobSize, knobSize));

    // Position labels under the knobs
    auto positionDistortionKnobLabel = [this, labelHeight, localPadding](juce::Slider& slider, juce::Label& label) {
        label.setBounds(slider.getX(), slider.getBottom() + localPadding, slider.getWidth(), labelHeight);
    };

    positionDistortionKnobLabel(preGainSlider, preGainLabel);
    positionDistortionKnobLabel(driveSlider, driveLabel);
    positionDistortionKnobLabel(postGainSlider, postGainLabel);
    positionDistortionKnobLabel(distMixSlider, distMixLabel);

    // Add padding before positioning the distortion type selector
    distortionControls.removeFromBottom(juce::roundToInt(localPadding * 10.5f));; // Adjust this value as needed

    // Set a specific width for the distortion selector
    const int distortionSelectorWidth = 200; // Adjust this value as needed
    const int arrowButtonWidth = 25; // Width for arrow buttons

    // Position the distortion type selector and buttons
    auto distortionSelectorBounds = distortionControls.removeFromBottom(localButtonHeight);

    // Calculate the center position for the selector
    int centerX = distortionSelectorBounds.getCentreX();
    int totalWidth = distortionSelectorWidth + (2 * arrowButtonWidth);

    // Position the prev button
    prevDistortionButton.setBounds(centerX - totalWidth / 2, distortionSelectorBounds.getY(),
                                   arrowButtonWidth, distortionSelectorBounds.getHeight());

    // Position the selector
    distortionTypeSelector.setBounds(centerX - distortionSelectorWidth / 2, distortionSelectorBounds.getY(),
                                     distortionSelectorWidth, distortionSelectorBounds.getHeight());

    // Position the next button
    nextDistortionButton.setBounds(centerX + distortionSelectorWidth / 2, distortionSelectorBounds.getY(),
                                   arrowButtonWidth, distortionSelectorBounds.getHeight());

    // 2. Filter section (left bottom)
    filterArea = leftSection.reduced(localPadding);
    auto filterControls = filterArea.reduced(localPadding).withTrimmedTop(titleHeight + extraPadding);
    
    // Move only the filter knobs up by 5 pixels
    filterFreqSlider.setBounds(filterControls.removeFromLeft(filterControls.getWidth() / 2).reduced(localPadding).withSizeKeepingCentre(knobSize, knobSize).translated(0, -5));
    filterQSlider.setBounds(filterControls.reduced(localPadding).withSizeKeepingCentre(knobSize, knobSize).translated(0, -5));

    // Position labels under the knobs, also moved up by 5 pixels
    auto filterLabelY = filterFreqSlider.getBottom() + localPadding - 5;
    filterFreqLabel.setBounds(filterFreqSlider.getX(), filterLabelY, filterFreqSlider.getWidth(), labelHeight);
    filterQLabel.setBounds(filterQSlider.getX(), filterLabelY, filterQSlider.getWidth(), labelHeight);

    // 3. Compressor section (right top)
    compressorArea = rightSection.removeFromTop(juce::roundToInt(rightSection.getHeight() * 0.55f)).reduced(localPadding); // Increased height slightly
    auto compressorControls = compressorArea.reduced(localPadding).withTrimmedTop(titleHeight + extraPadding);
    auto compressorKnobArea = compressorControls.removeFromTop(knobSize + labelHeight + localPadding);
    thresholdSlider.setBounds(compressorKnobArea.removeFromLeft(compressorKnobArea.getWidth() / 2).withSizeKeepingCentre(knobSize, knobSize));
    
    // Change this line: replace ratioSlider with attackSlider
    attackSlider.setBounds(compressorKnobArea.withSizeKeepingCentre(knobSize, knobSize));

    // Add a new row for Ratio and Release knobs
    auto ratioReleaseArea = compressorControls.removeFromTop(knobSize + labelHeight + localPadding * 3); // Increased padding
    
    // Change this line: replace attackSlider with ratioSlider
    ratioSlider.setBounds(ratioReleaseArea.removeFromLeft(ratioReleaseArea.getWidth() / 2).withSizeKeepingCentre(knobSize, knobSize));
    releaseSlider.setBounds(ratioReleaseArea.withSizeKeepingCentre(knobSize, knobSize));

    // Add extra space at the bottom of the compressor section
    compressorControls.removeFromBottom(localPadding * 4); // Adjust this value as needed

    // 4. Lo-Fi section (center)
    lofiArea = centerSection.reduced(localPadding);
    auto lofiControls = lofiArea.reduced(localPadding).withTrimmedTop(titleHeight + extraPadding);
    auto xyPadArea = lofiControls.removeFromTop(lofiControls.getHeight() - knobSize - labelHeight - localPadding * 6);
    auto lofiControlsArea = lofiControls;
    
    lofiXYPad.setBounds(xyPadArea);
    
    lofiXSlider.setBounds(lofiControlsArea.removeFromLeft(lofiControlsArea.getWidth() / 3).reduced(localPadding).withSizeKeepingCentre(knobSize, knobSize));
    lofiYSlider.setBounds(lofiControlsArea.removeFromLeft(lofiControlsArea.getWidth() / 2).reduced(localPadding).withSizeKeepingCentre(knobSize, knobSize));
    lofiDryWetSlider.setBounds(lofiControlsArea.reduced(localPadding).withSizeKeepingCentre(knobSize, knobSize));

    // Calculate the bottom of the Lo-Fi section
    int bottomAlignment = lofiArea.getBottom();

    // Adjust the Lo-Fi section
    lofiArea = centerSection.withBottom(bottomAlignment + localPadding * -7).reduced(localPadding);
    lofiControls = lofiArea.reduced(localPadding).withTrimmedTop(titleHeight + extraPadding);
    xyPadArea = lofiControls.removeFromTop(lofiControls.getHeight() - knobSize - labelHeight - localPadding * 6);
    lofiControlsArea = lofiControls;
    
    lofiXYPad.setBounds(xyPadArea);
    
    lofiXSlider.setBounds(lofiControlsArea.removeFromLeft(lofiControlsArea.getWidth() / 3).reduced(localPadding).withSizeKeepingCentre(knobSize, knobSize));
    lofiYSlider.setBounds(lofiControlsArea.removeFromLeft(lofiControlsArea.getWidth() / 2).reduced(localPadding).withSizeKeepingCentre(knobSize, knobSize));
    lofiDryWetSlider.setBounds(lofiControlsArea.reduced(localPadding).withSizeKeepingCentre(knobSize, knobSize));

    // Adjust the Filter section
    filterArea = leftSection.withBottom(bottomAlignment + localPadding * -9).reduced(localPadding);
    filterControls = filterArea.reduced(localPadding).withTrimmedTop(titleHeight + extraPadding);
    
    // Move only the filter knobs up by 5 pixels
    filterFreqSlider.setBounds(filterControls.removeFromLeft(filterControls.getWidth() / 2).reduced(localPadding).withSizeKeepingCentre(knobSize, knobSize).translated(0, -5));
    filterQSlider.setBounds(filterControls.reduced(localPadding).withSizeKeepingCentre(knobSize, knobSize).translated(0, -5));

    // Position labels under the knobs, also moved up by 5 pixels
    filterLabelY = filterFreqSlider.getBottom() + localPadding;
    filterFreqLabel.setBounds(filterFreqSlider.getX(), filterLabelY, filterFreqSlider.getWidth(), labelHeight);
    filterQLabel.setBounds(filterQSlider.getX(), filterLabelY, filterQSlider.getWidth(), labelHeight);

    // Adjust the right bottom section (Extra Terra and DRY/WET)
    auto rightBottomSection = rightSection.withBottom(bottomAlignment + localPadding * -1);
    
    // Calculate the width for each section, including padding
    int sectionWidth = (rightBottomSection.getWidth() - localPadding) / 2;
    
    // Extra Terra section
    extraTerraArea = rightBottomSection.removeFromLeft(sectionWidth).reduced(localPadding);
    auto extraTerraControls = extraTerraArea.reduced(localPadding);
    extraTerraSlider.setBounds(extraTerraControls.withSizeKeepingCentre(knobSize, knobSize));
    
    // DRY/WET section
    dryWetArea = rightBottomSection.reduced(localPadding);
    auto dryWetControls = dryWetArea.reduced(localPadding);
    mixSlider.setBounds(dryWetControls.withSizeKeepingCentre(knobSize, knobSize));

    // Position labels under the knobs
    auto positionLabel = [labelHeight, localPadding](juce::Slider& slider, juce::Label& label) {
        auto bounds = slider.getBounds();
        label.setBounds(bounds.getX(), bounds.getBottom() + localPadding, bounds.getWidth(), labelHeight);
    };

    // Apply to Extra Terra and DRY/WET sliders
    positionLabel(extraTerraSlider, extraTerraLabel);
    positionLabel(mixSlider, mixLabel);

    // Position labels under the knobs
    positionLabel(driveSlider, driveLabel);
    positionLabel(distMixSlider, distMixLabel);
    positionLabel(thresholdSlider, thresholdLabel);
    positionLabel(attackSlider, attackLabel);
    positionLabel(ratioSlider, ratioLabel);
    positionLabel(lofiXSlider, lofiXLabel);
    positionLabel(lofiYSlider, lofiYLabel);
    positionLabel(lofiDryWetSlider, lofiDryWetLabel);
    positionLabel(preGainSlider, preGainLabel);
    positionLabel(postGainSlider, postGainLabel);
    positionLabel(releaseSlider, releaseLabel);

    // Center the labels
    auto centerLabel = [](juce::Label& label, const juce::Rectangle<int>& area) {
        label.setBounds(area.withSizeKeepingCentre(label.getWidth(), label.getHeight()));
    };

    centerLabel(extraTerraLabel, extraTerraArea.removeFromBottom(labelHeight + localPadding));
    centerLabel(mixLabel, dryWetArea.removeFromBottom(labelHeight + localPadding));

    // Adjust label positions for Extra Terra and DRY/WET
    positionLabel(extraTerraSlider, extraTerraLabel);
    positionLabel(mixSlider, mixLabel);

    // Move Dist Mix knob and label down by 2 pixels
    auto distMixBounds = distMixSlider.getBounds();
    distMixSlider.setBounds(distMixBounds.withY(distMixBounds.getY() + 5));
    distMixLabel.setBounds(distMixLabel.getBounds().withY(distMixSlider.getBottom() + 2));

    // Move Post Gain knob and label down by 2 pixels
    auto postGainBounds = postGainSlider.getBounds();
    postGainSlider.setBounds(postGainBounds.withY(postGainBounds.getY() + 5));
    postGainLabel.setBounds(postGainLabel.getBounds().withY(postGainSlider.getBottom() + 2));

    // Only rescale if the size has changed
    if (originalBackgroundImage.isValid())
    {
        if (scaledBackgroundImage.getWidth() != getWidth() || scaledBackgroundImage.getHeight() != getHeight())
        {
            scaledBackgroundImage = originalBackgroundImage.rescaled(getWidth(), getHeight(), juce::Graphics::highResamplingQuality); // {{ change_2 }}
        }
    }

    // Set the logo click area
    logoClickArea = logoArea.reduced(10).removeFromLeft(juce::roundToInt(logoArea.getWidth() * 0.3f));

    // In the constructor of CyberDistoAudioProcessorEditor, add this after setting the logoClickArea
    extraTerraClickArea = logoArea.reduced(10).removeFromRight(juce::roundToInt(logoArea.getWidth() * 0.3f));
}

void CyberDistoAudioProcessorEditor::updatePresetList()
{
    auto currentPreset = presetComboBox.getText();
    presetComboBox.clear(juce::dontSendNotification);
    auto presetList = audioProcessor.getPresetList();
    presetComboBox.addItemList(presetList, 1);
    
    // If there was a preset selected, keep it selected
    if (presetList.contains(currentPreset))
    {
        presetComboBox.setText(currentPreset, juce::dontSendNotification);
    }
}

void CyberDistoAudioProcessorEditor::loadPreset()
{
    int selectedId = presetComboBox.getSelectedId();
    if (selectedId > 0)
    {
        juce::String presetName = presetComboBox.getItemText(selectedId - 1);
        const char* xmlData = PresetData::getPresetXml(presetName);
        
        if (xmlData != nullptr)
        {
            juce::String xmlString(xmlData);
            std::unique_ptr<juce::XmlElement> xmlState(juce::XmlDocument::parse(xmlString));
            
            if (xmlState != nullptr && xmlState->hasTagName("Parameters"))
            {
                juce::MemoryBlock destData;
                juce::AudioProcessor::copyXmlToBinary(*xmlState, destData);
                audioProcessor.setStateInformation(destData.getData(), static_cast<int>(destData.getSize()));
            }
        }
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

void CyberDistoAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &lofiXSlider || slider == &lofiYSlider)
    {
        lofiXYPad.updateFromSliders();
    }
}

void CyberDistoAudioProcessorEditor::timerCallback()
{
    // Mettez ici le code que vous voulez exécuter à chaque intervalle de temps
    // Par exemple, vous pouvez mettre à jour l'affichage du spectre
    lofiXYPad.repaint();
}

juce::Rectangle<int> CyberDistoAudioProcessorEditor::getConstrainedBounds(int width, int height) const
{
    const float aspectRatio = 1.81f;
    int newWidth = juce::jmax(950, width);
    int newHeight = juce::jmax(524, height);

    if (static_cast<float>(newWidth) / static_cast<float>(newHeight) > aspectRatio)
    {
        newWidth = juce::roundToInt(newHeight * aspectRatio);
    }
    else
    {
        newHeight = juce::roundToInt(newWidth / aspectRatio);
    }

    return juce::Rectangle<int>(newWidth, newHeight);
}

XYPad::XYPad(CyberDistoAudioProcessor& p, juce::AudioProcessorValueTreeState& apvts, const juce::String& xParamId, const juce::String& yParamId)
    : audioProcessor(p), apvts(apvts), xParamId(xParamId), yParamId(yParamId)
{
    xAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, xParamId, *new juce::Slider());
    yAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, yParamId, *new juce::Slider());
    
    updateFromSliders();
    smoothedSpectrumData.resize(audioProcessor.getSpectrumData().size(), 0.0f);
    startTimerHz(30); // Keep this at 30 Hz
}

void XYPad::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Ensure 1:1 ratio
    float size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    bounds = bounds.withSizeKeepingCentre(size, size);

    // Draw background
    g.setColour(juce::Colours::black);
    g.fillRect(bounds);

    // Draw futuristic border
    g.setColour(juce::Colours::cyan.withAlpha(0.5f));
    float borderThickness = 2.0f;
    g.drawRect(bounds, borderThickness);

    // Draw corner accents
    float cornerSize = 10.0f;
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getX() + cornerSize, bounds.getY(), borderThickness);
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getY() + cornerSize, borderThickness);
    g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight() - cornerSize, bounds.getY(), borderThickness);
    g.drawLine(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getY() + cornerSize, borderThickness);
    g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getX() + cornerSize, bounds.getBottom(), borderThickness);
    g.drawLine(bounds.getX(), bounds.getBottom(), bounds.getX(), bounds.getBottom() - cornerSize, borderThickness);
    g.drawLine(bounds.getRight(), bounds.getBottom(), bounds.getRight() - cornerSize, bounds.getBottom(), borderThickness);
    g.drawLine(bounds.getRight(), bounds.getBottom(), bounds.getRight(), bounds.getBottom() - cornerSize, borderThickness);

    // Draw spectrum
    if (!smoothedSpectrumData.empty())
    {
        int numBars = static_cast<int>(smoothedSpectrumData.size());
        float barWidth = bounds.getWidth() / numBars;
        float maxBarHeight = bounds.getHeight();

        juce::ColourGradient gradient(juce::Colours::magenta, bounds.getBottomLeft(),
                                      juce::Colours::cyan, bounds.getTopRight(),
                                      false);

        float maxValue = *std::max_element(smoothedSpectrumData.begin(), smoothedSpectrumData.end());

        if (maxValue > 0.0f)
        {
            for (int i = 0; i < numBars; ++i)
            {
                float value = smoothedSpectrumData[i] / maxValue;
                value = std::pow(value, 0.5f); // Apply some non-linear scaling for better visualization

                float barHeight = value * maxBarHeight;
                float x = bounds.getX() + i * barWidth;
                float y = bounds.getBottom() - barHeight;

                g.setGradientFill(gradient);
                g.fillRect(x, y, barWidth * 0.9f, barHeight);

                float mirroredX = bounds.getRight() - (i + 1) * barWidth;
                g.fillRect(mirroredX, y, barWidth * 0.9f, barHeight);
            }
        }
    }

    // Draw XY pad dot
    float x = juce::jmap(currentPosition.x, 0.0f, 1.0f, bounds.getX(), bounds.getRight() - 10.0f);
    float y = juce::jmap(currentPosition.y, 0.0f, 1.0f, bounds.getY(), bounds.getBottom() - 10.0f);
    g.setColour(juce::Colours::white);
    g.fillEllipse(x, y, 10.0f, 10.0f);

    // Display current values
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    float freq = 200.0f * std::pow(44100.0f / 200.0f, currentPosition.x);
    int bits = juce::roundToInt((1.0f - currentPosition.y) * 15.0f) + 1;
    g.drawText(juce::String(static_cast<int>(std::round(freq))) + " Hz", bounds.removeFromTop(20), juce::Justification::centred, false);
    g.drawText(juce::String(bits) + " bits", bounds.removeFromBottom(20), juce::Justification::centred, false);
}


void XYPad::mouseDown(const juce::MouseEvent& event)
{
    mouseDrag(event);
}

void XYPad::mouseDrag(const juce::MouseEvent& event)
{
    auto bounds = getLocalBounds().toFloat();
    const auto& pos = event.position;
    
    // Convert the mouse position to a 0-1 range
    float x = (pos.x - bounds.getX()) / bounds.getWidth();
    float y = (pos.y - bounds.getY()) / bounds.getHeight();

    // Clamp the values between 0 and 1
    x = juce::jlimit(0.0f, 1.0f, x);
    y = juce::jlimit(0.0f, 1.0f, y);

    updatePosition({x, y});
}

void XYPad::updateFromSliders()
{
    currentPosition.setXY(apvts.getParameter(xParamId)->getValue(), 1.0f - apvts.getParameter(yParamId)->getValue());
    repaint();
}

void XYPad::updatePosition(juce::Point<float> newPosition)
{
    currentPosition = newPosition;
    
    apvts.getParameter(xParamId)->setValueNotifyingHost(newPosition.x);
    apvts.getParameter(yParamId)->setValueNotifyingHost(1.0f - newPosition.y);
    
    repaint();
}

void XYPad::timerCallback()
{
    const auto& newSpectrumData = audioProcessor.getSpectrumData();
    
    if (newSpectrumData.size() == smoothedSpectrumData.size())
    {
        for (size_t i = 0; i < smoothedSpectrumData.size(); ++i)
        {
            smoothedSpectrumData[i] = smoothedSpectrumData[i] * smoothingFactor + 
                                      newSpectrumData[i] * (1.0f - smoothingFactor);
        }
    }
    else
    {
        smoothedSpectrumData = newSpectrumData; // Reset if sizes don't match
    }

    repaint();
}

void XYPad::resized()
{
    // This method is now implemented
    repaint();
}

void CyberDistoAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &distortionTypeSelector)
    {
        int selectedId = distortionTypeSelector.getSelectedId();
        DistortionType type = static_cast<DistortionType>(selectedId - 1);
        audioProcessor.setDistortionType(type);
    }
}

void CyberDistoAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &prevDistortionButton)
    {
        updateDistortionType(-1);
    }
    else if (button == &nextDistortionButton)
    {
        updateDistortionType(1);
    }
}

void CyberDistoAudioProcessorEditor::updateDistortionType(int change)
{
    int currentIndex = distortionTypeSelector.getSelectedItemIndex();
    int newIndex = (currentIndex + change + distortionTypeSelector.getNumItems()) % distortionTypeSelector.getNumItems();
    distortionTypeSelector.setSelectedItemIndex(newIndex, juce::sendNotification);
}

void CyberDistoAudioProcessorEditor::drawRoundedBlock(juce::Graphics& g, const juce::Rectangle<int>& bounds, const juce::String& title, const juce::Colour& colour)
{
    const float cornerSize = 10.0f;
    
    // Use the provided color (which is now fully transparent)
    g.setColour(colour);
    g.fillRoundedRectangle(bounds.toFloat(), cornerSize);
    
    // Draw the title with a semi-transparent white color
    g.setColour(juce::Colours::white.withAlpha(0.8f));
    g.setFont(16.0f);
    g.drawFittedText(title, bounds.withTrimmedTop(5).withHeight(25).reduced(5, 0), juce::Justification::centredTop, 1);
}

void CyberDistoAudioProcessorEditor::savePreset()
{
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
                
                // Set the combo box text to the new preset name
                presetComboBox.setText(presetName, juce::dontSendNotification);
            }
            delete window;
        }));
}

void CyberDistoAudioProcessorEditor::changePreset(int direction)
{
    int currentIndex = presetComboBox.getSelectedItemIndex();
    int newIndex = (currentIndex + direction + presetComboBox.getNumItems()) % presetComboBox.getNumItems();
    presetComboBox.setSelectedItemIndex(newIndex, juce::sendNotification);
    loadPreset();
}

void CyberDistoAudioProcessorEditor::createSliderAttachment(const juce::String& parameterID, juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment)
{
    auto* param = audioProcessor.getParameters().getParameter(parameterID);
    if (param == nullptr)
    {
        juce::Logger::writeToLog("Parameter not found: " + parameterID);
        return;
    }
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getParameters(), parameterID, slider);
}

// Add this method to create and show the about dialog
void CyberDistoAudioProcessorEditor::showAboutDialog()
{
    if (!isAboutDialogVisible)
    {
        createAboutDialogOverlay();
        isAboutDialogVisible = true;
        repaint();
    }
}

// Add this method to create the about dialog overlay
void CyberDistoAudioProcessorEditor::createAboutDialogOverlay()
{
    class AboutDialog : public juce::Component
    {
    public:
        AboutDialog(CyberDistoAudioProcessorEditor& editor)
            : owner(editor)
        {
            closeButton.setButtonText("X");
            closeButton.onClick = [this] { owner.removeAboutDialogOverlay(); };
            addAndMakeVisible(closeButton);

            addAndMakeVisible(githubLink);
            addAndMakeVisible(linkedinLink);
            addAndMakeVisible(extraTerraLink);

            githubLink.setButtonText("GitHub: @DrFuturo");
            linkedinLink.setButtonText("LinkedIn: Boris H.");
            extraTerraLink.setButtonText("Extra Terra Links");

            githubLink.setURL(juce::URL("https://github.com/DrFuturo"));
            linkedinLink.setURL(juce::URL("https://www.linkedin.com/in/boris-h/"));
            extraTerraLink.setURL(juce::URL("https://linktr.ee/extraterra"));

            githubLink.setColour(juce::HyperlinkButton::textColourId, juce::Colours::lightblue);
            linkedinLink.setColour(juce::HyperlinkButton::textColourId, juce::Colours::lightblue);
            extraTerraLink.setColour(juce::HyperlinkButton::textColourId, juce::Colours::lightblue);

            setOpaque(false);
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().reduced(15);
            g.setColour(juce::Colours::darkgrey.withAlpha(0.9f));
            g.fillRoundedRectangle(bounds.toFloat(), 8.0f);

            g.setColour(juce::Colours::white);
            g.setFont(14.0f);

            auto logoRect = bounds.removeFromTop(70);
            g.drawImage(owner.logo, logoRect.toFloat(), juce::RectanglePlacement::centred);

            bounds.removeFromTop(15);

            g.drawText("Version: 0.9", bounds.removeFromTop(25), juce::Justification::centred, true);
            g.drawText("Coding: Boris Hoechstetter (Extra Terra)", bounds.removeFromTop(25), juce::Justification::centred, true);
            g.drawText("GUI Design: Boris Hoechstetter (Extra Terra)", bounds.removeFromTop(25), juce::Justification::centred, true);

            // Add Extra Terra logo (significantly smaller)
            auto extraTerraLogoRect = bounds.removeFromTop(70);  // Reduced height
            bounds.removeFromTop(260); // Add extra space after the logo
            float scale = 0.3f;  // Significantly smaller
            auto scaledWidth = owner.extraTerraLogo.getWidth() * scale;
            auto scaledHeight = owner.extraTerraLogo.getHeight() * scale;
            g.drawImage(owner.extraTerraLogo, 
                        extraTerraLogoRect.withSizeKeepingCentre(static_cast<int>(std::round(scaledWidth)), 
                                                                 static_cast<int>(std::round(scaledHeight))).toFloat(), 
                        juce::RectanglePlacement::centred);
        }

        void resized() override
        {
            auto bounds = getLocalBounds().reduced(15);
            closeButton.setBounds(bounds.removeFromTop(25).removeFromRight(25));

            bounds.removeFromTop(190); // Adjusted space for logo, text, and smaller Extra Terra logo
            
            // Add padding before the links
            bounds.removeFromTop(60); // Adjust this value to increase or decrease padding
            
            auto linkHeight = 25;
            githubLink.setBounds(bounds.removeFromTop(linkHeight));
            linkedinLink.setBounds(bounds.removeFromTop(linkHeight));
            extraTerraLink.setBounds(bounds.removeFromTop(linkHeight));
        }

    private:
        CyberDistoAudioProcessorEditor& owner;
        juce::TextButton closeButton;
        juce::HyperlinkButton githubLink, linkedinLink, extraTerraLink;
    };

    aboutDialogOverlay = std::make_unique<AboutDialog>(*this);
    addAndMakeVisible(aboutDialogOverlay.get());

    // Make the dialog 30% smaller
    auto bounds = getLocalBounds().reduced(juce::roundToInt(static_cast<float>(getWidth()) * 0.15f), juce::roundToInt(static_cast<float>(getHeight()) * 0.15f));
    aboutDialogOverlay->setBounds(bounds);
    aboutDialogOverlay->setOpaque(false);
}

// Add this method to remove the about dialog overlay
void CyberDistoAudioProcessorEditor::removeAboutDialogOverlay()
{
    if (aboutDialogOverlay)
    {
        removeChildComponent(aboutDialogOverlay.get());
        aboutDialogOverlay.reset();
        isAboutDialogVisible = false;
        repaint();
    }
}

// Modify the mouseDown() method to handle logo clicks
void CyberDistoAudioProcessorEditor::mouseDown(const juce::MouseEvent& e)
{
    if ((logoClickArea.contains(e.getPosition()) || extraTerraClickArea.contains(e.getPosition())) 
        && e.eventComponent != &mixSlider)
    {
        showAboutDialog();
    }
    // The else part is removed to let other components handle their own mouse events
}

// Add or modify these methods for tooltip handling
void CyberDistoAudioProcessorEditor::mouseEnter(const juce::MouseEvent& e)
{
    if (e.eventComponent == &mixSlider)
    {
        auto knobBounds = mixSlider.getBounds();
        juce::Point<int> tooltipPosition(knobBounds.getCentreX(), knobBounds.getBottom());
        tooltipPosition = tooltipPosition.transformedBy(getTransform());
        tooltipComponent.showTooltip("Apply on all effects modules except Filter", tooltipPosition);
    }
}

void CyberDistoAudioProcessorEditor::mouseExit(const juce::MouseEvent& e)
{
    if (e.eventComponent == &mixSlider)
    {
        tooltipComponent.hideTooltip();
    }
}

void CyberDistoAudioProcessorEditor::mouseDrag(const juce::MouseEvent& e)
{
    if (e.eventComponent == &mixSlider)
    {
        tooltipComponent.hideTooltip();
    }
}

void CyberDistoAudioProcessorEditor::showTooltip(const juce::MouseEvent& /*e*/)
{
    if (!isTooltipVisible)
    {
        isTooltipVisible = true;
        
        auto knobBounds = mixSlider.getBounds();
        juce::Point<int> tooltipPosition(knobBounds.getCentreX(), knobBounds.getBottom());
        tooltipPosition = tooltipPosition.transformedBy(getTransform());
        
        tooltipComponent.showTooltip("Apply on all effects modules except Filter", tooltipPosition);
    }
}

void CyberDistoAudioProcessorEditor::hideTooltip()
{
    if (isTooltipVisible)
    {
        isTooltipVisible = false;
        tooltipComponent.hideTooltip();
    }
}