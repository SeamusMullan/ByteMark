//
// Created by Seamus Mullan on 26/09/2024.
//

#include "ParameterManager.h"

ParameterManager::ParameterManager(juce::AudioProcessorValueTreeState& state) : apvts(state)
{
    mapParametersToEffects();
}

ParameterManager::~ParameterManager()
{
}

void ParameterManager::mapParametersToEffects()
{
    // Map parameter IDs to effects

    // Compressor parameters
    effectParameters["Compressor"] = {
        apvts.getParameter("COMPRESSOR_THRESHOLD"),
        apvts.getParameter("COMPRESSOR_RATIO"),
        apvts.getParameter("COMPRESSOR_ATTACK"),
        apvts.getParameter("COMPRESSOR_RELEASE"),
        apvts.getParameter("COMPRESSOR_MIX")
    };

    // Pre Filters
    effectParameters["PreFilters"] = {
        apvts.getParameter("LOW_MID_FREQ"),
        apvts.getParameter("MID_HIGH_FREQ")
    };

    // Band Gains
    effectParameters["BandGains"] = {
        apvts.getParameter("LOW_GAIN"),
        apvts.getParameter("MID_GAIN"),
        apvts.getParameter("HIGH_GAIN")
    };

    // Haas Delay
    effectParameters["HaasDelay"] = {
        apvts.getParameter("TIME"),
        apvts.getParameter("HAAS_MIX")
    };

    // Chorus
    effectParameters["Chorus"] = {
        apvts.getParameter("CHORUS_RATE"),
        apvts.getParameter("CHORUS_DEPTH"),
        apvts.getParameter("CHORUS_CENTRE_DELAY"),
        apvts.getParameter("CHORUS_FEEDBACK"),
        apvts.getParameter("CHORUS_MIX")
    };

    // Convolution
    effectParameters["Convolution"] = {
        apvts.getParameter("CONV_MIX")
    };

    // Global parameters
    effectParameters["Global"] = {
        apvts.getParameter("IN"),
        apvts.getParameter("OUT"),
        apvts.getParameter("BYPASS"),
        apvts.getParameter("VIS_SMOOTH")
    };
}

void ParameterManager::categorizeParameters()
{
    // Parameters are already categorized in mapParametersToEffects
    // This function can be expanded if dynamic categorization is needed
}

void ParameterManager::updateParameters()
{
    // Bulk update parameters from the APVTS

    // Global parameters
    inGain = apvts.getRawParameterValue("IN")->load();
    outGain = apvts.getRawParameterValue("OUT")->load();
    bypass = apvts.getRawParameterValue("BYPASS")->load() > 0.5f;
    visSmooth = apvts.getRawParameterValue("VIS_SMOOTH")->load();

    // Pre Filters
    lowMidFreq = apvts.getRawParameterValue("LOW_MID_FREQ")->load();
    midHighFreq = apvts.getRawParameterValue("MID_HIGH_FREQ")->load();

    // Band Gains
    lowBandGain = apvts.getRawParameterValue("LOW_GAIN")->load();
    midBandGain = apvts.getRawParameterValue("MID_GAIN")->load();
    highBandGain = apvts.getRawParameterValue("HIGH_GAIN")->load();

    // Haas Delay
    haasTime = apvts.getRawParameterValue("HAAS_TIME")->load();
    haasMix = apvts.getRawParameterValue("HAAS_MIX")->load();

    // Chorus
    chorusRate = apvts.getRawParameterValue("CHORUS_RATE")->load();
    chorusDepth = apvts.getRawParameterValue("CHORUS_DEPTH")->load();
    chorusCentreDelay = apvts.getRawParameterValue("CHORUS_CENTRE_DELAY")->load();
    chorusFeedback = apvts.getRawParameterValue("CHORUS_FEEDBACK")->load();
    chorusMix = apvts.getRawParameterValue("CHORUS_MIX")->load();

    // Convolution
    convolutionMix = apvts.getRawParameterValue("CONV_MIX")->load();

    // Compressor
    compressorThreshold = apvts.getRawParameterValue("COMPRESSOR_THRESHOLD")->load();
    compressorRatio = apvts.getRawParameterValue("COMPRESSOR_RATIO")->load();
    compressorAttack = apvts.getRawParameterValue("COMPRESSOR_ATTACK")->load();
    compressorRelease = apvts.getRawParameterValue("COMPRESSOR_RELEASE")->load();
    compressorMix = apvts.getRawParameterValue("COMPRESSOR_MIX")->load();
}

void ParameterManager::updateEffectParameters(juce::dsp::Compressor<float>& compressor,
                                             juce::dsp::LinkwitzRileyFilter<float>& preLP,
                                             juce::dsp::LinkwitzRileyFilter<float>& preHP,
                                             juce::dsp::Gain<float>& lowGain,
                                             juce::dsp::Gain<float>& midGain,
                                             juce::dsp::Gain<float>& highGain,
                                             juce::dsp::DelayLine<float>& haasDelay,
                                             juce::dsp::Chorus<float>& chorus,
                                             juce::dsp::Convolution& convolution)
{
    // Update the effect instances with the new parameter values

    // Update compressor
    compressor.setThreshold(compressorThreshold);
    compressor.setRatio(compressorRatio);
    compressor.setAttack(compressorAttack);
    compressor.setRelease(compressorRelease);
    // Compressor mix can be handled in the processBlock when applying the effect

    // Update pre-filters (Linkwitz-Riley filters)
    preLP.setCutoffFrequency(lowMidFreq);
    preHP.setCutoffFrequency(midHighFreq);

    // Update band gains
    lowGain.setGainDecibels(lowBandGain);
    midGain.setGainDecibels(midBandGain);
    highGain.setGainDecibels(highBandGain);

    // Update Haas Delay
    auto haasTimeSamples = haasTime;
    haasDelay.setDelay(haasTime); // Time in samples
    // Haas mix can be handled in the processBlock

    // Update Chorus
    chorus.setRate(chorusRate);
    chorus.setDepth(chorusDepth);
    chorus.setCentreDelay(chorusCentreDelay);
    chorus.setFeedback(chorusFeedback);
    chorus.setMix(chorusMix / 100.0f); // Assuming mix is in percentage

    // Update Convolution
    // For convolution mix, handle in processBlock or set a mix parameter if available

    // Additional effects can be updated similarly
}
