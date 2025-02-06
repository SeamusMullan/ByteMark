#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include <memory>
#include <random>
#include <vector>

/**
    A simplified LPC-based audio processor:
    - Overlap-add framing
    - Compute LPC via autocorrelation + Levinson-Durbin
    - Optional naive pitch detection
    - Synthesize frames with impulse train or noise
*/
class LPCProcessor
{
public:
    LPCProcessor (int lpcOrder, int windowSize);
    ~LPCProcessor();

    //==========================================================================
    /** Adjusts the analysis/synthesis window size. */
    void setWindowSize (int newSize);

    /** Adjusts the number of LPC coefficients (model order). */
    void setLpcOrder (int newOrder);

    /** Enables or disables pitch detection (voiced/unvoiced). */
    void setPitchDetectionEnabled (bool shouldEnable) { pitchDetectionEnabled = shouldEnable; }

    /** Sets the sample rate used for pitch detection & period calculations. */
    void setTargetSampleRate (double newRate) { sampleRate = newRate; }

    //==========================================================================
    /** Main processing function: 
        1) Overlap-add frames from input
        2) Compute LPC & pitch
        3) Re-synthesize
        4) Overlap-add to output
    */
    void process (const juce::AudioBuffer<float>& inputBuffer,
        juce::AudioBuffer<float>& outputBuffer);

private:
    //==========================================================================
    // Internal helpers:

    /** Break input into overlapping windowed segments. */
    void stackOLA (const float* input, size_t numSamples);

    /** Overlap-add synthesized frames into output. */
    void pressStack (float* output, int outputSize);

    /** For each stacked frame, compute LPC + power (+ pitch if enabled). */
    void encodeLPC();

<<<<<<< Updated upstream
    /** For each frame, create an excitation signal & AR-filter it to get final audio. */
    void decodeLPC();

    /** Autocorrelation -> reflection coefficients -> LPC (Levinson-Durbin). */
    void computeLpc (const float* windowedData, size_t length, std::vector<float>& lpcOut, float& powerOut);
=======
    void decodeLPC (const std::vector<std::vector<float>>& lpcCoefficients,
        const std::vector<float>& signalPower,
        const std::vector<float>& pitchFrequencies,
        int numSegments,
        std::vector<std::vector<float>>& synthesizedData) const;
    static void computeAutocorrelation (const float* input, int numSamples, int order, std::vector<float>& autocorrelation);

    static void computeLpc(const float* input, size_t numSamples, int order,
                    std::vector<float>& coefficients, float& power);
>>>>>>> Stashed changes

    /** Compute the autocorrelation using an FFT-based method (faster for big windowSize). */
    void computeAutocorrelation (const float* data, int length, int order, float* dest);

    /** Naive pitch detection: largest bin in an FFT. */
    double detectPitch (const float* windowedData, size_t length);

    /** Forward FFT, return magnitudes of the half-spectrum. */
    void performFFT (const float* input, size_t length, std::vector<float>& magnitudes);

    /** Update internal buffers based on new windowSize or lpcOrder. */
    void updateInternalBuffers();

    /** Reallocate the FFT object/buffer if needed. */
    void updateFFTObject();

    /** (Optional) Regenerate Hann window. */
    void updateWindowFunction();

    //==========================================================================
    // Internal state:

    int lpcOrder = 0; ///< number of LPC coefficients (model order).
    int windowSize = 0; ///< size of analysis/synthesis window.
    int hopSize = 0; ///< overlap step size (windowSize / 2, for 50% overlap).
    double sampleRate = 44100.0; ///< sample rate for pitch detection.

    bool pitchDetectionEnabled = false;

    // Preallocated data for one process call:
    // Stacked, windowed input frames:
    std::vector<std::vector<float>> stackedData;
    // LPC coefficients for each frame:
    std::vector<std::vector<float>> lpcCoefficients;
    // Power per frame (for amplitude/gain):
    std::vector<float> signalPowers;
    // Pitch frequencies (Hz) per frame:
    std::vector<float> pitchFrequencies;
    // Synthesized frames:
    std::vector<std::vector<float>> synthesizedData;

    // Hann window coefficients:
    std::vector<float> hannWindow;

    // For FFT-based autocorrelation & pitch detection:
    std::unique_ptr<juce::dsp::FFT> fft;
    int fftSize = 0; // actual size used by juce::dsp::FFT
    std::vector<float> fftBuffer;

    // RNG for unvoiced frames:
    std::mt19937 rng { 0xDEADBEEF };
    std::uniform_real_distribution<float> dist { -1.0f, 1.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LPCProcessor)
};
