#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      spectrumAnalyzer (p),
      mainTab (processorRef),
      referenceTab (processorRef)
{
    // Set custom LookAndFeel
    setLookAndFeel(&customLookAndFeel);

    // Add tabs
    tabbedComponent.addTab("Main", juce::Colours::darkgrey, &mainTab, false);
    tabbedComponent.addTab("Reference", juce::Colours::darkgrey, &referenceTab, false);

    addAndMakeVisible(tabbedComponent);
    addAndMakeVisible (spectrumAnalyzer);


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
    spectrumAnalyzer.setVisualizerSmoothingValue (processorRef.apvts.getRawParameterValue ("VIS_SMOOTH")->load());
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();

    auto mainArea = area.removeFromTop(area.getHeight()*0.66f);
    auto visualizerArea = area;

    tabbedComponent.setBounds(mainArea);
    spectrumAnalyzer.setBounds(visualizerArea);

    // Position the Melatonin Inspector Button in the bottom right corner
    inspectButton.setBounds(getWidth() - 80 - 10, getHeight() - 30 - 10, 80, 30);

}
