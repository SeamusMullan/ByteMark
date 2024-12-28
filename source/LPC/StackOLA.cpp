//
// Created by seamu on 19/12/2024.
//

/*

% Stacks a signal into overlap-add chunks.
%
% x - a single channel signal
% w - the window function
%
% X - the overlap-add stack
%
function X = stackOLA(x, w)

n = length(x);
nw = length(w);
step = floor(nw*0.5);

count = floor((n-nw)/step) + 1;

X = zeros(nw, count);

for i = 1:count,
    X(:, i) = w .* x( (1:nw) + (i-1)*step );
end

*/

#include "StackOLA.h"


juce::AudioBuffer<float> stackOLA(const std::vector<float>& x, const std::vector<float>& w) {
    int n = static_cast<int>(x.size());
    int nw = static_cast<int>(w.size());
    int step = static_cast<int>(std::floor(nw * 0.5));

    // Calculate the number of chunks
    int count = static_cast<int>(std::floor((n - nw) / static_cast<float>(step))) + 1;

    // Initialize the AudioBuffer: count channels, nw samples per channel
    juce::AudioBuffer<float> X;
    X.setSize(count, nw);
    X.clear();

    for (int i = 0; i < count; ++i) {
        int start = i * step;

        // Fill the current channel with the overlap-add chunk
        for (int j = 0; j < nw; ++j) {
            float windowedSample = w[j] * x[start + j];
            X.setSample(i, j, windowedSample);
        }
    }

    return X;
}
