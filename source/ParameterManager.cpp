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

    // Global parameters
    effectParameters["Global"] = {
        apvts.getParameter("IN"),
        apvts.getParameter("OUT"),
        apvts.getParameter("BYPASS"),
        apvts.getParameter("VIS_SMOOTH"),
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
