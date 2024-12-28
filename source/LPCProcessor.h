//
// Created by seamu on 28/12/2024.
//

#ifndef LPCPROCESSOR_H
#define LPCPROCESSOR_H


#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>

class LPCProcessor
{
public:
    LPCProcessor(int lpcOrder, int windowSize);
    ~LPCProcessor();

    void process(const juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& outputBuffer);

    void setLpcOrder(int newOrder) { lpcOrder = newOrder; }
    void setWindowSize(int newSize);
    void setPitchDetectionEnabled(bool enabled) { pitchDetectionEnabled = enabled; }
    void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; }
    void setTargetSampleRate(double newSampleRate) { targetSampleRate = newSampleRate; }

private:
    int lpcOrder;                             // LPC order
    int windowSize;                           // Window size in samples
    bool pitchDetectionEnabled;               // Flag for enabling pitch detection
    double sampleRate = 44100.0;              // Default sample rate
    double targetSampleRate = 8000.0;

    juce::dsp::WindowingFunction<float> window; // Windowing function for OLA

    // Helper functions
    void stackOLA(const float* input, size_t numSamples, std::vector<std::vector<float>>& stackedOutput);
    void pressStack(const std::vector<std::vector<float>>& stackedInput, float* output, int outputSize);

    void encodeLPC(const std::vector<std::vector<float>>& stackedData,
                   std::vector<std::vector<float>>& lpcCoefficients,
                   std::vector<float>& signalPower,
                   std::vector<float>& pitchFrequencies);

    void decodeLPC (const std::vector<std::vector<float>>& lpcCoefficients,
        const std::vector<float>& signalPower,
        const std::vector<float>& pitchFrequencies,
        size_t numSegments,
        std::vector<std::vector<float>>& synthesizedData);
    void computeAutocorrelation (const float* input, size_t numSamples, int order, std::vector<float>& autocorrelation);

    void computeLpc(const float* input, size_t numSamples, int order,
                    std::vector<float>& coefficients, float& power);

    float detectPitch(const std::vector<float>& segment);

    void performFFT(const std::vector<float>& input, std::vector<float>& magnitudes);
};

#endif //LPCPROCESSOR_H
