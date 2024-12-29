//
// Created by Seamus Mullan on 30/09/2024.
//

#include "ReferenceTabComponent.h"

ReferenceTabComponent::ReferenceTabComponent(PluginProcessor& p)
    : processorRef(p),

      lpcOrder ("LPC Order"),
      lpcOrderAttachment(processorRef.apvts, "LPC_ORDER", lpcOrder.slider),

      lpcAlpha ("LPC Alpha"),
      lpcAlphaAttachment(processorRef.apvts, "LPC_ALPHA", lpcAlpha.slider),

      lpcSampleRate ("LPC Sample Rate"),
      lpcSampleRateAttachment (processorRef.apvts, "LPC_SAMPLE_RATE", lpcSampleRate.slider),

      lpcPitchEnabledButton("LPC Pitch Detection"),
      lpcPitchEnabledButtonAttachment (processorRef.apvts, "PITCH_DETECTION", lpcPitchEnabledButton)


{
    addAndMakeVisible(lpcOrder);
    addAndMakeVisible(lpcAlpha);
    addAndMakeVisible (lpcSampleRate);
    addAndMakeVisible (lpcPitchEnabledButton);
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
    flexBox.items.add(juce::FlexItem(lpcOrder).withFlex(1).withMargin(5));
    flexBox.items.add(juce::FlexItem(lpcAlpha).withFlex(1).withMargin(5));
    flexBox.items.add(juce::FlexItem(lpcSampleRate).withFlex(1).withMargin(5));
    flexBox.items.add(juce::FlexItem(lpcPitchEnabledButton).withFlex(1).withMargin(5));

    flexBox.performLayout(area);
}
