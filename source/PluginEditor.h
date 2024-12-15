#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"
#include "MainTabComponent.h"
// #include "ChorusTabComponent.h"
// #include "ConvolutionTabComponent.h"
#include "ReferenceTabComponent.h"
// Include other tab components as needed

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Reference to the processor
    PluginProcessor& processorRef;

    // Melatonin Inspector
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect" };

    // Tabbed Component
    juce::TabbedComponent tabbedComponent { juce::TabbedButtonBar::TabsAtTop };

    SpectrumAnalyzer spectrumAnalyzer;

    // Tab Components
    MainTabComponent mainTab;
    ReferenceTabComponent referenceTab;


    // Custom LookAndFeel for styling
    juce::LookAndFeel_V4 customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
