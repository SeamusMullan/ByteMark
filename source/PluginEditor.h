#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"
#include "MainTabComponent.h"
#include "HaasDelayTabComponent.h"
// #include "ChorusTabComponent.h"
// #include "ConvolutionTabComponent.h"
#include "CompressorTabComponent.h"
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

    // Tab Components
    MainTabComponent mainTab;
    HaasDelayTabComponent haasDelayTab;
    // ChorusTabComponent chorusTab;
    // ConvolutionTabComponent convolutionTab;
    CompressorTabComponent compressorTab;
    // Add other tabs as needed


    // Custom LookAndFeel for styling
    juce::LookAndFeel_V4 customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
