#include "SpectrumAnalyzer.h"

SpectrumAnalyzer::SpectrumAnalyzer(PluginProcessor& p)
    : forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann),
      processorRef(p)
{
    fifo.fill(0);
    fftData.fill(0);
    midSpectrum.resize(fftSize / 2, 0.0f);
    sideSpectrum.resize(fftSize / 2, 0.0f);
    startTimerHz(30); // Update at 30 FPS
}


SpectrumAnalyzer::~SpectrumAnalyzer()
{
    stopTimer();
}

void SpectrumAnalyzer::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    drawFrame(g);
}

void SpectrumAnalyzer::resized()
{
    // No need for layout in this component
}

void SpectrumAnalyzer::pushBuffer(const juce::AudioBuffer<float>& buffer)
{
    auto numSamples = buffer.getNumSamples();
    auto writePointer = buffer.getReadPointer(0); // Assuming stereo input

    for (int i = 0; i < numSamples; ++i)
    {
        // Calculate mid and side
        float left = buffer.getSample(0, i);
        float right = buffer.getSample(1, i);

        float mid = (left + right) * 0.5f;
        float side = (left - right) * 0.5f;

        // Push mid signal into FIFO
        if (fifoIndex < fftSize)
        {
            fifo[fifoIndex++] = mid;
        }

        // When FIFO is full, set flag for FFT processing
        if (fifoIndex == fftSize)
        {
            std::copy(fifo.begin(), fifo.end(), fftData.begin());
            window.multiplyWithWindowingTable(fftData.data(), fftSize);
            forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

            // Copy the spectrum data
            midSpectrum.assign(fftData.begin(), fftData.begin() + fftSize / 2);

            // Reset FIFO for side signal
            fifoIndex = 0;

            // Now process side signal
            for (int j = 0; j < fftSize; ++j)
            {
                fifo[j] = side;
            }

            window.multiplyWithWindowingTable(fifo.data(), fftSize);
            forwardFFT.performFrequencyOnlyForwardTransform(fifo.data());

            // Copy the spectrum data
            sideSpectrum.assign(fifo.begin(), fifo.begin() + fftSize / 2);

            nextFFTBlockReady = true;
        }
    }
}

void SpectrumAnalyzer::timerCallback()
{
    DBG("SpectrumAnalyzer timerCallback called");
    juce::AudioBuffer<float> buffer(2, fftSize);

    if (processorRef.fifoQueue.pull(buffer))
    {


        // Process the buffer to compute spectra
        // (Similar to the previous pushBuffer method)
        // Copy the code from pushBuffer here
        auto numSamples = buffer.getNumSamples();

        // Reset the fifoIndex
        fifoIndex = 0;

        for (int i = 0; i < numSamples; ++i)
        {
            float left = buffer.getSample(0, i);
            float right = buffer.getSample(1, i);

            float mid = (left + right) * 0.5f;
            float side = (left - right) * 0.5f;

            // Fill the fifo with mid samples
            if (fifoIndex < fftSize)
            {
                fifo[fifoIndex++] = mid;
            }
        }

        if (fifoIndex == fftSize)
        {
            // Process mid signal
            std::copy(fifo.begin(), fifo.end(), fftData.begin());
            window.multiplyWithWindowingTable(fftData.data(), fftSize);
            forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

            midSpectrum.assign(fftData.begin(), fftData.begin() + fftSize / 2);

            // Now process side signal
            fifoIndex = 0;
            for (int i = 0; i < numSamples; ++i)
            {
                float left = buffer.getSample(0, i);
                float right = buffer.getSample(1, i);

                float side = (left - right) * 0.5f;

                if (fifoIndex < fftSize)
                {
                    fifo[fifoIndex++] = side;
                }
            }

            if (fifoIndex == fftSize)
            {
                std::copy(fifo.begin(), fifo.end(), fftData.begin());
                window.multiplyWithWindowingTable(fftData.data(), fftSize);
                forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

                sideSpectrum.assign(fftData.begin(), fftData.begin() + fftSize / 2);

                nextFFTBlockReady = true;
            }
        }
    } else
    {
        DBG("SpectrumAnalyzer timerCallback called with no FFT");
    }


    if (nextFFTBlockReady)
    {
        applySmoothing();
        nextFFTBlockReady = false;
        repaint();
    }
}

