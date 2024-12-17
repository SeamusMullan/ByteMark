#ifndef LPC_AUDIO_PROCESSOR_H
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class LPCProcessor
{
public:
    // Constructor with sensible defaults
    LPCProcessor(int maxOrder = 10, double preEmphasisAlpha = 0.95);

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);

    // Setter methods with basic validation
    void setMaxOrder(int maxOrder);
    void setPreEmphasisAlpha(double alpha);

    // Get current configuration
    int getMaxOrder() const { return maxOrder; }
    double getPreEmphasisAlpha() const { return preEmphAlpha; }

    // Optional: Method to get current LPC coefficients
    std::vector<std::vector<double>> getCurrentLPCCoefficients() const { 
        return currentLPCCoefficients; 
    }

    // Reset processor state
    void reset();

private:
    void LPCAnalysis(juce::AudioBuffer<float>& buffer);
    static void LevinsonDurbin(const std::vector<double>& autocorr, int order,
                        std::vector<double>& lpcCoeffs, std::vector<double>& reflectionCoeffs);

    std::vector<float> prevSamples;
    int frameSize = 0;
    int maxOrder;
    double Fs = 44100.0;
    double preEmphAlpha;

    std::vector<double> hammingWindow;
    
    // Store current LPC coefficients for each channel
    std::vector<std::vector<double>> currentLPCCoefficients;
};

#endif // LPC_AUDIO_PROCESSOR_H