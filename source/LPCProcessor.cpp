#include "LPCProcessor.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include <juce_dsp/juce_dsp.h>

LPCProcessor::LPCProcessor(int lpcOrder, int windowSize)
    : lpcOrder(lpcOrder), windowSize(windowSize), pitchDetectionEnabled(false),
      window(windowSize, juce::dsp::WindowingFunction<float>::hann)
{
}

LPCProcessor::~LPCProcessor() {}

void LPCProcessor::setWindowSize(int newSize)
{
    windowSize = newSize;
    new (&window) juce::dsp::WindowingFunction<float>(windowSize, juce::dsp::WindowingFunction<float>::hann); // Reconstruct in place
}

void LPCProcessor::process(const juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& outputBuffer)
{
    const int numSamples = inputBuffer.getNumSamples();
    const int numChannels = inputBuffer.getNumChannels();
    const double currentSampleRate = sampleRate; // Assume `sampleRate` is a member variable representing the current rate
    const double resamplingFactor = targetSampleRate / currentSampleRate;

    outputBuffer.clear();

    // resample the input buffer and adjust the number of samples to represent it
    // resample to ~8k

    const int resampledNumSamples = static_cast<int>(numSamples * resamplingFactor);
    juce::AudioBuffer<float> resampledBuffer(numChannels, resampledNumSamples);

    // resample the input
    for (int channel = 0; channel < numChannels; ++channel)
    {
        // Resample the input signal
        const float* input = inputBuffer.getReadPointer(channel);
        float* resampled = resampledBuffer.getWritePointer(channel);

        // Resampling using linear interpolation
        for (int i = 0; i < resampledNumSamples; ++i)
        {
            float inputPosition = i / resamplingFactor;
            int pos0 = static_cast<int>(std::floor(inputPosition));
            int pos1 = std::min(pos0 + 1, numSamples - 1);
            float frac = inputPosition - pos0;

            resampled[i] = input[pos0] + frac * (input[pos1] - input[pos0]);
        }
    }


    // process the input
    for (int channel = 0; channel < numChannels; ++channel)
    {
        const float* input = resampledBuffer.getReadPointer(channel); // Use resampled data
        float* output = outputBuffer.getWritePointer(channel);

        // Stack the input signal into overlapping windows
        std::vector<std::vector<float>> stackedData;
        stackOLA(input, resampledNumSamples, stackedData);

        // Prepare buffers for LPC encoding
        std::vector<std::vector<float>> lpcCoefficients;
        std::vector<float> signalPower;
        std::vector<float> pitchFrequencies;

        encodeLPC(stackedData, lpcCoefficients, signalPower, pitchFrequencies);

        // Decode the signal
        std::vector<std::vector<float>> synthesizedData;
        decodeLPC(lpcCoefficients, signalPower, pitchFrequencies, stackedData.size(), synthesizedData);

        // Combine overlapping windows back into the output buffer
        pressStack(synthesizedData, output, numSamples);
    }
}

void LPCProcessor::encodeLPC(const std::vector<std::vector<float>>& stackedData,
                              std::vector<std::vector<float>>& lpcCoefficients,
                              std::vector<float>& signalPower,
                              std::vector<float>& pitchFrequencies)
{
    for (const auto& segment : stackedData)
    {
        std::vector<float> lpc(lpcOrder, 0.0f);
        float power = 0.0f;

        // Compute LPC coefficients using auto-correlation
        computeLpc(segment.data(), segment.size(), lpcOrder, lpc, power);
        lpcCoefficients.push_back(lpc);
        signalPower.push_back(power);

        // Perform pitch detection if enabled
        if (pitchDetectionEnabled)
        {
            float pitchFreq = detectPitch(segment);
            pitchFrequencies.push_back(pitchFreq);
        }
        else
        {
            pitchFrequencies.push_back(0.0f); // Default to unvoiced
        }
    }
}

void LPCProcessor::decodeLPC(const std::vector<std::vector<float>>& lpcCoefficients,
                              const std::vector<float>& signalPower,
                              const std::vector<float>& pitchFrequencies,
                              size_t numSegments,
                              std::vector<std::vector<float>>& synthesizedData)
{
    for (size_t i = 0; i < numSegments; ++i)
    {
        const auto& lpc = lpcCoefficients[i];
        float power = signalPower[i];
        float pitchFreq = pitchFrequencies[i];

        // Generate source signal
        std::vector<float> source(windowSize, 0.0f);
        if (pitchFreq > 0.0f) // Voiced
        {
            int period = static_cast<int>(1.0f / pitchFreq * sampleRate);
            for (size_t j = 0; j < source.size(); j += period)
            {
                source[j] = std::sqrt(period);
            }
        }
        else // Unvoiced
        {
            std::generate(source.begin(), source.end(), []() {
                static std::mt19937 rng{std::random_device{}()};
                static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
                return dist(rng);
            });
        }

        // Filter source through LPC coefficients
        std::vector<float> synthesizedSegment(windowSize, 0.0f);
        for (size_t j = 0; j < source.size(); ++j)
        {
            synthesizedSegment[j] = source[j] * std::sqrt(power);
            for (size_t k = 0; k < lpc.size(); ++k)
            {
                if (j > k)
                {
                    synthesizedSegment[j] -= lpc[k] * synthesizedSegment[j - k - 1];
                }
            }
        }
        synthesizedData.push_back(synthesizedSegment);
    }
}


