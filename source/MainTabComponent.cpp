//
// Created by Seamus Mullan on 26/09/2024.
//

#include "MainTabComponent.h"

#include "juce_audio_plugin_client/Unity/juce_UnityPluginInterface.h"

MainTabComponent::MainTabComponent (PluginProcessor& p)
    : processorRef (p),
      inputGain ("Input Gain"),
      outputGain ("Output Gain"),
      overallMix ("Overall Mix"), // If you have this parameter

      inputGainAttachment (processorRef.apvts, "IN", inputGain.slider),
      outputGainAttachment (processorRef.apvts, "OUT", outputGain.slider),
      overallMixAttachment (processorRef.apvts, "OVERALL_MIX", overallMix.slider), // If applicable
      bypassAttachment (processorRef.apvts, "BYPASS", bypassButton)
{
    addAndMakeVisible (inputGain);
    addAndMakeVisible (outputGain);
    addAndMakeVisible (overallMix); // If applicable
    addAndMakeVisible (bypassButton);

    // Options button
    addAndMakeVisible (optionsButton);

    optionsButton.addListener (this);
}

MainTabComponent::~MainTabComponent()
{
    optionsButton.removeListener(this);
}

void MainTabComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void MainTabComponent::resized()
{
    auto area = getLocalBounds();

    // Divide the area for controls and visualizer
    auto controlArea = area;

    juce::FlexBox flexBox;
    flexBox.flexDirection = juce::FlexBox::Direction::column;

    // Top Controls
    juce::FlexBox topControls;
    topControls.flexDirection = juce::FlexBox::Direction::row;
    topControls.items.add(juce::FlexItem(inputGain).withFlex(1).withMargin(5));
    topControls.items.add(juce::FlexItem(outputGain).withFlex(1).withMargin(5));
    topControls.items.add(juce::FlexItem(overallMix).withFlex(1).withMargin(5)); // If applicable
    topControls.items.add(juce::FlexItem(bypassButton).withMinWidth(100).withMinHeight(30).withMargin(juce::FlexItem::Margin(20, 10, 0, 10)).withAlignSelf(juce::FlexItem::AlignSelf::center));

    flexBox.items.add(juce::FlexItem(topControls).withFlex(1));

    // Band Splitting (use as example when implementing new parameters)
    // juce::FlexBox bandSplitControls;
    // bandSplitControls.flexDirection = juce::FlexBox::Direction::row;
    // bandSplitControls.items.add(juce::FlexItem(lowMidFreq).withFlex(1).withMargin(5));
    // bandSplitControls.items.add(juce::FlexItem(midHighFreq).withFlex(1).withMargin(5));

    // Layout
    flexBox.performLayout(controlArea);

    optionsButton.setBounds(getWidth()-40, 0, 40, 40);

}

void MainTabComponent::buttonClicked(juce::Button* button)
{
    if (button == &optionsButton)
    {
        auto optionsMenu = std::make_unique<OptionsMenu>(processorRef);
        juce::CallOutBox::launchAsynchronously(std::move(optionsMenu), optionsButton.getScreenBounds(), nullptr);
    }
}
