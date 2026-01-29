#include "threshold_proposed.h"

#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <limits>

#include "util/clamp.h"

static inline long double rectSum(const std::vector<long double>& integ,
                                  int w, int h,
                                  int x0, int y0, int x1, int y1)
{
    // integral is (h+1) x (w+1)
    auto atI = [&](int x, int y) -> long double {
        return integ[(std::size_t)y * (std::size_t)(w + 1) + (std::size_t)x];
    };
    return atI(x1 + 1, y1 + 1) - atI(x0, y1 + 1) - atI(x1 + 1, y0) + atI(x0, y0);
}

GrayImage threshold_proposed::binarize(const GrayImage& in, int r)
{
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("threshold_proposed::binarize: invalid image");
    if (r <= 0)
        throw std::runtime_error("threshold_proposed::binarize: r must be > 0");

    const int w = in.width;
    const int h = in.height;
    const int maxv = in.maxval;
    const std::size_t N = (std::size_t)w * (std::size_t)h;

    if (in.data.size() != N)
        throw std::runtime_error("threshold_proposed::binarize: data size mismatch");

    // 1) Build integral images on normalized grayscale in [0,1]
    //    I = sum(p), I2 = sum(p^2)
    std::vector<long double> integ((std::size_t)(w + 1) * (std::size_t)(h + 1), 0.0L);
    std::vector<long double> integSq((std::size_t)(w + 1) * (std::size_t)(h + 1), 0.0L);

    auto idx = [&](int x, int y) -> std::size_t {
        return (std::size_t)y * (std::size_t)(w + 1) + (std::size_t)x;
    };

    for (int y = 1; y <= h; ++y) {
        long double rowSum = 0.0L;
        long double rowSumSq = 0.0L;
        for (int x = 1; x <= w; ++x) {
            int iv = clampInt(in.at(x - 1, y - 1), 0, maxv);
            long double p = (long double)iv / (long double)maxv; // [0..1]

            rowSum += p;
            rowSumSq += p * p;

            integ[idx(x, y)]   = integ[idx(x, y - 1)] + rowSum;
            integSq[idx(x, y)] = integSq[idx(x, y - 1)] + rowSumSq;
        }
    }

    // 2) Global mean mg of entire image (normalized)
    const long double sumAll = rectSum(integ, w, h, 0, 0, w - 1, h - 1);
    const long double mg = sumAll / (long double)N;

    // 3) First pass: compute sigmaW for each window; track sigmaMin/sigmaMax
    //    (Eq. 5 needs min/max stddev across windows)
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

            const long double sum   = rectSum(integ,   w, h, x0, y0, x1, y1);
            const long double sumSq = rectSum(integSq, w, h, x0, y0, x1, y1);

            const long double mean = sum / (long double)area;
            const long double meanSq = sumSq / (long double)area;

            long double var = meanSq - mean * mean;
            if (var < 0.0L) var = 0.0L;

            const long double stddev = std::sqrt((double)var);

            const std::size_t i = (std::size_t)y * (std::size_t)w + (std::size_t)x;
            sigmaW[i] = stddev;

            if (stddev < sigmaMin) sigmaMin = stddev;
            if (stddev > sigmaMax) sigmaMax = stddev;
        }
    }

    const long double denomSigma = sigmaMax - sigmaMin;
    // 4) Second pass: compute threshold T per window (Eq. 4),
    //    then binarize (Eq. 6)
    GrayImage out = in;
    out.data.assign(N, 0);

    const long double EPS = 1e-12L;

    #pragma omp parallel for default(none) shared(in, out, integ, sigmaW, w, h, r, maxv, mg, sigmaMin, sigmaMax) firstprivate(denomSigma, EPS)
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

            // Eq (5): sigmaAdaptive = (sigmaW - sigmaMin) / (sigmaMax - sigmaMin)
            long double sAdaptive = 0.0L;
            if (denomSigma > EPS) {
                sAdaptive = (sW - sigmaMin) / denomSigma;
                if (sAdaptive < 0.0L) sAdaptive = 0.0L;
                if (sAdaptive > 1.0L) sAdaptive = 1.0L;
            }

            // Eq (4): T = mW - (mW^2 - sW) / ((mg + sW) * (sAdaptive + sW))
            const long double denom = (mg + sW) * (sAdaptive + sW) + EPS;
            long double T = mW - ((mW * mW) - sW) / denom;

            // clamp T into [0..1] for safety
            if (T < 0.0L) T = 0.0L;
            if (T > 1.0L) T = 1.0L;

            const int iv = clampInt(in.at(x, y), 0, maxv);
            const long double p = (long double)iv / (long double)maxv;

            // Eq (6): if p <= T => black else white
            out.at(x, y) = (p <= T) ? 0 : maxv;
        }
    }

    return out;
}
