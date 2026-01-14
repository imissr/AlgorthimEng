// threshold_su.cpp
// Su, Lu, Tan (2010) - contrast-based binarization using local max/min + Otsu on contrast
// Implementation includes a robust fallback when not enough edge pixels are present.
// Created by khale on 14.01.2026.

#include "threshold_su.h"

#include <stdexcept>
#include <vector>
#include <cmath>
#include <cstdint>
#include <string>

#include <omp.h>

#include "util/clamp.h"

// Rectangle sum from (h+1)x(w+1) integral image using inclusive bounds [x0..x1], [y0..y1]
static inline long double rectSum(const std::vector<long double>& integ,
                                  int w, int /*h*/,
                                  int x0, int y0, int x1, int y1) {
    auto atI = [&](int x, int y) -> long double {
        return integ[(std::size_t)y * (std::size_t)(w + 1) + (std::size_t)x];
    };
    return atI(x1 + 1, y1 + 1) - atI(x0, y1 + 1) - atI(x1 + 1, y0) + atI(x0, y0);
}

// Classic Otsu threshold for 8-bit values (0..255)
static int otsuThresholdU8(const std::vector<std::uint8_t>& data) {
    if (data.empty()) return 0;

    long double hist[256] = {0.0L};
    for (std::uint8_t v : data) {
        hist[(int)v] += 1.0L;
    }

    const long double total = (long double)data.size();

    long double sumAll = 0.0L;
    for (int i = 0; i < 256; ++i) sumAll += (long double)i * hist[i];

    long double sumB = 0.0L;
    long double wB = 0.0L;

    long double maxBetween = -1.0L;
    int bestT = 0;

    for (int t = 0; t < 256; ++t) {
        wB += hist[t];
        if (wB <= 0.0L) continue;

        long double wF = total - wB;
        if (wF <= 0.0L) break;

        sumB += (long double)t * hist[t];

        long double mB = sumB / wB;
        long double mF = (sumAll - sumB) / wF;

        long double between = wB * wF * (mB - mF) * (mB - mF);
        if (between > maxBetween) {
            maxBetween = between;
            bestT = t;
        }
    }

    return bestT;
}

GrayImage threshold_su::binarize(const GrayImage& in, int r, int Nmin, double eps) {
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("threshold_su::binarize: invalid image");

    const std::size_t N = (std::size_t)in.width * (std::size_t)in.height;
    if (in.data.size() != N)
        throw std::runtime_error("threshold_su::binarize: data size mismatch");

    if (r <= 0)
        throw std::runtime_error("threshold_su::binarize: windowRadius must be > 0");

    if (Nmin <= 0)
        throw std::runtime_error("threshold_su::binarize: Nmin must be > 0");

    if (!(eps > 0.0))
        throw std::runtime_error("threshold_su::binarize: eps must be > 0");

    const int w = in.width;
    const int h = in.height;
    const int maxv = in.maxval;

    // -------------------------------------------------------------------------
    // Step 1: Build contrast image D using local 3x3 max/min, scale to [0..255]
    // D = (fmax - fmin) / (fmax + fmin + eps)
    // -------------------------------------------------------------------------
    std::vector<std::uint8_t> contrastU8(N, 0);

#pragma omp parallel for default(none) shared(in, contrastU8, w, h, maxv, eps)
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int fmin = maxv;
            int fmax = 0;

            // 3x3 window
            for (int dy = -1; dy <= 1; ++dy) {
                int yy = y + dy;
                if (yy < 0 || yy >= h) continue;

                for (int dx = -1; dx <= 1; ++dx) {
                    int xx = x + dx;
                    if (xx < 0 || xx >= w) continue;

                    int v = clampInt(in.at(xx, yy), 0, maxv);
                    if (v < fmin) fmin = v;
                    if (v > fmax) fmax = v;
                }
            }

            const double num = (double)(fmax - fmin);
            const double den = (double)(fmax + fmin) + eps;
            double D = (den > 0.0) ? (num / den) : 0.0;

            if (D < 0.0) D = 0.0;
            if (D > 1.0) D = 1.0;

            int u = (int)std::lround(D * 255.0);
            if (u < 0) u = 0;
            if (u > 255) u = 255;

            contrastU8[(std::size_t)y * (std::size_t)w + (std::size_t)x] = (std::uint8_t)u;
        }
    }

    // -------------------------------------------------------------------------
    // Step 2: Otsu on contrast image to detect high-contrast pixels (stroke boundary)
    // edgeMask(x,y) = 1 if high contrast, else 0
    // -------------------------------------------------------------------------
    const int tOtsu = otsuThresholdU8(contrastU8);

    std::vector<std::uint8_t> edgeMask(N, 0);