void SpectrumAnalyzer::applySmoothing()
{
    static std::vector<float> previousMidSpectrum(midSpectrum.size(), 0.0f);
    static std::vector<float> previousSideSpectrum(sideSpectrum.size(), 0.0f);

    float smoothingFactor = 0.7f; // Adjust between 0.0f (no smoothing) and 1.0f (full smoothing)

    for (size_t i = 0; i < midSpectrum.size(); ++i)
    {
        midSpectrum[i] = smoothingFactor * previousMidSpectrum[i] + (1.0f - smoothingFactor) * midSpectrum[i];
        previousMidSpectrum[i] = midSpectrum[i];

        sideSpectrum[i] = smoothingFactor * previousSideSpectrum[i] + (1.0f - smoothingFactor) * sideSpectrum[i];
        previousSideSpectrum[i] = sideSpectrum[i];
    }
}


void SpectrumAnalyzer::drawNextFrameOfSpectrum()
{
    // Not needed here as we process in pushBuffer
}
void SpectrumAnalyzer::drawFrame(juce::Graphics& g)
{
    auto width = getLocalBounds().getWidth();
    auto height = getLocalBounds().getHeight();

    int numPoints = fftSize / 2;
    double nyquist = processorRef.getSampleRate() * 0.5;
    double binWidth = nyquist / numPoints;

    juce::Path midPath;
    juce::Path sidePath;

    constexpr float minDecibels = -100.0f;
    constexpr float maxDecibels = 0.0f;
    constexpr float epsilon = 1e-12f;

    for (int i = 1; i < numPoints; ++i)
    {
        double frequency = i * binWidth;
        double x = std::log10(frequency / 20.0) / std::log10(nyquist / 20.0) * width;

        // Mid magnitude
        float midMagnitude = juce::Decibels::gainToDecibels(midSpectrum[i] + epsilon);
        midMagnitude = juce::jlimit(minDecibels, maxDecibels, midMagnitude);
        float yMid = juce::jmap(midMagnitude, minDecibels, maxDecibels, static_cast<float>(height), 0.0f);

        if (i == 1)
            midPath.startNewSubPath(static_cast<float>(x), yMid);
        else
            midPath.lineTo(static_cast<float>(x), yMid);

        // Side magnitude
        float sideMagnitude = juce::Decibels::gainToDecibels(sideSpectrum[i] + epsilon);
        sideMagnitude = juce::jlimit(minDecibels, maxDecibels, sideMagnitude);
        float ySide = juce::jmap(sideMagnitude, minDecibels, maxDecibels, static_cast<float>(height), 0.0f);

        if (i == 1)
            sidePath.startNewSubPath(static_cast<float>(x), ySide);
        else
            sidePath.lineTo(static_cast<float>(x), ySide);
    }

    // Draw mid spectrum (red)
    g.setColour(juce::Colours::red);
    g.strokePath(midPath, juce::PathStrokeType(1.0f));

    // Draw side spectrum (blue)
    g.setColour(juce::Colours::blue);
    g.strokePath(sidePath, juce::PathStrokeType(1.0f));

    // Draw frequency markers
    g.setColour(juce::Colours::grey);
    std::vector<double> frequencies = { 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    for (auto freq : frequencies)
    {
        if (freq < 20.0 || freq > nyquist)
            continue;

        double x = std::log10(freq / 20.0) / std::log10(nyquist / 20.0) * width;
        g.drawVerticalLine(static_cast<int>(x), 0.0f, static_cast<float>(height));
        g.drawText(juce::String(freq) + " Hz", static_cast<int>(x) + 2, height - 20, 50, 20, juce::Justification::left);
    }
}

