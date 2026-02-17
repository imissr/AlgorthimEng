// threshold_proposed.cpp  (UNIT-CONSISTENT HYBRID)
//
// Goal:
// - Keep mW, mg, sW in RAW scale [0..maxv] (so behavior stays “strong” like your RAW version)
// - Still use sigmaAdaptive in [0,1] BUT convert it back into RAW units before combining with sW
//
// Key change:
//   b = (sigmaAdaptive * sigmaMax) + sW;   // RAW + RAW  (unit-consistent)
//
// Everything else follows Eq (4)(5)(6) from the paper, but computed on RAW intensities.

#include "threshold_proposed.h"

#include <vector>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <cstddef>
#include <iostream>
#include <iomanip>
#include <cstdint>

#include "util/clamp.h"

// Integral-image rectangle sum (inclusive coords x0..x1, y0..y1)
static inline long double rectSum(const std::vector<long double>& integ,
                                  int w,
                                  int x0, int y0, int x1, int y1) {
    auto atI = [&](int x, int y) -> long double {
        return integ[(std::size_t)y * (std::size_t)(w + 1) + (std::size_t)x];
    };
    return atI(x1 + 1, y1 + 1) - atI(x0, y1 + 1) - atI(x1 + 1, y0) + atI(x0, y0);
}

GrayImage threshold_proposed::binarize(const GrayImage& in, int r) {
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0) {
        throw std::runtime_error("threshold_proposed::binarize: invalid image");
    }
    if (r <= 0) {
        throw std::runtime_error("threshold_proposed::binarize: r must be > 0");
    }

    const int w = in.width;
    const int h = in.height;
    const int maxv = in.maxval;
    const std::size_t N = (std::size_t)w * (std::size_t)h;

    if (in.data.size() != N) {
        throw std::runtime_error("threshold_proposed::binarize: data size mismatch");
    }

    // Integral images over RAW values in [0..maxv]
    std::vector<long double> integ((std::size_t)(w + 1) * (std::size_t)(h + 1), 0.0L);
    std::vector<long double> integSq((std::size_t)(w + 1) * (std::size_t)(h + 1), 0.0L);

    auto idx = [&](int x, int y) -> std::size_t {
        return (std::size_t)y * (std::size_t)(w + 1) + (std::size_t)x;
    };

    for (int y = 1; y <= h; ++y) {
        long double rowSum = 0.0L;
        long double rowSumSq = 0.0L;

        for (int x = 1; x <= w; ++x) {
            const int iv = clampInt(in.at(x - 1, y - 1), 0, maxv);
            const long double p = (long double)iv; // RAW

            rowSum += p;
            rowSumSq += p * p;

            integ[idx(x, y)]   = integ[idx(x, y - 1)]   + rowSum;
            integSq[idx(x, y)] = integSq[idx(x, y - 1)] + rowSumSq;
        }
    }

    // Global mean mg (RAW)
    const long double sumAll = rectSum(integ, w, 0, 0, w - 1, h - 1);
    const long double mg = sumAll / (long double)N;

    std::cerr << std::fixed << std::setprecision(12);
    std::cerr << "\n=== Proposed Threshold (Unit-consistent HYBRID) Sanity Check ===\n";
    std::cerr << "Image: " << w << "x" << h << "  maxval=" << maxv
              << "  r=" << r << " (window=" << (2 * r + 1) << "x" << (2 * r + 1) << ")\n";
    std::cerr << "mg (RAW)            = " << (double)mg << "\n";

    // First pass: compute sigmaW + sigmaMin/sigmaMax across all windows
    std::vector<long double> sigmaW(N, 0.0L);

    long double sigmaMin = std::numeric_limits<long double>::max();
    long double sigmaMax = std::numeric_limits<long double>::lowest();

#pragma omp parallel for default(none) shared(integ, integSq, sigmaW, w, h, r) reduction(min:sigmaMin) reduction(max:sigmaMax)
    for (int y = 0; y < h; ++y) {
        const int y0 = (y - r < 0) ? 0 : (y - r);
        const int y1 = (y + r >= h) ? (h - 1) : (y + r);

        for (int x = 0; x < w; ++x) {
            const int x0 = (x - r < 0) ? 0 : (x - r);
            const int x1 = (x + r >= w) ? (w - 1) : (x + r);
            const int area = (x1 - x0 + 1) * (y1 - y0 + 1);

            const long double sum   = rectSum(integ,   w, x0, y0, x1, y1);
            const long double sumSq = rectSum(integSq, w, x0, y0, x1, y1);

            const long double mW = sum / (long double)area;
            const long double secondMoment = sumSq / (long double)area;

            long double var = secondMoment - mW * mW;
            if (var < 0.0L) var = 0.0L;

            const long double sW = sqrtl(var);

            const std::size_t i = (std::size_t)y * (std::size_t)w + (std::size_t)x;
            sigmaW[i] = sW;

            if (sW < sigmaMin) sigmaMin = sW;
            if (sW > sigmaMax) sigmaMax = sW;
        }
    }

    const long double EPS = 1e-9L;
    const long double sigmaRange = sigmaMax - sigmaMin;

    std::cerr << "sigmaMin (RAW)      = " << (double)sigmaMin << "\n";
    std::cerr << "sigmaMax (RAW)      = " << (double)sigmaMax << "\n";
    std::cerr << "sigmaRange (RAW)    = " << (double)sigmaRange << "\n";

    GrayImage out = in;
    out.data.assign(N, 0);

    // Stats
    long double T_min = std::numeric_limits<long double>::max();
    long double T_max = std::numeric_limits<long double>::lowest();
    long double sa_min = std::numeric_limits<long double>::max();
    long double sa_max = std::numeric_limits<long double>::lowest();
    std::uint64_t blackCount = 0;
    std::uint64_t whiteCount = 0;
    std::uint64_t denomClampA = 0;
    std::uint64_t denomClampB = 0;
    std::uint64_t ToutOfRange = 0;

