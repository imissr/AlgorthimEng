// threshold_su.cpp
// Su, Lu, Tan (2010) — Eq.(1) + Eq.(2) implementation
// Added:
//  - Stroke width estimation (paper paragraph) with Fix B (ignore tiny distances in mode)
//  - DEBUG prints
//  - Fallback when Ne < Nmin: local mean of ALL pixels (prevents white holes)
//
// Usage for auto parameters:
//   --su 0 0 1e-6
//
// Notes:
// - Main classification still follows paper when Ne>=Nmin.
// - Fallback is an engineering addition for real photos (uneven illumination).

#include "threshold_su.h"

#include <stdexcept>
#include <vector>
#include <cmath>
#include <cstdint>
#include <iostream>   // DEBUG

#include <omp.h>

#include "util/clamp.h"

// -------------------- Tunables --------------------
// Fix B: ignore tiny peak-to-peak distances (noise / texture)
static constexpr int SW_MIN_D = 7;   // try 7, 9, 11 if needed
// --------------------------------------------------

static inline long double rectSum(const std::vector<long double> &integ,
                                  int w,
                                  int x0, int y0, int x1, int y1)
{
    auto atI = [&](int x, int y) -> long double
    {
        return integ[(std::size_t)y * (std::size_t)(w + 1) + (std::size_t)x];
    };
    return atI(x1 + 1, y1 + 1)
         - atI(x0,     y1 + 1)
         - atI(x1 + 1, y0)
         + atI(x0,     y0);
}

// Standard Otsu threshold
static int otsuThresholdU8(const std::vector<std::uint8_t> &data)
{
    long double hist[256] = {0};
    for (std::uint8_t v : data)
        hist[v]++;

    const long double total = (long double)data.size();

    long double sumAll = 0.0L;
    for (int i = 0; i < 256; ++i)
        sumAll += (long double)i * hist[i];

    long double sumB = 0.0L;
    long double wB = 0.0L;
    long double maxVar = -1.0L;
    int bestT = 0;

    for (int t = 0; t < 256; ++t)
    {
        wB += hist[t];
        if (wB == 0.0L)
            continue;

        long double wF = total - wB;
        if (wF == 0.0L)
            break;

        sumB += (long double)t * hist[t];

        long double mB = sumB / wB;
        long double mF = (sumAll - sumB) / wF;

        long double varBetween = wB * wF * (mB - mF) * (mB - mF);

        if (varBetween > maxVar)
        {
            maxVar = varBetween;
            bestT = t;
        }
    }
    return bestT;
}

// Stroke width estimation (Su, Lu, Tan 2010) with Fix B
static int estimateStrokeWidth_Su2010(const std::vector<std::uint8_t> &contrast,
                                     int w, int h,
                                     int tOtsu)
{
    std::vector<int> hist((std::size_t)w + 1, 0);

    auto at = [&](int x, int y) -> std::uint8_t
    {
        return contrast[(std::size_t)y * (std::size_t)w + (std::size_t)x];
    };

    for (int y = 0; y < h; ++y)
    {
        int lastPeakX = -1;

        for (int x = 1; x < w - 1; ++x)
        {
            std::uint8_t v0 = at(x - 1, y);
            std::uint8_t v1 = at(x,     y);
            std::uint8_t v2 = at(x + 1, y);

            if (v1 < (std::uint8_t)tOtsu)
                continue;

            if (v1 > v0 && v1 > v2)
            {
                if (lastPeakX >= 0)
                {
                    int d = x - lastPeakX;
                    if (d > 0 && d <= w - 1)
                        hist[(std::size_t)d]++;
                }
                lastPeakX = x;
            }
        }
    }

    int bestD = 0;
    int bestCount = 0;

    const int startD = std::max(1, SW_MIN_D);
    for (int d = startD; d <= w - 1; ++d)
    {
        if (hist[(std::size_t)d] > bestCount)
        {
            bestCount = hist[(std::size_t)d];
            bestD = d;
        }
    }

    if (bestD <= 0)
        throw std::runtime_error("Stroke width estimation failed (no peak distances).");

    return bestD;
}

// Convert stroke width -> r and Nmin
static void chooseParamsFromStrokeWidth(int sw, int &rOut, int &NminOut)
{
    if (sw < 1) sw = 1;

    int Ws = sw;
    if ((Ws % 2) == 0)
        Ws += 1;

    rOut = (Ws - 1) / 2;
    NminOut = Ws;
}

