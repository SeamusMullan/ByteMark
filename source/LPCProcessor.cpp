#include "LPCProcessor.h"
#include <cmath>
#include <algorithm>

LPCProcessor::LPCProcessor(int maxOrder, double preEmphasisAlpha)
    : maxOrder(std::clamp(maxOrder, 1, 24)),
      preEmphAlpha(std::clamp(preEmphasisAlpha, 0.0, 1.0)) {}

void LPCProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    Fs = sampleRate;
    frameSize = samplesPerBlock;
    hammingWindow.resize(frameSize);
    currentLPCCoefficients.resize(2); // Assuming stereo

    // Generate the Hamming window coefficients
    for (int i = 0; i < frameSize; ++i)
        hammingWindow[i] = 0.54 - 0.46 * std::cos(2.0 * juce::MathConstants<double>::pi * i / (frameSize - 1));

    // Reset state
    reset();
}

void LPCProcessor::reset()
{
    prevSamples.assign(2, 0.0f);  // Assuming stereo
    for (auto& channelCoeffs : currentLPCCoefficients)
        channelCoeffs.assign(maxOrder + 1, 0.0);
}

void LPCProcessor::process(juce::AudioBuffer<float>& buffer)
{
    // Safety check
    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0)
        return;

    // Ensure prevSamples vector is large enough
    prevSamples.resize(std::max(static_cast<size_t>(buffer.getNumChannels()), prevSamples.size()), 0.0f);

    // Resize LPC coefficients storage if needed
    currentLPCCoefficients.resize(buffer.getNumChannels());
    for (auto& channelCoeffs : currentLPCCoefficients)
        channelCoeffs.resize(maxOrder + 1);

    // Process each channel
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        float localPrevSample = prevSamples[channel];

        // Pre-emphasis
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float currentSample = channelData[i];
            channelData[i] -= preEmphAlpha * localPrevSample;
            localPrevSample = currentSample;
        }

        // Update prevSample for this channel
        prevSamples[channel] = localPrevSample;

        // Apply Hamming window
        for (int i = 0; i < std::min(buffer.getNumSamples(), frameSize); ++i)
            channelData[i] *= hammingWindow[i];
    }

    // Perform LPC Analysis
    LPCAnalysis(buffer);
}

void LPCProcessor::LPCAnalysis(juce::AudioBuffer<float>& buffer)
{
    // Process each channel
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        std::vector<double> signal(frameSize);

        // Copy data to signal buffer and apply Hamming window
        for (int i = 0; i < frameSize; ++i)
            signal[i] = channelData[i] * hammingWindow[i];

        // Compute autocorrelation
        std::vector<double> autocorr(maxOrder + 1, 0.0);
        for (int lag = 0; lag <= maxOrder; ++lag)
        {
            for (int n = 0; n < frameSize - lag; ++n)
                autocorr[lag] += signal[n] * signal[n + lag];
        }

        // Perform Levinson-Durbin recursion
        std::vector<double> lpcCoeffs(maxOrder + 1, 0.0);
        std::vector<double> reflectionCoeffs(maxOrder, 0.0);
        LevinsonDurbin(autocorr, maxOrder, lpcCoeffs, reflectionCoeffs);

        // Apply LPC filter for synthesis
        std::vector<double> residual(frameSize, 0.0);
        for (int n = maxOrder; n < frameSize; ++n)
        {
            double predicted = 0.0;
            for (int k = 1; k <= maxOrder; ++k)
                predicted += lpcCoeffs[k] * signal[n - k];

            residual[n] = signal[n] - predicted;
        }

        // Synthesize the signal from the residual
        for (int n = maxOrder; n < frameSize; ++n)
        {
            double reconstructed = residual[n];
            for (int k = 1; k <= maxOrder; ++k)
                reconstructed += lpcCoeffs[k] * residual[n - k];
            double blendFactor = std::min(1.0, (n - maxOrder) / 10.0);
            signal[n] = blendFactor * reconstructed + (1.0 - blendFactor) * signal[n];
        }

        // Copy reconstructed signal back to buffer
        for (int i = 0; i < frameSize; ++i)
            channelData[i] = static_cast<float>(signal[i]);
    }
}

void LPCProcessor::LevinsonDurbin(const std::vector<double>& autocorr, int order,
                    std::vector<double>& lpcCoeffs, std::vector<double>& reflectionCoeffs)
{
    // Add small epsilon to prevent division by zero
    const double epsilon = 1e-10;

    // Reset coefficient vectors
    std::fill(lpcCoeffs.begin(), lpcCoeffs.end(), 0.0);
    std::fill(reflectionCoeffs.begin(), reflectionCoeffs.end(), 0.0);

    lpcCoeffs[0] = 1.0;
    double error = autocorr[0];

    if (error < epsilon) {
        // If energy is too low, zero out coefficients
        std::fill(lpcCoeffs.begin(), lpcCoeffs.end(), 0.0);
        return;
    }

    for (int i = 1; i <= order; ++i)
    {
        // More robust summation
        double sum = 0.0;
        for (int j = 1; j < i; ++j)
            sum += lpcCoeffs[j] * autocorr[i - j];

        // Additional stability checks
        double reflectionCoeff = (autocorr[i] - sum) / (error + epsilon);

        // Clip reflection coefficient to prevent extreme values
        reflectionCoeff = std::clamp(reflectionCoeff, -0.99, 0.99);

        reflectionCoeffs[i - 1] = reflectionCoeff;
        lpcCoeffs[i] = reflectionCoeff;

        // Update LPC coefficients
        for (int j = 1; j < i; ++j)
            lpcCoeffs[j] -= reflectionCoeff * lpcCoeffs[i - j];

        // Update error with numerical stability
        error *= (1.0 - reflectionCoeff * reflectionCoeff);

        // Prevent error from becoming too small
        error = std::max(error, epsilon);
    }
}

void LPCProcessor::setMaxOrder(int newOrder)
{
    maxOrder = std::clamp(newOrder, 1, 20);
}

void LPCProcessor::setPreEmphasisAlpha(double alpha)
{
    preEmphAlpha = std::clamp(alpha, 0.0, 1.0);
}