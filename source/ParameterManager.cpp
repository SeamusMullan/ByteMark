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
        apvts.getParameter ("BASS_MONO"),
        apvts.getParameter ("BASS_MONO_FREQ"),
        apvts.getParameter("BYPASS"),
        apvts.getParameter("VIS_SMOOTH"),
        apvts.getParameter("LOW_SOLO"),
        apvts.getParameter("MID_SOLO"),
        apvts.getParameter("HIGH_SOLO")
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


}

void ParameterManager::updateEffectParameters()
{
    // Update the effect instances with the new parameter values


}
