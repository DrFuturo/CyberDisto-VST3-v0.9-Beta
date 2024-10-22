// PresetData.cpp
#include "PresetData.h"
#include <JuceHeader.h>

namespace PresetData
{
    // Définissez toutes les variables XML ici
    const char* const afterlifeXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Diode in the Shell"><PARAM id="attack" value="6.800000190734863"/><PARAM id="distMix" value="0.8299999833106995"/><PARAM id="distortionType" value="1.0"/><PARAM id="drive" value="9.0"/><PARAM id="extraTerra" value="0.8499999642372131"/><PARAM id="filterFreq" value="659.3599853515625"/><PARAM id="filterQ" value="3.739999771118164"/><PARAM id="lofiDryWet" value="1.0"/><PARAM id="lofiX" value="0.5899999737739563"/><PARAM id="lofiY" value="0.4899999797344208"/><PARAM id="mix" value="0.9099999666213989"/><PARAM id="postGain" value="1.439999938011169"/><PARAM id="preGain" value="0.8600000143051147"/><PARAM id="ratio" value="15.92000007629395"/><PARAM id="release" value="227.9000091552734"/><PARAM id="threshold" value="-8.880001068115234"/></Parameters>)";
    const char* const akiraDistoXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Akira Disto" DistortionType="3"><PARAM id="attack" value="6.800000190734863"/><PARAM id="distMix" value="0.8100000023841858"/><PARAM id="distortionType" value="1.0"/><PARAM id="drive" value="7.989999771118164"/><PARAM id="extraTerra" value="0.4199999868869781"/><PARAM id="filterFreq" value="20000.0"/><PARAM id="filterQ" value="0.1000000014901161"/><PARAM id="lofiDryWet" value="0.0"/><PARAM id="lofiX" value="0.5199999809265137"/><PARAM id="lofiY" value="0.3799999952316284"/><PARAM id="mix" value="1.0"/><PARAM id="postGain" value="1.0"/><PARAM id="preGain" value="1.0"/><PARAM id="ratio" value="9.010000228881836"/><PARAM id="release" value="503.6000061035156"/><PARAM id="threshold" value="-8.880001068115234"/></Parameters>)";
    const char* const cyberpunk2020Xml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Cyberpunk 2020" DistortionType="4"><PARAM id="attack" value="10.0"/><PARAM id="distMix" value="0.8100000023841858"/><PARAM id="distortionType" value="1.0"/><PARAM id="drive" value="8.629999160766602"/><PARAM id="extraTerra" value="0.0"/><PARAM id="filterFreq" value="339.6799926757812"/><PARAM id="filterQ" value="2.709999799728394"/><PARAM id="lofiDryWet" value="1.0"/><PARAM id="lofiX" value="0.2800000011920929"/><PARAM id="lofiY" value="0.7099999785423279"/><PARAM id="mix" value="1.0"/><PARAM id="postGain" value="1.0"/><PARAM id="preGain" value="1.0"/><PARAM id="ratio" value="6.579999923706055"/><PARAM id="release" value="100.0"/><PARAM id="threshold" value="-8.880001068115234"/></Parameters>)";
    const char* const destinyXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Destiny" DistortionType="6"><PARAM id="attack" value="36.10000228881836"/><PARAM id="distMix" value="1.0"/><PARAM id="distortionType" value="1.0"/><PARAM id="drive" value="8.989999771118164"/><PARAM id="extraTerra" value="0.3599999845027924"/><PARAM id="filterFreq" value="20000.0"/><PARAM id="filterQ" value="0.1000000014901161"/><PARAM id="lofiDryWet" value="0.5099999904632568"/><PARAM id="lofiX" value="0.03999999910593033"/><PARAM id="lofiY" value="0.4799999892711639"/><PARAM id="mix" value="1.0"/><PARAM id="postGain" value="1.350000023841858"/><PARAM id="preGain" value="0.4399999976158142"/><PARAM id="ratio" value="6.579999923706055"/><PARAM id="release" value="319.8000183105469"/><PARAM id="threshold" value="-11.76000213623047"/></Parameters>)";
    const char* const diodeInTheShellXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Diode in the Shell" DistortionType="4"><PARAM id="attack" value="6.800000190734863"/><PARAM id="distMix" value="0.7599999904632568"/><PARAM id="distortionType" value="1.0"/><PARAM id="drive" value="6.799999713897705"/><PARAM id="extraTerra" value="0.2199999988079071"/><PARAM id="filterFreq" value="259.760009765625"/><PARAM id="filterQ" value="3.739999771118164"/><PARAM id="lofiDryWet" value="1.0"/><PARAM id="lofiX" value="0.2099999934434891"/><PARAM id="lofiY" value="0.5299999713897705"/><PARAM id="mix" value="1.0"/><PARAM id="postGain" value="1.439999938011169"/><PARAM id="preGain" value="0.8600000143051147"/><PARAM id="ratio" value="9.010000228881836"/><PARAM id="release" value="503.6000061035156"/><PARAM id="threshold" value="-8.880001068115234"/></Parameters>)";
    const char* const frenchTouchPartyXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="French Touch Party" DistortionType="5"><PARAM id="attack" value="3.600000143051147"/><PARAM id="distMix" value="1.0"/><PARAM id="distortionType" value="1.0"/><PARAM id="drive" value="10.0"/><PARAM id="extraTerra" value="0.0"/><PARAM id="filterFreq" value="739.2799682617188"/><PARAM id="filterQ" value="4.289999961853027"/><PARAM id="lofiDryWet" value="0.7099999785423279"/><PARAM id="lofiX" value="0.5099999904632568"/><PARAM id="lofiY" value="1.0"/><PARAM id="mix" value="0.9099999666213989"/><PARAM id="postGain" value="1.100000023841858"/><PARAM id="preGain" value="1.230000019073486"/><PARAM id="ratio" value="14.31999969482422"/><PARAM id="release" value="383.7000122070312"/><PARAM id="threshold" value="-8.880001068115234"/></Parameters>)";
    const char* const gantzDystopiaXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Gantz Dystopia" DistortionType="7"><PARAM id="attack" value="6.800000190734863"/><PARAM id="distMix" value="1.0"/><PARAM id="distortionType" value="1.0"/><PARAM id="drive" value="1.259999990463257"/><PARAM id="extraTerra" value="0.02999999932944775"/><PARAM id="filterFreq" value="19040.958984375"/><PARAM id="filterQ" value="4.730000019073486"/><PARAM id="lofiDryWet" value="0.1099999994039536"/><PARAM id="lofiX" value="0.3999999761581421"/><PARAM id="lofiY" value="0.5"/><PARAM id="mix" value="1.0"/><PARAM id="postGain" value="1.439999938011169"/><PARAM id="preGain" value="0.8600000143051147"/><PARAM id="ratio" value="9.010000228881836"/><PARAM id="release" value="503.6000061035156"/><PARAM id="threshold" value="-8.880001068115234"/></Parameters>)";
    const char* const heavyCrunchXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Heavy Crunch" DistortionType="1"><PARAM id="attack" value="10.0"/><PARAM id="distMix" value="0.5"/><PARAM id="distortionType" value="0.0"/><PARAM id="drive" value="3.589999914169312"/><PARAM id="extraTerra" value="0.0"/><PARAM id="filterFreq" value="1000.0"/><PARAM id="filterQ" value="1.0"/><PARAM id="lofiDryWet" value="0.5"/><PARAM id="lofiX" value="0.5"/><PARAM id="lofiY" value="0.5"/><PARAM id="mix" value="0.5"/><PARAM id="postGain" value="1.0"/><PARAM id="preGain" value="1.0"/><PARAM id="ratio" value="4.0"/><PARAM id="release" value="100.0"/><PARAM id="threshold" value="-12.0"/></Parameters>)";
    const char* const interfaceBassXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Interface Bass"><PARAM id="attack" value="53.20000076293945"/><PARAM id="distMix" value="1.0"/><PARAM id="distortionType" value="0.0"/><PARAM id="drive" value="10.0"/><PARAM id="extraTerra" value="0.0"/><PARAM id="filterFreq" value="20000.0"/><PARAM id="filterQ" value="5.549999713897705"/><PARAM id="lofiDryWet" value="1.0"/><PARAM id="lofiX" value="0.4099999964237213"/><PARAM id="lofiY" value="0.3499999940395355"/><PARAM id="mix" value="1.0"/><PARAM id="postGain" value="1.0"/><PARAM id="preGain" value="1.0"/><PARAM id="ratio" value="3.889999866485596"/><PARAM id="release" value="100.0"/><PARAM id="threshold" value="0.0"/></Parameters>)";
    const char* const maxDistoXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Max Disto" DistortionType="4"><PARAM id="attack" value="10.0"/><PARAM id="distMix" value="1.0"/><PARAM id="distortionType" value="4.0"/><PARAM id="drive" value="10.0"/><PARAM id="extraTerra" value="0.5299999713897705"/><PARAM id="filterFreq" value="20000.0"/><PARAM id="filterQ" value="0.1000000014901161"/><PARAM id="lofiDryWet" value="0.0"/><PARAM id="lofiX" value="0.3299999833106995"/><PARAM id="lofiY" value="0.6200000047683716"/><PARAM id="mix" value="1.0"/><PARAM id="postGain" value="1.259999990463257"/><PARAM id="preGain" value="1.0"/><PARAM id="ratio" value="3.539999961853027"/><PARAM id="release" value="100.0"/><PARAM id="threshold" value="-12.0"/></Parameters>)";
    const char* const warmDistortionXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Warm Distortion" DistortionType="2"><PARAM id="attack" value="10.0"/><PARAM id="distMix" value="0.7299999594688416"/><PARAM id="distortionType" value="0.0"/><PARAM id="drive" value="6.980000019073486"/><PARAM id="extraTerra" value="0.0"/><PARAM id="filterFreq" value="1000.0"/><PARAM id="filterQ" value="1.0"/><PARAM id="lofiDryWet" value="0.5"/><PARAM id="lofiX" value="0.5"/><PARAM id="lofiY" value="0.5"/><PARAM id="mix" value="0.5"/><PARAM id="postGain" value="1.0"/><PARAM id="preGain" value="1.0"/><PARAM id="ratio" value="4.0"/><PARAM id="release" value="100.0"/><PARAM id="threshold" value="-12.0"/></Parameters>)";
    const char* const wowBassXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Wow Bass" DistortionType="8"><PARAM id="attack" value="6.800000190734863"/><PARAM id="distMix" value="0.8100000023841858"/><PARAM id="distortionType" value="1.0"/><PARAM id="drive" value="7.989999771118164"/><PARAM id="extraTerra" value="0.119999997317791"/><PARAM id="filterFreq" value="6333.6796875"/><PARAM id="filterQ" value="2.079999923706055"/><PARAM id="lofiDryWet" value="1.0"/><PARAM id="lofiX" value="0.0"/><PARAM id="lofiY" value="0.9899999499320984"/><PARAM id="mix" value="1.0"/><PARAM id="postGain" value="1.429999947547913"/><PARAM id="preGain" value="0.550000011920929"/><PARAM id="ratio" value="6.960000038146973"/><PARAM id="release" value="303.8000183105469"/><PARAM id="threshold" value="-13.92000198364258"/></Parameters>)";
    const char* const yuyVocXml = R"(<?xml version="1.0" encoding="UTF-8"?> <Parameters PresetName="Akira Disto"><PARAM id="attack" value="6.800000190734863"/><PARAM id="distMix" value="1.0"/><PARAM id="distortionType" value="1.0"/><PARAM id="drive" value="8.699999809265137"/><PARAM id="extraTerra" value="0.4199999868869781"/><PARAM id="filterFreq" value="259.760009765625"/><PARAM id="filterQ" value="3.819999933242798"/><PARAM id="lofiDryWet" value="1.0"/><PARAM id="lofiX" value="0.3799999952316284"/><PARAM id="lofiY" value="0.5699999928474426"/><PARAM id="mix" value="1.0"/><PARAM id="postGain" value="1.0"/><PARAM id="preGain" value="1.0"/><PARAM id="ratio" value="9.010000228881836"/><PARAM id="release" value="503.6000061035156"/><PARAM id="threshold" value="-8.880001068115234"/></Parameters>)";

    // Gardez les définitions des fonctions getPresetNames et getPresetXml comme elles étaient avant
    juce::StringArray getPresetNames()
    {
        return {"Afterlife", "Akira Disto", "Cyberpunk 2020", "Destiny", "Diode in the Shell", "French Touch Party", "Gantz Dystopia", "Heavy Crunch", "Interface Bass", "Max Disto", "Warm Distortion", "Wow Bass", "Yuy Voc"};
    }

    const char* getPresetXml(const juce::String& presetName)
    {
        if (presetName == "Afterlife")
            return afterlifeXml;
        else if (presetName == "Akira Disto")
            return akiraDistoXml;
        else if (presetName == "Cyberpunk 2020")
            return cyberpunk2020Xml;
        else if (presetName == "Destiny")
            return destinyXml;
        else if (presetName == "Diode in the Shell")
            return diodeInTheShellXml;
        else if (presetName == "French Touch Party")
            return frenchTouchPartyXml;
        else if (presetName == "Gantz Dystopia")
            return gantzDystopiaXml;
        else if (presetName == "Heavy Crunch")
            return heavyCrunchXml;
        else if (presetName == "Interface Bass")
            return interfaceBassXml;
        else if (presetName == "Max Disto")
            return maxDistoXml;
        else if (presetName == "Warm Distortion")
            return warmDistortionXml;
        else if (presetName == "Wow Bass")
            return wowBassXml;
        else if (presetName == "Yuy Voc")
            return yuyVocXml;
        else
            return nullptr;
    }
}