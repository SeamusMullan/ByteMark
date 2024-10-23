//
// Created by Seamus Mullan on 26/09/2024.
//

#include "MainTabComponent.h"

#include "juce_audio_plugin_client/Unity/juce_UnityPluginInterface.h"

MainTabComponent::MainTabComponent (PluginProcessor& p)
    : processorRef (p),
      inputGain ("Input Gain"),
      outputGain ("Output Gain"),
      bassMonoButton ("Bass Mono"),
      bassMonoFreq ("Bass Mono Frequency"),
      overallMix ("Overall Mix"), // If you have this parameter
      lowMidFreq ("Low-Mid Freq", juce::Slider::LinearHorizontal),
      midHighFreq ("Mid-High Freq", juce::Slider::LinearHorizontal),
      lowSoloButton ("Low Band Solo"),
      midSoloButton ("Mid Band Solo"),
      highSoloButton ("High Band Solo"),

      inputGainAttachment (processorRef.apvts, "IN", inputGain.slider),
      outputGainAttachment (processorRef.apvts, "OUT", outputGain.slider),
      bassMonoFreqAttachment(processorRef.apvts, "BASS_MONO_FREQ", bassMonoFreq.slider),
      overallMixAttachment (processorRef.apvts, "OVERALL_MIX", overallMix.slider), // If applicable
      bypassAttachment (processorRef.apvts, "BYPASS", bypassButton),
      bassMonoButtonAttachment (processorRef.apvts, "BASS_MONO", bassMonoButton),

      lowSoloAttachment(processorRef.apvts, "LOW_SOLO", lowSoloButton),
      midSoloAttachment(processorRef.apvts, "MID_SOLO", midSoloButton),
      highSoloAttachment(processorRef.apvts, "HIGH_SOLO", highSoloButton),

      lowMidFreqAttachment (processorRef.apvts, "LOW_MID_FREQ", lowMidFreq.slider),
      midHighFreqAttachment (processorRef.apvts, "MID_HIGH_FREQ", midHighFreq.slider)
{
    addAndMakeVisible (inputGain);
    addAndMakeVisible (outputGain);
    addAndMakeVisible (bassMonoFreq);
    addAndMakeVisible (overallMix); // If applicable
    addAndMakeVisible (bypassButton);
    addAndMakeVisible (bassMonoButton);

    addAndMakeVisible (lowSoloButton);
    addAndMakeVisible (midSoloButton);
    addAndMakeVisible (highSoloButton);

    addAndMakeVisible (lowMidFreq);
    addAndMakeVisible (midHighFreq);



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

    // Band Splitting
    juce::FlexBox bandSplitControls;
    bandSplitControls.flexDirection = juce::FlexBox::Direction::row;
    bandSplitControls.items.add(juce::FlexItem(lowMidFreq).withFlex(1).withMargin(5));
    bandSplitControls.items.add(juce::FlexItem(midHighFreq).withFlex(1).withMargin(5));

    flexBox.items.add(juce::FlexItem(bandSplitControls).withFlex(1));

    // band mono toggles

    juce::FlexBox bandMonoToggles;
    bandMonoToggles.flexDirection = juce::FlexBox::Direction::row;
    bandMonoToggles.items.add(juce::FlexItem(lowSoloButton).withFlex(1).withMargin(5));
    bandMonoToggles.items.add(juce::FlexItem(midSoloButton).withFlex(1).withMargin(5));
    bandMonoToggles.items.add(juce::FlexItem(highSoloButton).withFlex(1).withMargin(5));

    flexBox.items.add(juce::FlexItem(bandMonoToggles).withFlex(1));

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
