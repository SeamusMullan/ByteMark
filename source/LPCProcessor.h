//
// Created by seamu on 15/12/2024.
//

#ifndef LPC_AUDIO_PROCESSOR_H
#define LPC_AUDIO_PROCESSOR_H

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class LPCProcessor
{
public:
    LPCProcessor(int maxOrder = 12, double preEmphasisAlpha = 0.95);

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);

    void setMaxOrder(int maxOrder);
    void setPreEmphasisAlpha(double alpha);

private:
    void LPCAnalysis(juce::AudioBuffer<float>& buffer);
    static void LevinsonDurbin(const std::vector<double>& autocorr, int order,
                        std::vector<double>& lpcCoeffs, std::vector<double>& reflectionCoeffs);

    int frameSize = 0;
    int maxOrder    ;
    double Fs = 44100.0;
    double preEmphAlpha;
    float prevSample = 0.0f;

    std::vector<double> hammingWindow;
};

#endif // LPC_AUDIO_PROCESSOR_H
