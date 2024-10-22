#include "ParameterSelector.h"

ParameterSelector::ParameterSelector(juce::AudioProcessorValueTreeState& apvts, const juce::String& parameterID)
{
    addAndMakeVisible(selector);
    addAndMakeVisible(prevButton);
    addAndMakeVisible(nextButton);

    selector.addListener(this);
    prevButton.addListener(this);
    nextButton.addListener(this);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, parameterID, selector);
}

ParameterSelector::~ParameterSelector()
{
    selector.removeListener(this);
    prevButton.removeListener(this);
    nextButton.removeListener(this);
}

void ParameterSelector::resized()
{
    auto bounds = getLocalBounds();
    auto buttonWidth = bounds.getHeight();

    prevButton.setBounds(bounds.removeFromLeft(buttonWidth));
    nextButton.setBounds(bounds.removeFromRight(buttonWidth));
    selector.setBounds(bounds);
}

void ParameterSelector::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ParameterSelector::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &selector)
    {
        if (onSelectionChange)
            onSelectionChange(selector.getSelectedItemIndex());
    }
}

void ParameterSelector::buttonClicked(juce::Button* button)
{
    int currentIndex = selector.getSelectedItemIndex();
    int numItems = selector.getNumItems();

    if (button == &prevButton)
    {
        currentIndex = (currentIndex - 1 + numItems) % numItems;
    }
    else if (button == &nextButton)
    {
        currentIndex = (currentIndex + 1) % numItems;
    }

    selector.setSelectedItemIndex(currentIndex, juce::sendNotification);
}