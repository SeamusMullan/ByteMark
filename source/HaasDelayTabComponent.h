//
// Created by Seamus Mullan on 26/09/2024.
//

#ifndef HAASDELAYTABCOMPONENT_H
#define HAASDELAYTABCOMPONENT_H



#pragma once


#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "SliderWithLabel.h"

class HaasDelayTabComponent : public juce::Component
{
public:
    HaasDelayTabComponent(PluginProcessor& p);
    ~HaasDelayTabComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& processorRef;

    // UI Components
    SliderWithLabel haasTime;
    SliderWithLabel haasMix;

    // Attachments
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    Attachment haasTimeAttachment;
    Attachment haasMixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HaasDelayTabComponent)
};



#endif //HAASDELAYTABCOMPONENT_H
