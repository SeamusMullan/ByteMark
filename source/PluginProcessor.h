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
            - Filter Type (Low Pass & High Pass for multiband processing)
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

        // Pre Filters
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"LOW_MID_FREQ", 1}, "Low Mid Crossover Frequency", 0.0f, 8000.0f, 8000.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"MID_HIGH_FREQ", 1}, "Mid High Crossover Frequency", 8000.0f, 15000.0f, 15000.0f));

        // Band Gains
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"LOW_GAIN", 1}, "Low Gain", -60.0f, 10.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"MID_GAIN", 1}, "Mid Gain", -60.0f, 10.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"HIGH_GAIN", 1}, "High Gain", -60.0f, 10.0f, 0.0f));

        // Haas Delay
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"TIME", 1}, "Time (ms)", 5.0f, 35.0f, 20.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"HAAS_MIX", 1}, "Haas Mix", 0.0f, 100.0f, 50.0f));

        // Chorus
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"CHORUS_RATE", 1}, "Chorus Rate", 0.1f, 5.0f, 1.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"CHORUS_DEPTH", 1}, "Chorus Depth", 0.0f, 1.0f, 0.5f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"CHORUS_CENTRE_DELAY", 1}, "Chorus Centre Delay", 1.0f, 30.0f, 10.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"CHORUS_FEEDBACK", 1}, "Chorus Feedback", -95.0f, 95.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"CHORUS_MIX", 1}, "Chorus Mix", 0.0f, 100.0f, 50.0f));

        // Convolution
        // Impulse Response?
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"CONV_MIX", 1}, "Convolution Mix", 0.0f, 100.0f, 50.0f));

        // Compressor

        // Compressor Parameters
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"COMPRESSOR_THRESHOLD", 1}, "Compressor Threshold", -60.0f, 0.0f, -24.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"COMPRESSOR_RATIO", 1}, "Compressor Ratio", 1.0f, 20.0f, 4.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"COMPRESSOR_ATTACK", 1}, "Compressor Attack", 1.0f, 100.0f, 10.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"COMPRESSOR_RELEASE", 1}, "Compressor Release", 10.0f, 500.0f, 100.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"COMPRESSOR_MIX", 1}, "Compressor Mix", 0.0f, 100.0f, 50.0f));

        return { params.begin(), params.end() };
    }

private:

    juce::dsp::Compressor<float> compressor;

    juce::dsp::LinkwitzRileyFilter<float> preLP, preHP;
    juce::dsp::Gain<float> lowGain, midGain, highGain;

    juce::dsp::DelayLine<float> haasDelay; // 5-35  ms
    juce::dsp::Chorus<float> chorus;
    // juce::dsp::WaveShaper<float> waveshaper;
    juce::dsp::Convolution convolution; // ir for cabinets or reverb?



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
