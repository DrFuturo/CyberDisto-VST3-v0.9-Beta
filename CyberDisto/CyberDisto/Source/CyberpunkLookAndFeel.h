#pragma once

#include <JuceHeader.h>

class CyberpunkLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CyberpunkLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;

    juce::Colour getBackgroundColor() const;
    juce::Colour getGridColor() const;
    juce::Font getTitleFont() const;
    juce::Colour getTitleColor() const;
    juce::Colour getLoFiKnobColor() const;

    // Add this to your CyberpunkLookAndFeel class
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                  int buttonX, int buttonY, int buttonW, int buttonH,
                  juce::ComboBox& box) override;

    void drawComboBoxTextWhenNothingSelected(juce::Graphics& g, juce::ComboBox& box, juce::Label& label) override;
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;
};