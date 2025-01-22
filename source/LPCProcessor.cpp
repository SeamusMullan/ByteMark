#include "LPCProcessor.h"
#include <algorithm>
#include <cmath>

LPCProcessor::LPCProcessor (int lpcOrder_, int windowSize_)
    : lpcOrder (lpcOrder_),
      windowSize (windowSize_)
{
    hopSize = windowSize / 2;

    updateFFTObject();
    updateWindowFunction();
    updateInternalBuffers();
}

LPCProcessor::~LPCProcessor() = default;

//==============================================================================
void LPCProcessor::setWindowSize (int newSize)
{
    if (newSize <= 0 || newSize == windowSize)
        return;

    windowSize = newSize;
    hopSize = windowSize / 2;

    updateFFTObject();
    updateWindowFunction();
    updateInternalBuffers();
}

void LPCProcessor::setLpcOrder (int newOrder)
{
    if (newOrder <= 0 || newOrder == lpcOrder)
        return;

    lpcOrder = newOrder;
    // Realloc or clear buffers that depend on lpcOrder
    updateInternalBuffers();
}

//==============================================================================
void LPCProcessor::process (const juce::AudioBuffer<float>& inputBuffer,
    juce::AudioBuffer<float>& outputBuffer)
{
    jassert (inputBuffer.getNumChannels() == outputBuffer.getNumChannels());
    const int numChannels = inputBuffer.getNumChannels();
    const int numSamples = inputBuffer.getNumSamples();

    // Clear output first
    outputBuffer.clear();

    // Process each channel independently:
    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* inPtr = inputBuffer.getReadPointer (ch);
        float* outPtr = outputBuffer.getWritePointer (ch);

        // 1) stackOLA: partition + window the input
        stackOLA (inPtr, (size_t) numSamples);

        // 2) encodeLPC: compute LPC + power (+ pitch if enabled)
        encodeLPC();

        // 3) decodeLPC: create excitation & filter with LPC
        decodeLPC();

        // 4) pressStack: overlap-add the frames to outPtr
        pressStack (outPtr, numSamples);
    }
}

//==============================================================================
void LPCProcessor::stackOLA (const float* input, size_t numSamples)
{
    stackedData.clear();

    // If there's not enough data for even 1 window, just bail
    if (windowSize <= 0 || hopSize <= 0 || numSamples < (size_t) windowSize)
        return;

    const int numWindows = ((int) numSamples - windowSize) / hopSize + 1;
    stackedData.reserve ((size_t) numWindows);

    for (int i = 0; i < numWindows; ++i)
    {
        std::vector<float> segment ((size_t) windowSize, 0.0f);
        const size_t startIdx = i * (size_t) hopSize;

        // Copy samples from input into segment
        std::copy_n (input + startIdx, (size_t) windowSize, segment.begin());

        // Multiply by our Hann window
        for (int n = 0; n < windowSize; ++n)
            segment[(size_t) n] *= hannWindow[(size_t) n];

        stackedData.push_back (std::move (segment));
    }
}

//==============================================================================
void LPCProcessor::pressStack (float* output, int outputSize)
{
    if (stackedData.size() != synthesizedData.size())
        return; // mismatch

    for (size_t i = 0; i < synthesizedData.size(); ++i)
    {
        const auto& frame = synthesizedData[i];
        const size_t startIdx = i * (size_t) hopSize;

        for (size_t n = 0; n < (size_t) windowSize; ++n)
        {
            if (startIdx + n < (size_t) outputSize)
                output[startIdx + n] += frame[n];
        }
    }
}

//==============================================================================
void LPCProcessor::encodeLPC()
{
    lpcCoefficients.clear();
    signalPowers.clear();
    pitchFrequencies.clear();

    lpcCoefficients.reserve (stackedData.size());
    signalPowers.reserve (stackedData.size());
    pitchFrequencies.reserve (stackedData.size());

    for (auto& frame : stackedData)
    {
        std::vector<float> lpc ((size_t) lpcOrder, 0.0f);
        float power = 0.0f;

        computeLpc (frame.data(), frame.size(), lpc, power);

        lpcCoefficients.push_back (std::move (lpc));
        signalPowers.push_back (power);

        if (pitchDetectionEnabled)
        {
            const double pitchHz = detectPitch (frame.data(), frame.size());
            pitchFrequencies.push_back ((float) pitchHz);
        }
        else
        {
            pitchFrequencies.push_back (0.0f); // unvoiced
        }
    }
}

