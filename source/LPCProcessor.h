#pragma once

#include <JuceHeader.h>

class LPCProcessor {
public:
    LPCProcessor(int maxOrder = 12, double preEmphasisAlpha = 0.95);

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // Setters for tuning
    void setMaxOrder(int newOrder);
    void setPreEmphasisAlpha(double alpha);

private:
    // Core LPC Analysis Methods
    void calculateLPCCoefficients(
        const float* channelData,
        int numSamples,
        std::vector<double>& lpcCoefficients
    );

    void applyLPCReconstruction(
        float* channelData,
        int numSamples,
        const std::vector<double>& lpcCoefficients
    );

    // Utility methods
    float hammingWindow(int index, int frameLength);
    void applyHammingWindow(float* channelData, int numSamples);

    // Parameters
    int maxOrder;
    double preEmphAlpha;
    double sampleRate;

    // State variables
    std::vector<float> prevSamples;
    std::vector<std::vector<double>> currentLPCCoefficients;
};