GrayImage threshold_su::binarize(const GrayImage &in,
                                 int r,
                                 int Nmin,
                                 double eps)
{
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("Invalid image");

    const int w = in.width;
    const int h = in.height;
    const int maxv = in.maxval;
    const std::size_t N = (std::size_t)w * (std::size_t)h;

    // ------------------------------------------------------------------
    // STEP 1: Contrast image D(x,y)
    // ------------------------------------------------------------------

    std::vector<std::uint8_t> contrast(N, 0);

#pragma omp parallel for
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            int fmin = maxv;
            int fmax = 0;

            for (int dy = -1; dy <= 1; ++dy)
            {
                int yy = y + dy;
                if (yy < 0 || yy >= h) continue;

                for (int dx = -1; dx <= 1; ++dx)
                {
                    int xx = x + dx;
                    if (xx < 0 || xx >= w) continue;

                    int v = clampInt(in.at(xx, yy), 0, maxv);
                    if (v < fmin) fmin = v;
                    if (v > fmax) fmax = v;
                }
            }

            double D = (double)(fmax - fmin) /
                       ((double)(fmax + fmin) + eps);

            if (D < 0.0) D = 0.0;
            if (D > 1.0) D = 1.0;

            contrast[(std::size_t)y * (std::size_t)w + (std::size_t)x] =
                (std::uint8_t)std::lround(D * 255.0);
        }
    }

    // ------------------------------------------------------------------
    // STEP 2: Otsu on contrast image
    // ------------------------------------------------------------------

    const int tOtsu = otsuThresholdU8(contrast);

    // ------------------------------------------------------------------
    // STEP 2b: Stroke width estimation -> auto r, Nmin
    // ------------------------------------------------------------------

    int sw = -1;
    if (r <= 0 || Nmin <= 0)
    {
        sw = estimateStrokeWidth_Su2010(contrast, w, h, tOtsu);
        chooseParamsFromStrokeWidth(sw, r, Nmin);
    }

    // DEBUG: print parameters
    std::cerr << "[Su2010] tOtsu=" << tOtsu;
    if (sw > 0) std::cerr << " sw=" << sw;
    else        std::cerr << " sw=(manual)";
    std::cerr << " r=" << r << " Nmin=" << Nmin
              << " SW_MIN_D=" << SW_MIN_D
              << "\n";

    // ------------------------------------------------------------------
    // STEP 2c: Edge mask (strict: contrast >= tOtsu)
    // ------------------------------------------------------------------

    std::vector<std::uint8_t> edgeMask(N, 0);

#pragma omp parallel for
    for (std::int64_t i = 0; i < (std::int64_t)N; ++i)
    {
        edgeMask[(std::size_t)i] =
            (contrast[(std::size_t)i] >= (std::uint8_t)tOtsu) ? 1u : 0u;
    }

    // ------------------------------------------------------------------
    // STEP 3: Integral images
    //   - over edge pixels only: cnt / sum / sq
    //   - over ALL pixels: sumAll (for fallback)
    // ------------------------------------------------------------------

    std::vector<long double> integCnt((w + 1) * (h + 1), 0.0L);
    std::vector<long double> integSum((w + 1) * (h + 1), 0.0L);
    std::vector<long double> integSq ((w + 1) * (h + 1), 0.0L);

    // NEW: integral sum over ALL pixels for fallback mean
    std::vector<long double> integAll((w + 1) * (h + 1), 0.0L);

    auto idxI = [&](int x, int y)
    {
        return (std::size_t)y * (std::size_t)(w + 1) + (std::size_t)x;
    };

    for (int y = 1; y <= h; ++y)
    {
        long double rowCnt = 0.0L;
        long double rowSum = 0.0L;
        long double rowSq  = 0.0L;

        long double rowAll = 0.0L;

        for (int x = 1; x <= w; ++x)
        {
            const int ix = x - 1;
            const int iy = y - 1;

            std::size_t p = (std::size_t)iy * (std::size_t)w + (std::size_t)ix;
            long double e = (edgeMask[p] != 0) ? 1.0L : 0.0L;

            int v = clampInt(in.at(ix, iy), 0, maxv);
            long double lv = (long double)v;

            // edge-only integrals
            rowCnt += e;
            rowSum += e * lv;
            rowSq  += e * lv * lv;

            integCnt[idxI(x, y)] = integCnt[idxI(x, y - 1)] + rowCnt;
            integSum[idxI(x, y)] = integSum[idxI(x, y - 1)] + rowSum;
            integSq [idxI(x, y)] = integSq [idxI(x, y - 1)] + rowSq;

            // all-pixels integral (fallback)
            rowAll += lv;
            integAll[idxI(x, y)] = integAll[idxI(x, y - 1)] + rowAll;
        }
    }

    // ------------------------------------------------------------------
    // STEP 4: Classification
    //   - If Ne >= Nmin: strict paper rule (Eq. 2)
    //   - Else: fallback local mean of ALL pixels (prevents white holes)
    // ------------------------------------------------------------------

    GrayImage out = in;
    out.data.assign(N, maxv); // default background

#pragma omp parallel for
    for (int y = 0; y < h; ++y)
    {
        int y0 = std::max(0, y - r);
        int y1 = std::min(h - 1, y + r);

        for (int x = 0; x < w; ++x)
        {
            int x0 = std::max(0, x - r);
            int x1 = std::min(w - 1, x + r);

            const int v = clampInt(in.at(x, y), 0, maxv);

            long double Ne = rectSum(integCnt, w, x0, y0, x1, y1);

            if (Ne >= (long double)Nmin && Ne > 0.0L)
            {
                long double sum   = rectSum(integSum, w, x0, y0, x1, y1);
                long double sumSq = rectSum(integSq,  w, x0, y0, x1, y1);

                long double mean = sum / Ne;
                long double var  = (sumSq / Ne) - mean * mean;
                if (var < 0.0L) var = 0.0L;

                long double stddev = std::sqrt((double)var);
                long double T = mean + stddev / 2.0L;

                if ((long double)v <= T)
                    out.at(x, y) = 0; // foreground
            }
            else
            {
                // Fallback: local mean of ALL pixels
                const int area = (x1 - x0 + 1) * (y1 - y0 + 1);
                const long double sumAll = rectSum(integAll, w, x0, y0, x1, y1);
                const long double meanAll = sumAll / (long double)area;

                const long double T = meanAll - 10.0L;   // try 5..20
                out.at(x, y) = ((long double)v <= T) ? 0 : maxv;
                // else remain background
            }
        }
    }

    return out;
}
