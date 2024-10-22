#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      mainTab (processorRef),
      haasDelayTab (processorRef),
      // chorusTab (processorRef),
      // convolutionTab (processorRef),
      compressorTab (processorRef)
{
    // Set custom LookAndFeel
    setLookAndFeel(&customLookAndFeel);

    // Add tabs
    tabbedComponent.addTab("Main", juce::Colours::darkgrey, &mainTab, false);
    tabbedComponent.addTab("Haas Delay", juce::Colours::darkgrey, &haasDelayTab, false);
    // tabbedComponent.addTab("Chorus", juce::Colours::darkgrey, &chorusTab, false);
    // tabbedComponent.addTab("Convolution", juce::Colours::darkgrey, &convolutionTab, false);
    tabbedComponent.addTab("Compressor", juce::Colours::darkgrey, &compressorTab, false);
    // Add other tabs as needed

    addAndMakeVisible(tabbedComponent);



    // Melatonin Inspector Button
    addAndMakeVisible (inspectButton);
    inspectButton.onClick = [&] {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() { inspector.reset(); };
        }

        inspector->setVisible (true);
    };

    // Allow the editor to be resizable
    setResizable(true, true);
    setResizeLimits(600, 400, 1920, 1080);
    setSize (960, 540);
}

PluginEditor::~PluginEditor()
{
    setLookAndFeel(nullptr); // Reset the LookAndFeel to avoid dangling pointer
}

void PluginEditor::paint (juce::Graphics& g)
{
    // Fill background
    g.fillAll (juce::Colours::black);

}

void PluginEditor::resized()
{
    auto area = getLocalBounds();
    tabbedComponent.setBounds(area);

    // Position the Melatonin Inspector Button at the bottom right corner
    inspectButton.setBounds(getWidth() - 80 - 10, getHeight() - 30 - 10, 80, 30);

}
