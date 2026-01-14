//
// Created by khale on 14.01.2026.
//

#include "threshold_nick.h"

#include <stdexcept>

#include <omp.h>

#include "util/clamp.h"


// rectangle sum from integral image (inclusive bounds)
static inline long double rectSum(const std::vector<long double>& integ,
                                 int w, int h,
                                 int x0, int y0, int x1, int y1) {
    auto atI = [&](int x, int y) -> long double {
        return integ[(std::size_t)y * (std::size_t)(w + 1) + (std::size_t)x];
    };
    return atI(x1 + 1, y1 + 1) - atI(x0, y1 + 1) - atI(x1 + 1, y0) + atI(x0, y0);
}

GrayImage threshold_nick::binarize(const GrayImage& in, int r, double k) {
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("threshold_nick::binarize: invalid image");

    const std::size_t N = (std::size_t)in.width * (std::size_t)in.height;
    if (in.data.size() != N)
        throw std::runtime_error("threshold_nick::binarize: data size mismatch");

    if (r <= 0)
        throw std::runtime_error("threshold_nick::binarize: windowRadius must be > 0");

    const int w = in.width;
    const int h = in.height;
    const int maxv = in.maxval;

    // integral images for sum and sum of squares
    std::vector<long double> integ((std::size_t)(w + 1) * (std::size_t)(h + 1), 0.0L);
    std::vector<long double> integSq((std::size_t)(w + 1) * (std::size_t)(h + 1), 0.0L);

    auto idx = [&](int x, int y) -> std::size_t {
        return (std::size_t)y * (std::size_t)(w + 1) + (std::size_t)x;
    };
    for (int y = 1; y <= h; ++y) {
        long double rowSum = 0.0L;
        long double rowSumSq = 0.0L;
        for (int x = 1; x <= w; ++x) {
            int v = clampInt(in.at(x - 1, y - 1), 0, maxv);
            long double lv = (long double)v;
            rowSum += lv;
            rowSumSq += lv * lv;

            integ[idx(x, y)]   = integ[idx(x, y - 1)] + rowSum;
            integSq[idx(x, y)] = integSq[idx(x, y - 1)] + rowSumSq;
        }
    }

    GrayImage out = in;
    out.data.assign(N, 0);
#pragma omp parallel for default(none) shared(in, out, integ, integSq, w, h, r, k, maxv)
    for (int y = 0; y < h; ++y) {
        int y0 = (y - r < 0) ? 0 : (y - r);
        int y1 = (y + r >= h) ? (h - 1) : (y + r);

        for (int x = 0; x < w; ++x) {
            int x0 = (x - r < 0) ? 0 : (x - r);
            int x1 = (x + r >= w) ? (w - 1) : (x + r);

            int area = (x1 - x0 + 1) * (y1 - y0 + 1);

            long double sum   = rectSum(integ,   w, h, x0, y0, x1, y1);
            long double sumSq = rectSum(integSq, w, h, x0, y0, x1, y1);

            long double mean = sum / (long double)area;
            long double secondMoment = sumSq / (long double)area;

            // variance = E[x^2] - (E[x])^2
            long double var = secondMoment - mean * mean;
            if (var < 0.0L) var = 0.0L;

            long double stddev = std::sqrt((double)var);

            // NICK: T = m + k * stddev   (k negative shifts threshold down)
            long double T = mean + (long double)k * stddev;

            int v = clampInt(in.at(x, y), 0, maxv);
            out.at(x, y) = ((long double)v <= T) ? 0 : maxv;
        }
    }

    return out;
}