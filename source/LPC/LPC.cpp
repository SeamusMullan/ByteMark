//
// Created by seamu on 19/12/2024.
//

#include "LPC.h"

// Function to solve a least-squares problem via normal equations
std::vector<float> solveLeastSquares(const std::vector<std::vector<float>>& A, const std::vector<float>& b) {
    int rows = A.size();
    int cols = A[0].size();

    // Compute A^T * A (a square matrix)
    std::vector<std::vector<float>> ATA(cols, std::vector<float>(cols, 0.0f));
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < cols; ++j) {
            for (int k = 0; k < rows; ++k) {
                ATA[i][j] += A[k][i] * A[k][j];
            }
        }
    }

    // Compute A^T * b (a vector)
    std::vector<float> ATb(cols, 0.0f);
    for (int i = 0; i < cols; ++i) {
        for (int k = 0; k < rows; ++k) {
            ATb[i] += A[k][i] * b[k];
        }
    }

    // Solve ATA * a = ATb using Gaussian elimination
    std::vector<float> a(cols, 0.0f);
    for (int i = 0; i < cols; ++i) {
        // Normalize the diagonal
        float diag = ATA[i][i];
        for (int j = 0; j < cols; ++j) {
            ATA[i][j] /= diag;
        }
        ATb[i] /= diag;

        // Eliminate below rows
        for (int k = i + 1; k < cols; ++k) {
            float factor = ATA[k][i];
            for (int j = 0; j < cols; ++j) {
                ATA[k][j] -= factor * ATA[i][j];
            }
            ATb[k] -= factor * ATb[i];
        }
    }

    // Back substitution
    for (int i = cols - 1; i >= 0; --i) {
        a[i] = ATb[i];
        for (int j = i + 1; j < cols; ++j) {
            a[i] -= ATA[i][j] * a[j];
        }
    }

    return a;
}


//  myLPC(signal, order, coefficients, variance, errorSignal);
void myLPC(const std::vector<float>& x, int p, std::vector<float>& a, float& g, std::vector<float>& e) {

    int N = static_cast<int>(x.size());

    // Form b: x(2:N)
    std::vector<float> b(x.begin() + 1, x.end());

    // Form xz: signal padded with zeros for circular shifting
    std::vector<float> xz(x);
    xz.resize(N + p, 0.0f); // Padding with zeros

    // Form the A matrix as a vector of vectors
    std::vector<std::vector<float>> A(N - 1, std::vector<float>(p, 0.0f));
    for (int i = 0; i < p; ++i) {
        for (int j = 0; j < N - 1; ++j) {
            A[j][i] = xz[j + i]; // Circular shift effect
        }
    }

    // Solve for a using least-squares
    a = solveLeastSquares(A, b);

    // Calculate error signal: e = b - A * a
    e.resize(N - 1, 0.0f);
    for (int i = 0; i < N - 1; ++i) {
        float predicted = 0.0f;
        for (int j = 0; j < p; ++j) {
            predicted += A[i][j] * a[j];
        }
        e[i] = b[i] - predicted;
    }

    // Calculate variance of errors: g = var(e)
    float meanError = std::accumulate(e.begin(), e.end(), 0.0f) / e.size();
    g = 0.0f;
    for (const auto& err : e) {
        g += (err - meanError) * (err - meanError);
    }
    g /= e.size();
}
