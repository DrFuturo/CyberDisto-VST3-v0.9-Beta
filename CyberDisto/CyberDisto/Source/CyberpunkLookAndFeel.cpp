#include "CyberpunkLookAndFeel.h"

CyberpunkLookAndFeel::CyberpunkLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colour(0xFF00FFFF));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF00FFFF));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF303030));
    setColour(juce::Label::textColourId, juce::Colour(0xFFE0E0E0));
    setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFF00FFFF));
    setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xFF808080));
}

void CyberpunkLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                            const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(2.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = juce::jmin(8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius,
                                0.0f, rotaryStartAngle, rotaryEndAngle, true);

    g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius,
                           0.0f, rotaryStartAngle, toAngle, true);

    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path thumb;
    auto thumbWidth = lineW * 2.0f;
    thumb.addRectangle(-thumbWidth / 2, -thumbWidth / 2, thumbWidth, thumbWidth);

    g.setColour(slider.findColour(juce::Slider::thumbColourId));
    g.fillPath(thumb, juce::AffineTransform::rotation(toAngle).translated(bounds.getCentreX(), bounds.getCentreY()));
}

juce::Colour CyberpunkLookAndFeel::getBackgroundColor() const
{
    return juce::Colour(0xFF101010);
}

juce::Colour CyberpunkLookAndFeel::getGridColor() const
{
    return juce::Colour(0xFF303030);
}

juce::Font CyberpunkLookAndFeel::getTitleFont() const
{
    return juce::Font("Arial", 24.0f, juce::Font::bold);
}

juce::Colour CyberpunkLookAndFeel::getTitleColor() const
{
    return juce::Colour(0xFF00FFFF);
}