//==============================================================================
void LPCProcessor::decodeLPC()
{
    synthesizedData.clear();
    synthesizedData.reserve (stackedData.size());

    for (size_t i = 0; i < stackedData.size(); ++i)
    {
        const auto& coefs = lpcCoefficients[i];
        const float power = signalPowers[i];
        const float pitch = pitchFrequencies[i];

        // Create the excitation signal
        std::vector<float> source ((size_t) windowSize, 0.0f);

        if (pitch > 0.0f)
        {
            // Voiced -> impulse train
            int period = (sampleRate > 0.0 && pitch > 1.0)
                             ? (int) std::floor (sampleRate / pitch + 0.5)
                             : windowSize; // fallback

            for (int idx = 0; idx < windowSize; idx += period)
            {
                if (idx < windowSize)
                    source[(size_t) idx] = std::sqrt ((float) period);
            }
        }
        else
        {
            // Unvoiced -> white noise
            for (auto& x : source)
                x = dist (rng);
        }

        // AR filter: out[n] = gain * in[n] - sum(a[k]*out[n-(k+1)])
        std::vector<float> synth ((size_t) windowSize, 0.0f);
        float gain = std::sqrt (std::max (power, 1e-8f));

        for (size_t n = 0; n < (size_t) windowSize; ++n)
        {
            float y = source[n] * gain;

            for (int k = 0; k < lpcOrder; ++k)
            {
                if (n > (size_t) k)
                    y -= coefs[(size_t) k] * synth[n - (k + 1)];
            }
            synth[n] = y;
        }

        synthesizedData.push_back (std::move (synth));
    }
}

//==============================================================================
void LPCProcessor::computeLpc (const float* windowedData, size_t length, std::vector<float>& lpcOut, float& powerOut)
{
    if (length < (size_t) (lpcOrder + 1))
    {
        std::fill (lpcOut.begin(), lpcOut.end(), 0.0f);
        powerOut = 0.0f;
        return;
    }

    // Autocorrelation for lags 0..lpcOrder
    std::vector<float> autocorr ((size_t) lpcOrder + 1, 0.0f);
    computeAutocorrelation (windowedData, (int) length, lpcOrder, autocorr.data());

    // The "energy" (R[0]) can be our initial power estimate
    powerOut = std::max (autocorr[0], 1e-8f);

    // Levinson-Durbin
    std::fill (lpcOut.begin(), lpcOut.end(), 0.0f);
    std::vector<float> error ((size_t) lpcOrder + 1, powerOut);

    for (int i = 1; i <= lpcOrder; ++i)
    {
        float acc = 0.0f;
        for (int j = 1; j < i; ++j)
        {
            acc += lpcOut[(size_t) (j - 1)] * autocorr[(size_t) (i - j)];
        }

        float ref = 0.0f;
        const float denom = error[(size_t) i - 1];
        if (std::fabs (denom) > 1e-12f)
            ref = (autocorr[(size_t) i] - acc) / denom;

        // clamp reflection
        if (ref > 0.999f)
            ref = 0.999f;
        else if (ref < -0.999f)
            ref = -0.999f;

        // update lpcOut
        auto oldCoefs = lpcOut; // copy
        for (int k = 0; k < (i - 1) / 2; ++k)
        {
            float c1 = oldCoefs[(size_t) k];
            float c2 = oldCoefs[(size_t) (i - k - 2)];
            lpcOut[(size_t) k] = c1 + ref * c2;
            lpcOut[(size_t) (i - k - 2)] = c2 + ref * c1;
        }
        lpcOut[(size_t) i - 1] = ref;

        // update error
        error[(size_t) i] = error[(size_t) i - 1] * (1.0f - ref * ref);
        if (error[(size_t) i] < 1e-12f)
            error[(size_t) i] = 1e-12f; // avoid blow-ups
    }
}

