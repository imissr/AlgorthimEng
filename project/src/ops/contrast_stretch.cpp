#include "contrast_stretch.h"

#include <stdexcept>
#include <vector>
#include <cmath>

#include "src/util/clamp.h"

GrayImage contrast_stretch::apply(const GrayImage& in) {
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("contrast_stretch::apply: invalid image");

    const std::size_t expected =
        (std::size_t)in.width * (std::size_t)in.height;

    if (in.data.size() != expected)
        throw std::runtime_error("contrast_stretch::apply: data size mismatch");

    int minv = in.data[0];
    int maxv = in.data[0];
    for (int v : in.data) {
        if (v < minv) minv = v;
        if (v > maxv) maxv = v;
    }

    if (maxv <= minv) return in;

    GrayImage out = in;
    out.data.assign(expected, 0);

    const int maxval = in.maxval;
    const int range = maxv - minv;
#pragma omp parallel for
    for (std::size_t i = 0; i < expected; ++i) {
        long long scaled = (long long)(in.data[i] - minv) * (long long)maxval;
        int nv = (int)(scaled / range);
        out.data[i] = clampInt(nv, 0, maxval);
    }

    return out;
}

static int findPercentileValue(const std::vector<int>& hist,
                               int maxval,
                               long long totalPixels,
                               double pct) {
    // pct in [0..100]
    if (pct <= 0.0) return 0;
    if (pct >= 100.0) return maxval;

    long long target = (long long)std::llround((pct / 100.0) * (double)(totalPixels - 1));
    long long cum = 0;
    for (int v = 0; v <= maxval; ++v) {
        cum += hist[v];
        if (cum > target) return v;
    }
    return maxval;
}

GrayImage contrast_stretch::applyPercentile(const GrayImage& in,
                                            double lowPct,
                                            double highPct) {
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("contrast_stretch::applyPercentile: invalid image");

    const std::size_t expected = (std::size_t)in.width * (std::size_t)in.height;
    if (in.data.size() != expected)
        throw std::runtime_error("contrast_stretch::applyPercentile: data size mismatch");

    if (!(lowPct >= 0.0 && lowPct <= 100.0 && highPct >= 0.0 && highPct <= 100.0))
        throw std::runtime_error("contrast_stretch::applyPercentile: percent must be in [0..100]");

    if (highPct <= lowPct) return in;

    const int maxv = in.maxval;
    std::vector<int> hist((std::size_t)maxv + 1, 0);

    for (int v : in.data) {
        int cv = clampInt(v, 0, maxv);
        hist[cv]++;
    }

    const long long total = (long long)expected;

    int lo = findPercentileValue(hist, maxv, total, lowPct);
    int hi = findPercentileValue(hist, maxv, total, highPct);

    if (hi <= lo) return in;

    GrayImage out = in;
    out.data.assign(expected, 0);

    const int range = hi - lo;

    for (std::size_t i = 0; i < expected; ++i) {
        int v = clampInt(in.data[i], 0, maxv);

        // clamp into [lo..hi] first
        if (v < lo) v = lo;
        if (v > hi) v = hi;

        long long scaled = (long long)(v - lo) * (long long)maxv;
        int nv = (int)(scaled / range);
        out.data[i] = clampInt(nv, 0, maxv);
    }

    return out;
}
