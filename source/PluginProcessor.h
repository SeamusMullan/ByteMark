#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include "ParameterManager.h"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    // FIFO for audio visualization
    class FifoQueue
    {
    public:
        void push(const juce::AudioBuffer<float>& buffer);
        bool pull(juce::AudioBuffer<float>& buffer);

    private:
        static constexpr int bufferSize = 48000; // 1 second buffer at 48kHz
        juce::AbstractFifo fifo { bufferSize };
        juce::AudioBuffer<float> circularBuffer { 2, bufferSize };
    };

    FifoQueue fifoQueue;

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

        Main Settings
        - In / Out Gain
        - Bypass

        Visualizer Settings
        - Visualizer Smoothing Value

        */

        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        // Main Settings
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"IN", 1}, "In Gain", -60.0f, 10.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"OUT", 1}, "Out Gain", -60.0f, 10.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"BYPASS", 1}, "Bypass", false));

        // Visualizer settings
        params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"VIS_SMOOTH", 1}, "Visualizer Smoothing Value", 0.0f, 1.0f, 0.69f));

        return { params.begin(), params.end() };
    }

private:

    ParameterManager paramManager;

    juce::dsp::Compressor<float> compressor;

    juce::dsp::IIR::Filter<float> bassMonoFilter; // IIR has less latency (generally), may change to FIR because of linear phase...

    // Add additional filters for splitting
    juce::dsp::LinkwitzRileyFilter<float> lowMidCrossover;
    juce::dsp::LinkwitzRileyFilter<float> midHighCrossover;
    juce::dsp::Gain<float> lowGain, midGain, highGain;

    juce::dsp::DelayLine<float> haasDelay; // 5-35  ms
    juce::dsp::Chorus<float> chorus;
    // juce::dsp::WaveShaper<float> waveshaper;
    juce::dsp::Convolution convolution; // ir for cabinets or reverb?


    // visualiser

    juce::AudioBuffer<float> midBuffer;
    juce::AudioBuffer<float> sideBuffer;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
