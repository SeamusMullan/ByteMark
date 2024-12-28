//
// Created by seamu on 21/12/2024.
//

#ifndef FINDPEAKFREQS_H
#define FINDPEAKFREQS_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <algorithm>
#include <cmath>

class findPeakFreqs {

public:
    findPeakFreqs(float samplingFrequency = 1.0f, int numPeaks = 1) : fs(samplingFrequency), np(numPeaks) {}
    std::vector<float> findPeakFrequencies(const std::vector<float>& inputSignal);
    std::vector<size_t> findPeaks(const std::vector<float>& magnitudes);

private:
    float fs; // Sampling frequency
    int np;   // Number of peaks to find
};



#endif //FINDPEAKFREQS_H
