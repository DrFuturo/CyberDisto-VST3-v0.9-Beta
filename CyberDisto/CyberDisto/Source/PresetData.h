#pragma once

#include <JuceHeader.h>

namespace PresetData
{
    extern const char* const afterlifeXml;
    extern const char* const akiraDistoXml;
    extern const char* const cyberpunk2020Xml;
    extern const char* const destinyXml;
    extern const char* const diodeInTheShellXml;
    extern const char* const frenchTouchPartyXml;
    extern const char* const gantzDystopiaXml;
    extern const char* const heavyCrunchXml;
    extern const char* const interfaceBassXml;
    extern const char* const maxDistoXml;
    extern const char* const warmDistortionXml;
    extern const char* const wowBassXml;
    extern const char* const yuyVocXml;

    juce::StringArray getPresetNames();
    const char* getPresetXml(const juce::String& presetName);
}
