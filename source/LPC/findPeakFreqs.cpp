//
// Created by seamu on 21/12/2024.
//

#include "findPeakFreqs.h"


/**
 * Finds the peak frequencies in the given signal.
 * @param inputSignal: The input signal as a vector of floats.
 * @return A vector of peak frequencies sorted in ascending order.
 */
std::vector<float> findPeakFreqs::findPeakFrequencies(const std::vector<float>& inputSignal)
{
    // Ensure the input signal is non-empty
    if (inputSignal.empty())
        return {};

    // Compute FFT size
    size_t nFFT = 4 * (1 << static_cast<int>(std::ceil(std::log2(inputSignal.size()))));

    // Prepare FFT buffers
    juce::dsp::FFT fft(static_cast<int>(std::log2(nFFT)));
    std::vector<std::complex<float>> fftBuffer(nFFT);
    std::vector<float> magnitudes(nFFT / 2);

    // Copy input signal into FFT buffer and zero-pad
    std::copy(inputSignal.begin(), inputSignal.end(), fftBuffer.begin());
    std::fill(fftBuffer.begin() + inputSignal.size(), fftBuffer.end(), 0.0f);

    // Perform FFT
    fft.performFrequencyOnlyForwardTransform(reinterpret_cast<float*>(fftBuffer.data()));

    // Normalize the magnitude and keep only the first half
    auto maxMagnitude = *std::max_element(magnitudes.begin(), magnitudes.end());
    for (size_t i = 0; i < magnitudes.size(); ++i)
    {
        magnitudes[i] = std::abs(fftBuffer[i]) / maxMagnitude;
    }

    // Frequency axis
    std::vector<float> frequencies(magnitudes.size());
    for (size_t i = 0; i < frequencies.size(); ++i)
    {
        frequencies[i] = static_cast<float>(i) / static_cast<float>(nFFT) * fs;
    }

    // Find peaks
    std::vector<size_t> peakIndices = findPeaks(magnitudes);

    // Extract peak frequencies
    std::vector<float> peakFrequencies;
    for (auto idx : peakIndices)
    {
        peakFrequencies.push_back(frequencies[idx]);
    }

    // Sort and limit to the requested number of peaks
    std::sort(peakFrequencies.begin(), peakFrequencies.end());
    if (peakFrequencies.size() > static_cast<size_t>(np))
    {
        peakFrequencies.resize(np);
    }

    return peakFrequencies;
}



/**
 * Helper function to find peaks in a signal.
 * @param magnitudes: The magnitudes of the FFT.
 * @return Indices of the peaks sorted by descending magnitude.
 */
std::vector<size_t> findPeakFreqs::findPeaks(const std::vector<float>& magnitudes)
{
    std::vector<size_t> peakIndices;

    for (size_t i = 1; i < magnitudes.size() - 1; ++i)
    {
        if (magnitudes[i] > magnitudes[i - 1] && magnitudes[i] > magnitudes[i + 1])
        {
            peakIndices.push_back(i);
        }
    }

    // Sort indices by magnitude in descending order
    std::sort(peakIndices.begin(), peakIndices.end(), [&magnitudes](size_t a, size_t b) {
        return magnitudes[a] > magnitudes[b];
    });

    return peakIndices;
}
