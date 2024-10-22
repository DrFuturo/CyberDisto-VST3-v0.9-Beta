#include <JuceHeader.h>
#include "CyberpunkLookAndFeel.h"
#include <cmath>


CyberpunkLookAndFeel::CyberpunkLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colour(0xFF00FFFF));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFF00FFFF));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF303030));
    setColour(juce::Label::textColourId, juce::Colour(0xFFE0E0E0));
    setColour(juce::ToggleButton::tickColourId, juce::Colour(0xFF00FFFF));
    setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xFF808080));
}

juce::Colour CyberpunkLookAndFeel::getLoFiKnobColor() const
{
    return juce::Colour(0xFF8A2BE2); // Purple color
}

void CyberpunkLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                            const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)).reduced(2.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    
    radius *= 0.75f;

    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = juce::jmin(2.5f, radius * 0.2f);
    auto arcRadius = radius + lineW * 2.0f;

    auto centre = bounds.getCentre();

    // Outer ring (black background)
    g.setColour(juce::Colours::transparentBlack);
    g.fillEllipse(centre.getX() - radius, centre.getY() - radius, radius * 2, radius * 2);

    // Darker grey line around the knob (partial circle)
    float startAngle = juce::MathConstants<float>::pi * 1.25f;
    float endAngle = juce::MathConstants<float>::pi * 1.75f;

    juce::Path backgroundArc;
    if (rotaryStartAngle < rotaryEndAngle)
    {
        backgroundArc.addCentredArc(centre.getX(), centre.getY(), arcRadius, arcRadius,
                                    0.0f, rotaryStartAngle, juce::jmin(rotaryEndAngle, endAngle), true);
        if (rotaryEndAngle > endAngle)
            backgroundArc.addCentredArc(centre.getX(), centre.getY(), arcRadius, arcRadius,
                                        0.0f, startAngle, rotaryEndAngle, true);
    }
    else
    {
        backgroundArc.addCentredArc(centre.getX(), centre.getY(), arcRadius, arcRadius,
                                    0.0f, rotaryEndAngle, juce::jmin(rotaryStartAngle, endAngle), true);
        if (rotaryStartAngle > endAngle)
            backgroundArc.addCentredArc(centre.getX(), centre.getY(), arcRadius, arcRadius,
                                        0.0f, startAngle, rotaryStartAngle, true);
    }

    g.setColour(juce::Colours::darkgrey.darker(0.1f));    
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW));

    // Value arc (outside the knob)
    juce::ColourGradient arcGradient;
    if (slider.getComponentID() == "lofiX" || slider.getComponentID() == "lofiY")
    {
        arcGradient = juce::ColourGradient(getLoFiKnobColor(), bounds.getTopLeft(),
                                           getLoFiKnobColor().brighter(0.5f), bounds.getBottomRight(), false);
    }
    else
    {
        auto baseColor = slider.findColour(juce::Slider::rotarySliderFillColourId);
        arcGradient = juce::ColourGradient(baseColor, bounds.getTopLeft(),
                                           baseColor.brighter(0.5f), bounds.getBottomRight(), false);
    }

    g.setGradientFill(arcGradient);

    juce::Path valueArc;
    if (rotaryStartAngle < toAngle)
    {
        valueArc.addCentredArc(centre.getX(), centre.getY(), arcRadius, arcRadius,
                               0.0f, rotaryStartAngle, juce::jmin(toAngle, endAngle), true);
        if (toAngle > endAngle)
            valueArc.addCentredArc(centre.getX(), centre.getY(), arcRadius, arcRadius,
                                   0.0f, startAngle, toAngle, true);
    }
    else
    {
        valueArc.addCentredArc(centre.getX(), centre.getY(), arcRadius, arcRadius,
                               0.0f, toAngle, juce::jmin(rotaryStartAngle, endAngle), true);
        if (rotaryStartAngle > endAngle)
            valueArc.addCentredArc(centre.getX(), centre.getY(), arcRadius, arcRadius,
                                   0.0f, startAngle, rotaryStartAngle, true);
    }
    g.strokePath(valueArc, juce::PathStrokeType(lineW));

    // Pointer (shortened)
    juce::Path pointer;
    auto pointerLength = radius * 0.25f;
    auto pointerThickness = 2.0f;
    pointer.addRectangle(-pointerThickness * 0.5f, -radius + lineW * 0.5f, pointerThickness, pointerLength);
    g.setColour(juce::Colours::white);
    g.fillPath(pointer, juce::AffineTransform::rotation(toAngle).translated(centre.x, centre.y));
}

juce::Colour CyberpunkLookAndFeel::getBackgroundColor() const
{
    return juce::Colour(0xFF141616);  // #272b2d en format ARGB
}

juce::Colour CyberpunkLookAndFeel::getGridColor() const
{
    return juce::Colour(0xFF303030);
}

#pragma warning(disable: 4996) // Disable warning for deprecated use of juce::Font::FontOptions, maybe to remove later
juce::Font CyberpunkLookAndFeel::getTitleFont() const
{
    return juce::Font(juce::Font::getDefaultSansSerifFontName(), 24.0f, juce::Font::bold);
}
#pragma warning(default: 4996)

juce::Colour CyberpunkLookAndFeel::getTitleColor() const
{
    return juce::Colour(0xFF00FFFF);
}

void CyberpunkLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                                                const juce::Colour& backgroundColour,
                                                bool shouldDrawButtonAsHighlighted [[maybe_unused]], 
                                                bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
    auto baseColour = backgroundColour;

    // Only change color if the button is pressed down
    if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.1f);

    // We're not changing the color for hover (shouldDrawButtonAsHighlighted)

    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 3.0f);
}

void CyberpunkLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                        int, int, int, int,
                                        juce::ComboBox& box)
{
    auto cornerSize = box.findParentComponentOfClass<juce::ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
    juce::Rectangle<int> boxBounds(0, 0, width, height);

    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);
}

void CyberpunkLookAndFeel::drawComboBoxTextWhenNothingSelected(juce::Graphics& g, juce::ComboBox& box, juce::Label& label)
{
    g.setColour(box.findColour(juce::ComboBox::textColourId).withMultipliedAlpha(0.5f));
    g.setFont(label.getFont());
    g.drawFittedText(box.getTextWhenNothingSelected(), label.getBounds().reduced(2, 1), label.getJustificationType(), juce::jmax(1, (int)(label.getHeight() / label.getFont().getHeight())));
}

void CyberpunkLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(1, 1, box.getWidth() - 2, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
}

