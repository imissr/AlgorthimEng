#include "denoise_median.h"
#include <algorithm>

static int clampCoord(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

GrayImage median3x3(const GrayImage& in) {
    GrayImage out = in;
    out.data.assign((size_t)in.width * (size_t)in.height, 0);

    int neigh[9];
#pragma omp parallel for
    for (int y = 0; y < in.height; ++y) {
        for (int x = 0; x < in.width; ++x) {
            int k = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int xx = clampCoord(x + dx, 0, in.width - 1);
                    int yy = clampCoord(y + dy, 0, in.height - 1);
                    neigh[k++] = in.at(xx, yy);
                }
            }
            std::nth_element(neigh, neigh + 4, neigh + 9); // median at index 4
            out.at(x, y) = neigh[4];
        }
    }
    return out;
}
