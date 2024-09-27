#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "SliderWithLabel.h"

class OptionsMenu : public juce::Component, public juce::Button::Listener
{
public:
    OptionsMenu(PluginProcessor& p);
    ~OptionsMenu() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Remove the show() method declaration
    // void show();

    // Button listener
    void buttonClicked(juce::Button* button) override;

private:
    PluginProcessor& processorRef;

    // Close button
    juce::TextButton closeButton { "Close" };

    // Smoothing Slider
    SliderWithLabel smoothingSlider;

    // Attachments
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    Attachment smoothingAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OptionsMenu)
};