void LPCProcessor::computeAutocorrelation(const float* input, size_t numSamples, int order, std::vector<float>& autocorrelation)
{
    size_t fftSize = juce::nextPowerOfTwo(numSamples + order);
    std::vector<float> fftBuffer(fftSize * 2, 0.0f);

    // Copy input into FFT buffer
    std::copy(input, input + numSamples, fftBuffer.begin());

    // Perform FFT
    juce::dsp::FFT fft(static_cast<int>(std::log2(fftSize)));
    fft.performRealOnlyForwardTransform(fftBuffer.data());

    // Compute power spectrum (complex multiplication)
    for (size_t i = 0; i < fftSize; ++i)
    {
        float real = fftBuffer[2 * i];
        float imag = fftBuffer[2 * i + 1];
        fftBuffer[2 * i] = real * real + imag * imag; // Magnitude squared
        fftBuffer[2 * i + 1] = 0.0f; // Zero imaginary part
    }

    // Perform inverse FFT
    fft.performRealOnlyInverseTransform(fftBuffer.data());

    // Extract autocorrelation values
    autocorrelation.resize(order + 1);
    for (int i = 0; i <= order; ++i)
    {
        autocorrelation[i] = fftBuffer[i] / numSamples;
    }
}


void LPCProcessor::computeLpc(const float* input, size_t numSamples, int order,
                              std::vector<float>& coefficients, float& power)
{
    if (numSamples <= order)
    {
        DBG("Insufficient samples for LPC computation!");
        coefficients.assign(order, 0.0f);
        power = 0.0f;
        return;
    }

    std::vector<float> autocorrelation(order + 1, 0.0f);

    // Compute autocorrelation O(n^2)
    // for (int lag = 0; lag <= order; ++lag)
    // {
    //     for (size_t i = lag; i < numSamples; ++i)
    //     {
    //         autocorrelation[lag] += input[i] * input[i - lag];
    //     }
    // }

    computeAutocorrelation(input, numSamples, order, autocorrelation);

    power = std::max(autocorrelation[0], 1e-6f); // Prevent divide-by-zero or silent output
    coefficients.assign(order, 0.0f);

    std::vector<float> reflection(order, 0.0f);
    std::vector<float> error(order + 1, power);

    for (int i = 1; i <= order; ++i)
    {
        float sum = 0.0f;
        for (int j = 1; j < i; ++j)
        {
            sum += coefficients[j - 1] * autocorrelation[i - j];
        }

        if (error[i - 1] <= 1e-6f)
        {
            DBG("Numerical instability detected in computeLpc! Clamping reflection coefficient.");
            reflection[i - 1] = 0.0f;
        }
        else
        {
            reflection[i - 1] = std::clamp((autocorrelation[i] - sum) / error[i - 1], -1.0f, 1.0f);
        }

        // Update coefficients
        std::vector<float> tempCoefficients = coefficients;
        for (int j = 0; j < i / 2; ++j)
        {
            coefficients[j] = tempCoefficients[j] + reflection[i - 1] * tempCoefficients[i - j - 2];
            coefficients[i - j - 2] = tempCoefficients[i - j - 2] + reflection[i - 1] * tempCoefficients[j];
        }
        coefficients[i - 1] = reflection[i - 1];

        error[i] = error[i - 1] * (1.0f - reflection[i - 1] * reflection[i - 1]);
    }

    // Debugging
    DBG("Autocorrelation: ");
    for (auto value : autocorrelation)
        DBG(value);

    DBG("Reflection Coefficients: ");
    for (auto value : reflection)
        DBG(value);

    DBG("Final LPC Coefficients: ");
    for (auto coef : coefficients)
        DBG(coef);
}

float LPCProcessor::detectPitch(const std::vector<float>& segment)
{
    // FFT-based pitch detection
    std::vector<float> fftMagnitudes;
    performFFT(segment, fftMagnitudes);

    size_t peakIndex = std::distance(fftMagnitudes.begin(),
                                     std::max_element(fftMagnitudes.begin(), fftMagnitudes.end()));
    return static_cast<float>(sampleRate * peakIndex) / fftMagnitudes.size();
}

void LPCProcessor::performFFT(const std::vector<float>& input, std::vector<float>& magnitudes)
{
    // Implement FFT and populate magnitudes
    // Placeholder using JUCE's FFT
    juce::dsp::FFT fft(static_cast<int>(std::log2(input.size())));
    std::vector<float> fftBuffer(input.size() * 2, 0.0f);
    std::copy(input.begin(), input.end(), fftBuffer.begin());

    fft.performRealOnlyForwardTransform(fftBuffer.data());

    magnitudes.resize(input.size() / 2);
    for (size_t i = 0; i < magnitudes.size(); ++i)
    {
        magnitudes[i] = std::sqrt(fftBuffer[2 * i] * fftBuffer[2 * i] +
                                  fftBuffer[2 * i + 1] * fftBuffer[2 * i + 1]);
    }
}

void LPCProcessor::stackOLA(const float* input, size_t numSamples, std::vector<std::vector<float>>& stackedOutput)
{
    int step = windowSize / 2;
    int numWindows = (static_cast<int>(numSamples) - windowSize) / step + 1;

    for (int i = 0; i < numWindows; ++i)
    {
        std::vector<float> windowedSegment(windowSize);
        std::copy(input + i * step, input + i * step + windowSize, windowedSegment.begin());
        window.multiplyWithWindowingTable(windowedSegment.data(), windowSize); // Apply windowing function
        stackedOutput.push_back(windowedSegment);
    }
}

void LPCProcessor::pressStack(const std::vector<std::vector<float>>& stackedInput, float* output, int outputSize)
{
    int step = windowSize / 2;
    std::fill(output, output + outputSize, 0.0f);

    for (size_t i = 0; i < stackedInput.size(); ++i)
    {
        for (int j = 0; j < windowSize; ++j)
        {
            output[i * step + j] += stackedInput[i][j];
        }
    }
}
