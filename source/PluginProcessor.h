#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        /*
        Parameters List

        Pre-LP & HP
            - Filter Type
            - Cutoff Freq

        Multiband Pre-Gain
            - Low Gain
            - Mid Gain
            - High Gain

        Haas Delay
            - Time
            - Mix

        Chorus
            - Rate
            - Depth
            - Centre Delay
            - Feedback
            - Mix

        Waveshaper
            - n/a

        Convolution
            - Impulse Response
            - Mix

        Compressor
            - Threshold
            - Ratio
            - Attack
            - Release
            - Mix

        */

        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"IN", 1}, "In Gain", -60.0f, 10.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"OUT", 1}, "Out Gain", -60.0f, 10.0f, 0.0f));

        params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"BYPASS", 1}, "Bypass", false));

        return { params.begin(), params.end() };
    }

private:

    juce::dsp::Compressor<float> compressor;

    juce::dsp::LinkwitzRileyFilter<float> preLP, preHP;
    juce::dsp::Gain<float> lowGain, midGain, highGain;

    juce::dsp::DelayLine<float> haasDelay; // 5-35  ms
    juce::dsp::Chorus<float> chorus;
    // juce::dsp::WaveShaper<float> waveshaper;
    juce::dsp::Convolution<float> convolution; // ir for cabinets or reverb?



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
