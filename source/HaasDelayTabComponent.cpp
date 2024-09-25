#include "HaasDelayTabComponent.h"

HaasDelayTabComponent::HaasDelayTabComponent(PluginProcessor& p)
    : processorRef(p),
      haasTime("Haas Time"),
      haasMix("Haas Mix"),
      haasTimeAttachment(processorRef.apvts, "TIME", haasTime.slider),
      haasMixAttachment(processorRef.apvts, "HAAS_MIX", haasMix.slider)
{
    addAndMakeVisible(haasTime);
    addAndMakeVisible(haasMix);
}

HaasDelayTabComponent::~HaasDelayTabComponent()
{
}

void HaasDelayTabComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void HaasDelayTabComponent::resized()
{
    auto area = getLocalBounds().reduced(10);

    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::row;
    flexBox.items.add(juce::FlexItem(haasTime).withFlex(1).withMargin(5));
    flexBox.items.add(juce::FlexItem(haasMix).withFlex(1).withMargin(5));

    flexBox.performLayout(area);
}
