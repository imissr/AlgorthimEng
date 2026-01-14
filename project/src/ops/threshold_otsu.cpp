#include "threshold_otsu.h"
#include <stdexcept>
#include <vector>

#include "src/util/clamp.h"
#include <omp.h>



GrayImage threshold_otsu::binarize(const GrayImage& in) {
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("threshold_otsu::binarize: invalid image");

    const std::size_t N = (std::size_t)in.width * (std::size_t)in.height;
    if (in.data.size() != N)
        throw std::runtime_error("threshold_otsu::binarize: data size mismatch");

    const int maxv = in.maxval;
    std::vector<long long> hist((std::size_t)maxv + 1, 0);

    for (int v : in.data) {
        int cv = clampInt(v, 0, maxv);
        hist[cv]++;
    }

    // Total mean
    long double sumAll = 0.0L;
    for (int i = 0; i <= maxv; ++i) sumAll += (long double)i * (long double)hist[i];

    long double sumB = 0.0L;
    long double wB = 0.0L;
    long double wF = 0.0L;

    long double bestVar = -1.0L;
    int bestT = 0;

    for (int t = 0; t <= maxv; ++t) {
        wB += (long double)hist[t];
        if (wB == 0.0L) continue;

        wF = (long double)N - wB;
        if (wF == 0.0L) break;

        sumB += (long double)t * (long double)hist[t];

        long double mB = sumB / wB;
        long double mF = (sumAll - sumB) / wF;

        long double betweenVar = wB * wF * (mB - mF) * (mB - mF);
        if (betweenVar > bestVar) {
            bestVar = betweenVar;
            bestT = t;
        }
    }

    GrayImage out = in;
    out.data.assign(N, 0);

    // Convention: <= threshold => black (0), else white (maxv)
#pragma omp parallel for default(none) shared(in, out, maxv, bestT, N)
    for (std::size_t i = 0; i < N; ++i) {
        int v = clampInt(in.data[i], 0, maxv);
        out.data[i] = (v <= bestT) ? 0 : maxv;
    }
    return out;
}
