//
// Created by Seamus Mullan on 26/09/2024.
//

#ifndef PARAMETERMANAGER_H
#define PARAMETERMANAGER_H


#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <unordered_map>
#include <string>
#include <vector>

class ParameterManager
{
public:
    ParameterManager(juce::AudioProcessorValueTreeState& apvts);
    ~ParameterManager();

    // Bulk update parameters from the APVTS
    void updateParameters();

    // Categorize parameters based on the effects they are relevant to
    void categorizeParameters();

    // Update relevant instances of effects (e.g., reverb)
    void updateEffectParameters(juce::dsp::Compressor<float>& compressor,
                                juce::dsp::LinkwitzRileyFilter<float>& preLP,
                                juce::dsp::LinkwitzRileyFilter<float>& preHP,
                                juce::dsp::Gain<float>& lowGain,
                                juce::dsp::Gain<float>& midGain,
                                juce::dsp::Gain<float>& highGain,
                                juce::dsp::DelayLine<float>& haasDelay,
                                juce::dsp::Chorus<float>& chorus,
                                juce::dsp::Convolution& convolution);

    // Accessors for parameters
    bool isBypassed() const { return bypass; }
    float getInGain() const { return inGain; }
    float getOutGain() const { return outGain; }
    float getVisSmooth() const { return visSmooth; }

    // ... (additional accessors if needed)

private:
    juce::AudioProcessorValueTreeState& apvts;

    // Store parameters categorized by effect
    std::unordered_map<std::string, std::vector<juce::RangedAudioParameter*>> effectParameters;

    // Internal function to map parameter IDs to effects
    void mapParametersToEffects();

    // Parameters
    float inGain = 0.0f;
    float outGain = 0.0f;
    bool bypass = false;
    float visSmooth = 0.69f;

    // Pre Filters
    float lowMidFreq = 8000.0f;
    float midHighFreq = 15000.0f;

    // Band Gains
    float lowBandGain = 0.0f;
    float midBandGain = 0.0f;
    float highBandGain = 0.0f;

    // Haas Delay
    float haasTime = 20.0f;
    float haasMix = 50.0f;

    // Chorus
    float chorusRate = 1.0f;
    float chorusDepth = 0.5f;
    float chorusCentreDelay = 10.0f;
    float chorusFeedback = 0.0f;
    float chorusMix = 50.0f;

    // Convolution
    float convolutionMix = 50.0f;

    // Compressor
    float compressorThreshold = -24.0f;
    float compressorRatio = 4.0f;
    float compressorAttack = 10.0f;
    float compressorRelease = 100.0f;
    float compressorMix = 50.0f;
};


#endif //PARAMETERMANAGER_H
