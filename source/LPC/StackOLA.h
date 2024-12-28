//
// Created by seamu on 19/12/2024.
//

#ifndef STACKOLA_H
#define STACKOLA_H


#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

class StackOLA {
    juce::AudioBuffer<float> stackOLA(const std::vector<float>& x, const std::vector<float>& w);
};



#endif //STACKOLA_H
