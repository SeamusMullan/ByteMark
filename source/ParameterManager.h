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
    void updateEffectParameters();

    // Accessors for parameters
    bool isBypassed() const { return bypass; }
    float getInGain() const { return inGain; }
    float getOutGain() const { return outGain; }
    float getVisSmooth() const { return visSmooth; }
    bool getPitchDetection() const { return pitchDetection; }

    // Parameters
    float inGain = 0.0f;
    float outGain = 0.0f;
    bool bypass = false;
    float visSmooth = 0.69f;
    int lpcOrder = 10;
    float lpcAlpha = 0.5f;
    bool pitchDetection = false;
    float lpcSampleRate = 44100.0f;
    float mix = 1.0f;

private:
    juce::AudioProcessorValueTreeState& apvts;

    // Store parameters categorized by effect
    std::unordered_map<std::string, std::vector<juce::RangedAudioParameter*>> effectParameters;

    // Internal function to map parameter IDs to effects
    void mapParametersToEffects();



};


#endif //PARAMETERMANAGER_H
