//
// Created by Seamus Mullan on 30/09/2024.
//

#include "ReferenceTabComponent.h"

ReferenceTabComponent::ReferenceTabComponent(PluginProcessor& p)
    : processorRef(p),
      // haasTime("Haas Time"),
      // haasMix("Haas Mix"),
      // haasTimeAttachment(processorRef.apvts, "TIME", haasTime.slider),
      // haasMixAttachment(processorRef.apvts, "HAAS_MIX", haasMix.slider)
      threshold ("Threshold"),
      ratio ("Ratio"),
      attack ("Attack"),
      release ("Release"),
      mix ("Mix"),
      thresholdAttachment(processorRef.apvts, "COMPRESSOR_THRESHOLD", threshold.slider),
      ratioAttachment(processorRef.apvts, "COMPRESSOR_RATIO", ratio.slider),
      attackAttachment(processorRef.apvts, "COMPRESSOR_ATTACK", attack.slider),
      releaseAttachment(processorRef.apvts, "COMPRESSOR_RELEASE", release.slider),
      mixAttachment(processorRef.apvts, "COMPRESSOR_MIX", mix.slider)


{
    addAndMakeVisible(threshold);
    addAndMakeVisible(ratio);
    addAndMakeVisible(attack);
    addAndMakeVisible(release);
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
    flexBox.items.add(juce::FlexItem(threshold).withFlex(1).withMargin(5));
    flexBox.items.add(juce::FlexItem(ratio).withFlex(1).withMargin(5));
    flexBox.items.add(juce::FlexItem(attack).withFlex(1).withMargin(5));
    flexBox.items.add(juce::FlexItem(release).withFlex(1).withMargin(5));
    flexBox.items.add(juce::FlexItem(mix).withFlex(1).withMargin(5));

    flexBox.performLayout(area);
}
