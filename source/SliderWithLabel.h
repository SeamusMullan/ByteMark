//
// Created by Seamus Mullan on 26/09/2024.
//

#ifndef SLIDERWITHLABEL_H
#define SLIDERWITHLABEL_H
#pragma once

#include "MainTabComponent.h"

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

class SliderWithLabel : public juce::Component
{
public:
    SliderWithLabel(const juce::String& labelText, 
                    juce::Slider::SliderStyle style = juce::Slider::RotaryVerticalDrag)
    {
        slider.setSliderStyle(style);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        addAndMakeVisible(slider);

        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);

        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        
        // Consider using a separate look and feel or heap allocation
        lookAndFeel = std::make_unique<SliderWithLabelLookAndFeel>();
        setLookAndFeel(lookAndFeel.get());
    }

    ~SliderWithLabel()
    {
        setLookAndFeel(nullptr);
    }

    class SliderWithLabelLookAndFeel : public juce::LookAndFeel_V4
    {
    public:

        SliderWithLabelLookAndFeel()
        {
            image = juce::ImageCache::getFromMemory(BinaryData::main_62x62knob_png, BinaryData::main_62x62knob_pngSize);
            // byImage = juce::ImageCache::getFromMemory(BinaryData::LittlePhatty_OFF_png, BinaryData::LittlePhatty_OFF_pngSize);
        }

        ~SliderWithLabelLookAndFeel() override
        {
        }

        void drawRotarySlider(
            juce::Graphics& g,
            int x,
            int y,
            int width,
            int height,
            float sliderPosProportional,
            float /*rotaryStartAngle*/,
            float /*rotaryEndAngle*/,
            juce::Slider& /*slider*/) override
        {
            const int frames = static_cast<int>(height/width);
            const auto frameId = static_cast<int>(ceil(sliderPosProportional * (static_cast<float>(frames) - 1.0f)));

            // image is vertical strip, each frame is 1/100th of the height

            g.drawImage(image,
                x,
                y,
                width,
                height,
                0,
                frameId * image.getHeight() / frames,
                image.getWidth(),
                image.getHeight() / frames);
        }

    private:
        juce::Image image;
        juce::Image byImage;

        bool bp = false;


        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SliderWithLabelLookAndFeel)
    } SliderWithLabelLookAndFeel;

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
