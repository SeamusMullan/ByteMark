//
// Created by Seamus Mullan on 26/09/2024.
//

#ifndef SLIDERWITHLABEL_H
#define SLIDERWITHLABEL_H
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

class SliderWithLabel : public juce::Component
{
public:
    SliderWithLabel(const juce::String& labelText, juce::Slider::SliderStyle style = juce::Slider::RotaryVerticalDrag)
    {
        // Configure the slider
        slider.setSliderStyle(style);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        addAndMakeVisible(slider);

        // Configure the label
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        label.setBounds(area.removeFromTop(20));
        slider.setBounds(area);
    }

    juce::Slider slider;

private:
    juce::Label label;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderWithLabel);
};

#endif //SLIDERWITHLABEL_H
