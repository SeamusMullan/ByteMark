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
        apvts.getParameter("LPC_ORDER"),
        apvts.getParameter("LPC_ALPHA")
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
    lpcOrder = apvts.getRawParameterValue ("LPC_ORDER")->load();
    lpcAlpha = apvts.getRawParameterValue ("LPC_ALPHA")->load();
    pitchDetection = apvts.getRawParameterValue ("PITCH_DETECTION")->load();

}

void ParameterManager::updateEffectParameters()
{
    // Update the effect instances with the new parameter values


}
