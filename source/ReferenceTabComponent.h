//
// Created by Seamus Mullan on 30/09/2024.
//

//
// Created by Seamus Mullan on 26/09/2024.
//

#ifndef CompressorTABCOMPONENT_H
#define CompressorTABCOMPONENT_H

#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "SliderWithLabel.h"

class ReferenceTabComponent : public juce::Component
{
public:
    ReferenceTabComponent(PluginProcessor& p);
    ~ReferenceTabComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& processorRef;

    // UI Components
    SliderWithLabel lpcOrder;
    SliderWithLabel lpcAlpha;

    // Attachments
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    Attachment lpcOrderAttachment;
    Attachment lpcAlphaAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReferenceTabComponent)
};



#endif //CompressorTABCOMPONENT_H
