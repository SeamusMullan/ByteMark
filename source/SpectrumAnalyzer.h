//
// Created by Seamus Mullan on 26/09/2024.
//

#ifndef SPECTRUMVISUALISER_H
#define SPECTRUMVISUALISER_H
#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_dsp/juce_dsp.h>

class SpectrumAnalyzer : public juce::Component,
                         private juce::Timer
{
public:
    SpectrumAnalyzer();
    ~SpectrumAnalyzer() override;
    SpectrumAnalyzer(PluginProcessor& p);

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setVisualizerSmoothingValue(float val);

    // Method to push audio data into the analyzer
    void pushBuffer(const juce::AudioBuffer<float>& buffer);

    void drawSpectrumPath(juce::Graphics& g, const std::vector<juce::Point<float>>& points);

private:

    PluginProcessor& processorRef;

    // FFT parameters
    static constexpr int fftOrder = 11; // 2048-point FFT
    static constexpr int fftSize = 1 << fftOrder;

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    // FIFO and buffers
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData; // For real and imaginary parts
    int fifoIndex = 0;



    bool nextFFTBlockReady = false;

    // Visualization parameters
    float visualizerSmoothingValue = 0.5f;

    std::vector<float> midSpectrum;
    std::vector<float> sideSpectrum;

    std::vector<float> midPeakSpectrum;
    std::vector<float> sidePeakSpectrum;


    void timerCallback() override;
    void drawNextFrameOfSpectrum();
    void applySmoothing();
    void drawFrame(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};


#endif //SPECTRUMVISUALISER_H
