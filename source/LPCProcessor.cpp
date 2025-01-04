#include "LPCProcessor.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <juce_dsp/juce_dsp.h>

LPCProcessor::LPCProcessor(const int lpcOrder, const int windowSize)
    : lpcOrder(lpcOrder), windowSize(windowSize), pitchDetectionEnabled(false),
      window(windowSize, juce::dsp::WindowingFunction<float>::hann)
{
}

LPCProcessor::~LPCProcessor() = default;

void LPCProcessor::setWindowSize (const int newSize)
{
    windowSize = newSize;
    new (&window) juce::dsp::WindowingFunction<float> (windowSize, juce::dsp::WindowingFunction<float>::hann); // Reconstruct in place
}

void LPCProcessor::process(const juce::AudioBuffer<float>& inputBuffer, juce::AudioBuffer<float>& outputBuffer) const
{
    // this is the processors job, since we want this to be a parameter users can control.
    //const juce::AudioBuffer<float> resampledBuffer = resampleBuffer(inputBuffer, targetSampleRate);

    const int numSamples = inputBuffer.getNumSamples();
    const int numChannels = inputBuffer.getNumChannels();

    // process the input
    for (int channel = 0; channel < numChannels; ++channel)
    {
        const float* input = inputBuffer.getReadPointer(channel);
        float* output = outputBuffer.getWritePointer(channel);

        // Stack the input signal into overlapping windows
        std::vector<std::vector<float>> stackedData;
        stackOLA(input, numSamples, stackedData);

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
                              std::vector<float>& pitchFrequencies) const
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
            const float pitchFreq = static_cast<float>(detectPitch(segment));
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
                              const size_t numSegments,
                              std::vector<std::vector<float>>& synthesizedData) const
{
    for (size_t i = 0; i < numSegments; ++i)
    {
        const auto& lpc = lpcCoefficients[i];
        const float power = signalPower[i];
        const float pitchFreq = pitchFrequencies[i];

        // Generate source signal
        std::vector<float> source(windowSize, 0.0f);
        if (pitchFreq > 0.0f) // Voiced
        {
            const int period = static_cast<int>(1.0f / pitchFreq * sampleRate);
            for (size_t j = 0; j < source.size(); j += period)
            {
                source[j] = static_cast<float>(std::sqrt(period));
            }
        }
        else // Unvoiced
        {
            std::ranges::generate (source, []() {
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

void LPCProcessor::computeAutocorrelation(const float* input, const int numSamples, const int order, std::vector<float>& autocorrelation)
{
    const int fftSize = juce::nextPowerOfTwo(numSamples + order);
    std::vector<float> fftBuffer(fftSize * 2, 0.0f);

    // Copy input into FFT buffer
    std::copy_n(input, numSamples, fftBuffer.begin());

    // Perform FFT
    const juce::dsp::FFT fft(static_cast<int>(std::log2(fftSize)));
    fft.performRealOnlyForwardTransform(fftBuffer.data());

    // Compute power spectrum (complex multiplication)
    for (size_t i = 0; i < fftSize; ++i)
    {
        const float real = fftBuffer[2 * i];
        const float imag = fftBuffer[2 * i + 1];
        fftBuffer[2 * i] = real * real + imag * imag; // Magnitude squared
        fftBuffer[2 * i + 1] = 0.0f; // Zero imaginary part
    }

    // Perform inverse FFT
    fft.performRealOnlyInverseTransform(fftBuffer.data());

    // Extract autocorrelation values
    autocorrelation.resize(order + 1);
    for (int i = 0; i <= order; ++i)
    {
        autocorrelation[i] = fftBuffer[i] / static_cast<float>(numSamples);
    }
}

void LPCProcessor::computeLpc(const float* input, const size_t numSamples, const int order,
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

    computeAutocorrelation (input, static_cast<int>(numSamples), order, autocorrelation);

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
    for (const auto value : autocorrelation)
        DBG(value);

    DBG("Reflection Coefficients: ");
    for (const auto value : reflection)
        DBG(value);

    DBG("Final LPC Coefficients: ");
    for (const auto coef : coefficients)
        DBG(coef);
}

double LPCProcessor::detectPitch(const std::vector<float>& segment) const
{
    // FFT-based pitch detection
    std::vector<float> fftMagnitudes;
    performFFT (segment, fftMagnitudes);

    // iterate through all the bins and select the one with the largest magnitude and assume it's the pitch
    // should work ok for voices but some cases occur where f0 is quieter than other harmonics

    const int peakIndex = static_cast<int>(std::distance(fftMagnitudes.begin(), std::ranges::max_element (fftMagnitudes)));
    return (peakIndex * sampleRate) / static_cast<double>(fftMagnitudes.size());
}

// performs FFT on input vector using JUCE's dsp::FFT
// Populates the magnitudes vector with values
void LPCProcessor::performFFT(const std::vector<float>& input, std::vector<float>& magnitudes)
{
    const juce::dsp::FFT fft(static_cast<int>(std::log2(input.size())));
    std::vector<float> fftBuffer(input.size() * 2, 0.0f);
    std::ranges::copy (input, fftBuffer.begin());

    fft.performRealOnlyForwardTransform(fftBuffer.data());

    magnitudes.resize(input.size() / 2);
    for (size_t i = 0; i < magnitudes.size(); ++i)
    {
        magnitudes[i] = std::sqrt(fftBuffer[2 * i] * fftBuffer[2 * i] +
                                  fftBuffer[2 * i + 1] * fftBuffer[2 * i + 1]);
    }
}

void LPCProcessor::stackOLA(const float* input, const size_t numSamples, std::vector<std::vector<float>>& stackedOutput) const
{
    const int step = windowSize / 2;
    const int numWindows = (static_cast<int>(numSamples) - windowSize) / step + 1;

    for (int i = 0; i < numWindows; ++i)
    {
        std::vector<float> windowedSegment(windowSize);
        std::copy_n(input + i * step, windowSize, windowedSegment.begin());
        window.multiplyWithWindowingTable(windowedSegment.data(), windowSize); // Apply windowing function
        stackedOutput.push_back(windowedSegment);
    }
}

void LPCProcessor::pressStack(const std::vector<std::vector<float>>& stackedInput, float* output, const int outputSize) const
{
    const int step = windowSize / 2;
    std::fill_n(output, outputSize, 0.0f);

    for (size_t i = 0; i < stackedInput.size(); ++i)
    {
        for (int j = 0; j < windowSize; ++j)
        {
            output[i * step + j] += stackedInput[i][j];
        }
    }
}