#pragma omp parallel for default(none) \
shared(in, out, integ, sigmaW, w, h, r, maxv, mg, sigmaMin, sigmaMax, sigmaRange) \
firstprivate(EPS) \
reduction(min:T_min, sa_min) reduction(max:T_max, sa_max) \
reduction(+:blackCount, whiteCount, denomClampA, denomClampB, ToutOfRange)
    for (int y = 0; y < h; ++y) {
        const int y0 = (y - r < 0) ? 0 : (y - r);
        const int y1 = (y + r >= h) ? (h - 1) : (y + r);

        for (int x = 0; x < w; ++x) {
            const int x0 = (x - r < 0) ? 0 : (x - r);
            const int x1 = (x + r >= w) ? (w - 1) : (x + r);
            const int area = (x1 - x0 + 1) * (y1 - y0 + 1);

            const long double sum = rectSum(integ, w, x0, y0, x1, y1);
            const long double mW  = sum / (long double)area;

            const std::size_t i = (std::size_t)y * (std::size_t)w + (std::size_t)x;
            const long double sW = sigmaW[i];

            // Eq (5): sigmaAdaptive in [0,1]
            long double sigmaAdaptive = 0.0L;
            if (sigmaRange > EPS) {
                sigmaAdaptive = (sW - sigmaMin) / sigmaRange;
                if (sigmaAdaptive < 0.0L) sigmaAdaptive = 0.0L;
                if (sigmaAdaptive > 1.0L) sigmaAdaptive = 1.0L;
            }

            // Unit-consistent hybrid:
            // Convert sigmaAdaptive back to RAW "sigma-like" magnitude before adding to sW.
            // (0..1) * sigmaMax gives a RAW-scale term.
            const long double sigmaAdaptiveRaw = sigmaAdaptive * sigmaMax;

            // Eq (4): T = mW - (mW^2 - σW) / ((mg + σW) * (σAdaptive + σW))
            long double a = mg + sW;                       // RAW
            long double b = sigmaAdaptiveRaw + sW;         // RAW (unit-consistent)
            if (a <= EPS) { a = EPS; denomClampA++; }
            if (b <= EPS) { b = EPS; denomClampB++; }

            long double T = mW - (((mW * mW) - sW) / (a * b)); // RAW

            if (T < 0.0L || T > (long double)maxv) ToutOfRange++;

            if (T < 0.0L) T = 0.0L;
            if (T > (long double)maxv) T = (long double)maxv;

            const int iv = clampInt(in.at(x, y), 0, maxv);
            const long double p = (long double)iv;

            out.at(x, y) = (p < T) ? 0 : maxv;

            if (p < T) blackCount++; else whiteCount++;

            if (T < T_min) T_min = T;
            if (T > T_max) T_max = T;
            if (sigmaAdaptive < sa_min) sa_min = sigmaAdaptive;
            if (sigmaAdaptive > sa_max) sa_max = sigmaAdaptive;
        }
    }

    std::cerr << "sigmaAdaptive min/max = " << (double)sa_min << " / " << (double)sa_max << "\n";
    std::cerr << "T min/max (RAW)       = " << (double)T_min << " / " << (double)T_max << "\n";
    std::cerr << "denom clamp (a)       = " << denomClampA << "\n";
    std::cerr << "denom clamp (b)       = " << denomClampB << "\n";
    std::cerr << "T out-of-range cnt    = " << ToutOfRange << "\n";
    std::cerr << "black pixels          = " << blackCount << "\n";
    std::cerr << "white pixels          = " << whiteCount << "\n";
    std::cerr << "black ratio           = "
              << ((blackCount + whiteCount) ? (double)blackCount / (double)(blackCount + whiteCount) : 0.0)
              << "\n";
    std::cerr << "===============================================\n\n";

    return out;
}
