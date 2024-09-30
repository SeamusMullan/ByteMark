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

class CompressorTabComponent : public juce::Component
{
public:
    CompressorTabComponent(PluginProcessor& p);
    ~CompressorTabComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& processorRef;

    // UI Components
    SliderWithLabel threshold;
    SliderWithLabel ratio;
    SliderWithLabel attack;
    SliderWithLabel release;
    SliderWithLabel mix;

    // Attachments
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    Attachment thresholdAttachment;
    Attachment ratioAttachment;
    Attachment attackAttachment;
    Attachment releaseAttachment;
    Attachment mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompressorTabComponent)
};



#endif //CompressorTABCOMPONENT_H