//==============================================================================
void LPCProcessor::computeAutocorrelation (const float* data, int length, int order, float* dest)
{
    // 1) Zero out the fftBuffer
    std::fill (fftBuffer.begin(), fftBuffer.end(), 0.0f);

    // 2) Copy 'length' samples into the real part of fftBuffer
    for (int i = 0; i < length; ++i)
        fftBuffer[(size_t) i] = data[i];

    // 3) Forward FFT
    fft->performRealOnlyForwardTransform (fftBuffer.data());

    // 4) Compute power spectrum => real part = magnitude^2, imag part = 0
    for (int i = 0; i < fftSize; ++i)
    {
        float re = fftBuffer[2 * (size_t) i];
        float im = fftBuffer[2 * (size_t) i + 1];
        float mag = re * re + im * im;

        fftBuffer[2 * (size_t) i] = mag;
        fftBuffer[2 * (size_t) i + 1] = 0.0f;
    }

    // 5) Inverse FFT => time-domain autocorrelation in fftBuffer
    fft->performRealOnlyInverseTransform (fftBuffer.data());

    // 6) Normalize by fftSize (NOT by 'length'), then copy lags [0..order] to 'dest'
    for (int k = 0; k <= order; ++k)
        dest[k] = fftBuffer[(size_t) k] / (float) fftSize;
}


//==============================================================================
double LPCProcessor::detectPitch (const float* windowedData, size_t length)
{
    if (sampleRate <= 0.0 || length == 0)
        return 0.0;

    std::vector<float> mags;
    performFFT (windowedData, length, mags);
    if (mags.empty())
        return 0.0;

    // largest bin
    auto it = std::max_element (mags.begin(), mags.end());
    int idx = (int) std::distance (mags.begin(), it);

    // freq = (Fs * binIndex) / (N * 2)  since we only have half the bins in mags
    double freq = (sampleRate * idx) / (mags.size() * 2.0);
    return freq;
}

//==============================================================================
void LPCProcessor::performFFT (const float* input, size_t length, std::vector<float>& magnitudes)
{
    // zero fftBuffer
    std::fill (fftBuffer.begin(), fftBuffer.end(), 0.0f);

    // copy input
    jassert (length <= (size_t) fftSize); // must not exceed our allocated size
    for (size_t i = 0; i < length; ++i)
        fftBuffer[i] = input[i];

    // forward transform
    fft->performRealOnlyForwardTransform (fftBuffer.data());

    // half-spectrum magnitude
    size_t halfSize = length / 2;
    magnitudes.resize (halfSize);

    for (size_t i = 0; i < halfSize; ++i)
    {
        float re = fftBuffer[2 * i];
        float im = fftBuffer[2 * i + 1];
        magnitudes[i] = std::sqrt (re * re + im * im);
    }
}

//==============================================================================
void LPCProcessor::updateInternalBuffers()
{
    // Clear vectors and preallocate a typical capacity
    stackedData.clear();
    lpcCoefficients.clear();
    signalPowers.clear();
    pitchFrequencies.clear();
    synthesizedData.clear();

    stackedData.reserve (128);
    lpcCoefficients.reserve (128);
    signalPowers.reserve (128);
    pitchFrequencies.reserve (128);
    synthesizedData.reserve (128);

    // Also re-zero FFT buffer
    fftBuffer.assign ((size_t) fftSize * 2, 0.0f);
}

void LPCProcessor::updateFFTObject()
{
    // We need an FFT size that can handle 2*windowSize
    fftSize = juce::nextPowerOfTwo (2 * windowSize);

    fft = std::make_unique<juce::dsp::FFT> ((int) std::log2 ((double) fftSize));
    fftBuffer.assign ((size_t) fftSize * 2, 0.0f);
}

void LPCProcessor::updateWindowFunction()
{
    hannWindow.resize ((size_t) windowSize, 1.0f);

    // Fill with a Hann window: w[n] = 0.5 * (1 - cos(2*pi*n/(N-1)))
    if (windowSize > 1)
    {
        for (int n = 0; n < windowSize; ++n)
        {
            float ratio = (float) n / (float) (windowSize - 1);
            float w = 0.5f * (1.0f - std::cos (2.0f * juce::MathConstants<float>::pi * ratio));
            hannWindow[(size_t) n] = w;
        }
    }
}
