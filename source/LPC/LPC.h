//
// Created by seamu on 19/12/2024.
//

#ifndef LPC_H
#define LPC_H

#include <vector>
#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>
#include <numeric>

class LPC {
    std::vector<float> solveLeastSquares(const std::vector<std::vector<float>>& A, const std::vector<float>& b);
    void myLPC(const std::vector<float>& x, int p, std::vector<float>& a, float& g, std::vector<float>& e);
};



#endif //LPC_H
