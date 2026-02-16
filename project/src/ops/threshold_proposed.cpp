#include "threshold_proposed.h"

#include <vector>
#include <cmath>
#include <stdexcept>
#include <limits>

#include "util/clamp.h"

static inline long double rectSum(const std::vector<long double>& integ,
                                  int w, int h,
                                  int x0, int y0, int x1, int y1) {
    (void)h; // Integral shape is defined by w and row-major indexing.
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

    // Build integral images from normalized pixel values p in [0,1].
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
            const long double p = (long double)iv / (long double)maxv;

            rowSum += p;
            rowSumSq += p * p;

            integ[idx(x, y)] = integ[idx(x, y - 1)] + rowSum;
            integSq[idx(x, y)] = integSq[idx(x, y - 1)] + rowSumSq;
        }
    }

    const long double sumAll = rectSum(integ, w, h, 0, 0, w - 1, h - 1);
    const long double mg = sumAll / (long double)N;

    // First pass: local standard deviation per window + global min/max sigma.
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

            const long double sum = rectSum(integ, w, h, x0, y0, x1, y1);
            const long double sumSq = rectSum(integSq, w, h, x0, y0, x1, y1);

            const long double mW = sum / (long double)area;
            const long double secondMoment = sumSq / (long double)area;

            long double var = secondMoment - mW * mW;
            if (var < 0.0L) {
                var = 0.0L;
            }

            const long double sW = sqrtl(var);
            const std::size_t i = (std::size_t)y * (std::size_t)w + (std::size_t)x;
            sigmaW[i] = sW;

            if (sW < sigmaMin) sigmaMin = sW;
            if (sW > sigmaMax) sigmaMax = sW;
        }
    }

    GrayImage out = in;
    out.data.assign(N, 0);

    const long double EPS = 1e-12L;
    const long double sigmaRange = sigmaMax - sigmaMin;

    // Second pass: Eq. (5) adaptive sigma, Eq. (4) threshold, Eq. (6) binarization.
#pragma omp parallel for default(none) shared(in, out, integ, sigmaW, w, h, r, maxv, mg, sigmaMin) firstprivate(sigmaRange, EPS)
    for (int y = 0; y < h; ++y) {
        const int y0 = (y - r < 0) ? 0 : (y - r);
        const int y1 = (y + r >= h) ? (h - 1) : (y + r);

        for (int x = 0; x < w; ++x) {
            const int x0 = (x - r < 0) ? 0 : (x - r);
            const int x1 = (x + r >= w) ? (w - 1) : (x + r);
            const int area = (x1 - x0 + 1) * (y1 - y0 + 1);

            const long double sum = rectSum(integ, w, h, x0, y0, x1, y1);
            const long double mW = sum / (long double)area;

            const std::size_t i = (std::size_t)y * (std::size_t)w + (std::size_t)x;
            const long double sW = sigmaW[i];

            long double sigmaAdaptive = 0.0L;
            if (sigmaRange > EPS) {
                sigmaAdaptive = (sW - sigmaMin) / sigmaRange;
            }

            long double denom = (mg + sW) * (sigmaAdaptive + sW);
            if (denom <= EPS) {
                denom = EPS;
            }

            const long double T = mW - (((mW * mW) - sW) / denom);

            const int iv = clampInt(in.at(x, y), 0, maxv);
            const long double p = (long double)iv / (long double)maxv;
            out.at(x, y) = (p <= T) ? 0 : maxv;
        }
    }

    return out;
}