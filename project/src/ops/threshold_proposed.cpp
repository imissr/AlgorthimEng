// threshold_proposed.cpp  (RAW-intensity stabilized version)
// Computes Eq. (4)(5)(6) on RAW intensities [0..maxval] instead of [0..1].
// This prevents sigma values from becoming extremely tiny, which makes the
// denominator in Eq. (4) collapse and forces T negative (=> clamped to 0 => all white).

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

static inline long double rectSum(const std::vector<long double>& integ,
                                  int w, int /*h*/,
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

    const long double sumAll = rectSum(integ, w, h, 0, 0, w - 1, h - 1);
    const long double mg = sumAll / (long double)N; // RAW mean

    // First pass: sigmaW + sigmaMinNonZero/sigmaMax
    std::vector<long double> sigmaW(N, 0.0L);

    const long double INF = std::numeric_limits<long double>::max();
    long double sigmaMinNonZero = INF;
    long double sigmaMax = std::numeric_limits<long double>::lowest();

    // EPS in RAW scale: 1e-12 is too tiny; use something meaningful for RAW.
    // 1e-6 is still “almost zero” but avoids denoms that go to 0.
    const long double EPS = 1e-6L;

#pragma omp parallel for default(none) shared(integ, integSq, sigmaW, w, h, r) reduction(min:sigmaMinNonZero) reduction(max:sigmaMax)
    for (int y = 0; y < h; ++y) {
        const int y0 = (y - r < 0) ? 0 : (y - r);
        const int y1 = (y + r >= h) ? (h - 1) : (y + r);

        for (int x = 0; x < w; ++x) {
            const int x0 = (x - r < 0) ? 0 : (x - r);
            const int x1 = (x + r >= w) ? (w - 1) : (x + r);
            const int area = (x1 - x0 + 1) * (y1 - y0 + 1);

            const long double sum   = rectSum(integ,   w, h, x0, y0, x1, y1);
            const long double sumSq = rectSum(integSq, w, h, x0, y0, x1, y1);

            const long double mW = sum / (long double)area;
            const long double secondMoment = sumSq / (long double)area;

            long double var = secondMoment - mW * mW;
            if (var < 0.0L) var = 0.0L;

            const long double sW = sqrtl(var);

            const std::size_t i = (std::size_t)y * (std::size_t)w + (std::size_t)x;
            sigmaW[i] = sW;

            if (sW > EPS && sW < sigmaMinNonZero) sigmaMinNonZero = sW;
            if (sW > sigmaMax) sigmaMax = sW;
        }
    }

    if (sigmaMinNonZero == INF) sigmaMinNonZero = 0.0L;
    const long double sigmaRange = sigmaMax - sigmaMinNonZero;

    // --- SANITY CHECK (after first pass) ---
    long double sigmaW_min = std::numeric_limits<long double>::max();
    long double sigmaW_max = std::numeric_limits<long double>::lowest();

#pragma omp parallel for default(none) shared(sigmaW, w, h) reduction(min:sigmaW_min) reduction(max:sigmaW_max)
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const std::size_t i = (std::size_t)y * (std::size_t)w + (std::size_t)x;
            const long double s = sigmaW[i];
            if (s < sigmaW_min) sigmaW_min = s;
            if (s > sigmaW_max) sigmaW_max = s;
        }
    }

    std::cerr << std::fixed << std::setprecision(12);
    std::cerr << "\n=== Proposed Threshold (RAW): Sanity Check ===\n";
    std::cerr << "Image: " << w << "x" << h << "  maxval=" << maxv << "  r=" << r
              << " (window=" << (2 * r + 1) << "x" << (2 * r + 1) << ")\n";
    std::cerr << "mg (RAW)           = " << (double)mg << "\n";
    std::cerr << "sigmaMinNonZero    = " << (double)sigmaMinNonZero << "\n";
    std::cerr << "sigmaMax           = " << (double)sigmaMax << "\n";
    std::cerr << "sigmaRange         = " << (double)sigmaRange << "\n";
    std::cerr << "sigmaW min/max     = " << (double)sigmaW_min << " / " << (double)sigmaW_max << "\n";

    GrayImage out = in;
    out.data.assign(N, 0);

    // Second pass
    long double T_min = std::numeric_limits<long double>::max();
    long double T_max = std::numeric_limits<long double>::lowest();
    std::uint64_t blackCount = 0;
    std::uint64_t whiteCount = 0;
    std::uint64_t clampedLo = 0;
    std::uint64_t clampedHi = 0;

