#include "OptionsMenu.h"

OptionsMenu::OptionsMenu(PluginProcessor& p)
    : processorRef(p),
      smoothingSlider("Smoothing Value"),
      smoothingAttachment(processorRef.apvts, "VIS_SMOOTH", smoothingSlider.slider)
{
    addAndMakeVisible(smoothingSlider);
    addAndMakeVisible(closeButton);

    closeButton.addListener(this);

    // Set the size of the options menu
    setSize(450, 200);
}

OptionsMenu::~OptionsMenu()
{
    closeButton.removeListener(this);
}

void OptionsMenu::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB (45,45,45));
    g.setColour(juce::Colours::darkgrey);
}

void OptionsMenu::resized()
{
    auto area = getLocalBounds().reduced(10);


    smoothingSlider.setBounds(area.removeFromTop(50).removeFromLeft(150));
    closeButton.setBounds(area.removeFromBottom(30).removeFromRight(80));
}

void OptionsMenu::buttonClicked(juce::Button* button)
{
    if (button == &closeButton)
    {
        // Close the CallOutBox
        if (auto* callOutBox = findParentComponentOfClass<juce::CallOutBox>())
        {
            callOutBox->dismiss();
        }
    }
}
