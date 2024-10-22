#pragma once

#include <JuceHeader.h>

class ParameterSelector : public juce::Component,
                          private juce::ComboBox::Listener,
                          private juce::Button::Listener
{
public:
    ParameterSelector(juce::AudioProcessorValueTreeState& apvts, const juce::String& parameterID);
    ~ParameterSelector() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    std::function<void(int)> onSelectionChange;

private:
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked(juce::Button* button) override;

    juce::ComboBox selector;
    juce::TextButton prevButton{"<"};
    juce::TextButton nextButton{">"};

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterSelector)
};