#pragma omp parallel for default(none) \
shared(in, out, integ, sigmaW, w, h, r, maxv, mg, sigmaMinNonZero) \
firstprivate(sigmaRange, EPS) \
reduction(min:T_min) reduction(max:T_max) \
reduction(+:blackCount, whiteCount, clampedLo, clampedHi)
    for (int y = 0; y < h; ++y) {
        const int y0 = (y - r < 0) ? 0 : (y - r);
        const int y1 = (y + r >= h) ? (h - 1) : (y + r);

        for (int x = 0; x < w; ++x) {
            const int x0 = (x - r < 0) ? 0 : (x - r);
            const int x1 = (x + r >= w) ? (w - 1) : (x + r);
            const int area = (x1 - x0 + 1) * (y1 - y0 + 1);

            const long double sum = rectSum(integ, w, h, x0, y0, x1, y1);
            const long double mW  = sum / (long double)area;

            const std::size_t i = (std::size_t)y * (std::size_t)w + (std::size_t)x;
            const long double sW = sigmaW[i];

            // Eq (5): sigmaAdaptive in [0,1] (RAW scale still fine because it's normalized by range)
            long double sigmaAdaptive = 0.0L;
            if (sigmaRange > EPS) {
                sigmaAdaptive = (sW - sigmaMinNonZero) / sigmaRange;
                if (sigmaAdaptive < 0.0L) sigmaAdaptive = 0.0L;
                if (sigmaAdaptive > 1.0L) sigmaAdaptive = 1.0L;
            }

            // Eq (4) denominator factors (avoid collapse)
            long double a = mg + sW;
            long double b = sigmaAdaptive + sW;
            if (a <= EPS) a = EPS;
            if (b <= EPS) b = EPS;
            const long double denom = a * b;

            // Eq (4) as written in the paper:
            // T = mW - (mW^2 - sigmaW) / ((mg + sigmaW) * (sigmaAdaptive + sigmaW))
            long double T = mW - (((mW * mW) - sW) / denom);

            // Clamp T to RAW range
            if (T < 0.0L) { T = 0.0L; clampedLo++; }
            if (T > (long double)maxv) { T = (long double)maxv; clampedHi++; }

            if (T < T_min) T_min = T;
            if (T > T_max) T_max = T;

            const int iv = clampInt(in.at(x, y), 0, maxv);
            const long double p = (long double)iv;

            // Eq (6): black if i(x,y) < Tw else white
            const bool isBlack = (p < T);
            out.at(x, y) = isBlack ? 0 : maxv;
            if (isBlack) blackCount++; else whiteCount++;
        }
    }

    // --- SANITY CHECK (after second pass) ---
    std::cerr << "T min/max (RAW)    = " << (double)T_min << " / " << (double)T_max << "\n";
    std::cerr << "clamped low count  = " << clampedLo << "\n";
    std::cerr << "clamped high count = " << clampedHi << "\n";
    std::cerr << "black pixels       = " << blackCount << "\n";
    std::cerr << "white pixels       = " << whiteCount << "\n";
    std::cerr << "black ratio        = "
              << ((blackCount + whiteCount) ? (double)blackCount / (double)(blackCount + whiteCount) : 0.0)
              << "\n";
    std::cerr << "========================================\n\n";

    return out;
}
