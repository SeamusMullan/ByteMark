//
// Created by Seamus Mullan on 26/09/2024.
//

#ifndef MAINTABCOMPONENT_H
#define MAINTABCOMPONENT_H
#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "SliderWithLabel.h"
#include "SpectrumAnalyzer.h"
#include "OptionsMenu.h"


class MainTabComponent : public juce::Component, public juce::Button::Listener
{
public:
    MainTabComponent(PluginProcessor& p);
    ~MainTabComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;

private:
    PluginProcessor& processorRef;



    juce::TextButton optionsButton { u8"⚙", "Options" };

    // UI Components
    SliderWithLabel inputGain;
    SliderWithLabel outputGain;
    SliderWithLabel bassMonoFreq;
    SliderWithLabel overallMix; // If you have an overall mix parameter
    juce::ToggleButton bassMonoButton;
    juce::ToggleButton bypassButton;
    juce::ToggleButton lowSoloButton, midSoloButton, highSoloButton;

    SliderWithLabel lowMidFreq;
    SliderWithLabel midHighFreq;

    // Attachments
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    Attachment inputGainAttachment;
    Attachment outputGainAttachment;
    Attachment bassMonoFreqAttachment;
    Attachment overallMixAttachment; // If applicable

    using Button = juce::AudioProcessorValueTreeState::ButtonAttachment;

    Button bypassAttachment;
    Button bassMonoButtonAttachment;

    Button lowSoloAttachment;
    Button midSoloAttachment;
    Button highSoloAttachment;

    Attachment lowMidFreqAttachment;
    Attachment midHighFreqAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainTabComponent)
};

#endif //MAINTABCOMPONENT_H