#pragma omp parallel for default(none) shared(edgeMask, contrastU8, N, tOtsu)
    for (std::int64_t i = 0; i < (std::int64_t)N; ++i) {
        edgeMask[(std::size_t)i] = (contrastU8[(std::size_t)i] >= (std::uint8_t)tOtsu) ? 1u : 0u;
    }

    // -------------------------------------------------------------------------
    // Step 3: Build integral images over edge pixels only (count, sum, sumSq)
    // plus integAll for fallback local mean of ALL pixels.
    // -------------------------------------------------------------------------
    std::vector<long double> integCnt((std::size_t)(w + 1) * (std::size_t)(h + 1), 0.0L);
    std::vector<long double> integSum((std::size_t)(w + 1) * (std::size_t)(h + 1), 0.0L);
    std::vector<long double> integSq ((std::size_t)(w + 1) * (std::size_t)(h + 1), 0.0L);
    std::vector<long double> integAll((std::size_t)(w + 1) * (std::size_t)(h + 1), 0.0L);

    auto idxI = [&](int x, int y) -> std::size_t {
        return (std::size_t)y * (std::size_t)(w + 1) + (std::size_t)x;
    };

    for (int y = 1; y <= h; ++y) {
        long double rowCnt = 0.0L;
        long double rowSum = 0.0L;
        long double rowSq  = 0.0L;
        long double rowAll = 0.0L;

        for (int x = 1; x <= w; ++x) {
            const int xx = x - 1;
            const int yy = y - 1;
            const std::size_t p = (std::size_t)yy * (std::size_t)w + (std::size_t)xx;

            const int v = clampInt(in.at(xx, yy), 0, maxv);
            const long double lv = (long double)v;

            const long double e = (edgeMask[p] != 0) ? 1.0L : 0.0L;

            rowCnt += e;
            rowSum += e * lv;
            rowSq  += e * (lv * lv);

            rowAll += lv; // all pixels

            integCnt[idxI(x, y)] = integCnt[idxI(x, y - 1)] + rowCnt;
            integSum[idxI(x, y)] = integSum[idxI(x, y - 1)] + rowSum;
            integSq [idxI(x, y)] = integSq [idxI(x, y - 1)] + rowSq;
            integAll[idxI(x, y)] = integAll[idxI(x, y - 1)] + rowAll;
        }
    }

    // -------------------------------------------------------------------------
    // Step 4: Classify pixels using local thresholds from edge pixels
    // Fallback when Ne < Nmin: threshold using local mean of ALL pixels in window.
    // -------------------------------------------------------------------------
    GrayImage out = in;
    out.data.assign(N, 0);

#pragma omp parallel for default(none) shared(in, out, integCnt, integSum, integSq, integAll, w, h, r, Nmin, maxv)
    for (int y = 0; y < h; ++y) {
        const int y0 = (y - r < 0) ? 0 : (y - r);
        const int y1 = (y + r >= h) ? (h - 1) : (y + r);

        for (int x = 0; x < w; ++x) {
            const int x0 = (x - r < 0) ? 0 : (x - r);
            const int x1 = (x + r >= w) ? (w - 1) : (x + r);

            const long double Ne_ld = rectSum(integCnt, w, h, x0, y0, x1, y1);
            const int Ne = (int)std::llround((double)Ne_ld);

            const int v = clampInt(in.at(x, y), 0, maxv);

            if (Ne >= Nmin && Ne > 0) {
                // SU edge-based stats
                const long double sum   = rectSum(integSum, w, h, x0, y0, x1, y1);
                const long double sumSq = rectSum(integSq,  w, h, x0, y0, x1, y1);

                const long double mean = sum / (long double)Ne;
                const long double secondMoment = sumSq / (long double)Ne;

                long double var = secondMoment - mean * mean;
                if (var < 0.0L) var = 0.0L;

                const long double stddev = std::sqrt((double)var);

                // Paper rule: I(x,y) <= Emean + Estd/2 -> foreground
                const long double T = mean + stddev / 2.0L;

                out.at(x, y) = ((long double)v <= T) ? 0 : maxv;
            } else {
                // Fallback: local mean of ALL pixels (prevents "white holes" in shadows / stroke interiors)
                const int area = (x1 - x0 + 1) * (y1 - y0 + 1);
                const long double sumAll = rectSum(integAll, w, h, x0, y0, x1, y1);
                const long double meanAll = sumAll / (long double)area;

                out.at(x, y) = ((long double)v <= meanAll) ? 0 : maxv;
            }
        }
    }

    return out;
}
