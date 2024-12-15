//
// Created by Seamus Mullan on 30/09/2024.
//

#include "ReferenceTabComponent.h"

ReferenceTabComponent::ReferenceTabComponent(PluginProcessor& p)
    : processorRef(p),

      mix ("Mix"),
      mixAttachment(processorRef.apvts, "REFERENCE_MIX", mix.slider)


{
    addAndMakeVisible(mix);
}

ReferenceTabComponent::~ReferenceTabComponent()
{
}

void ReferenceTabComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void ReferenceTabComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::row;
    flexBox.items.add(juce::FlexItem(mix).withFlex(1).withMargin(5));

    flexBox.performLayout(area);
}
