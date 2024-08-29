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
};