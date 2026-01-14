#include "background_estimate.h"

#include <stdexcept>
#include <vector>
#include <omp.h>

#include "src/util/clamp.h"


GrayImage background_estimate::boxBlurSeparable(const GrayImage& in, int radius) {
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("boxBlurSeparable: invalid image");

    const std::size_t expected = (std::size_t)in.width * (std::size_t)in.height;
    if (in.data.size() != expected)
        throw std::runtime_error("boxBlurSeparable: data size mismatch");

    if (radius <= 0) return in;

    GrayImage tmp = in;
    GrayImage out = in;
    tmp.data.assign(expected, 0);
    out.data.assign(expected, 0);

    const int w = in.width;
    const int h = in.height;
    const int window = 2 * radius + 1;

    // Horizontal pass
#pragma omp parallel for default(none) shared(in, tmp, w, h, radius, window)
    for (int y = 0; y < h; ++y) {
        long long sum = 0;

        // initial window around x=0 (replicate borders)
        for (int dx = -radius; dx <= radius; ++dx) {
            int xx = clampInt(dx, 0, w - 1);
            sum += in.at(xx, y);
        }

        for (int x = 0; x < w; ++x) {
            tmp.at(x, y) = (int)(sum / window);

            // slide window: remove left, add right
            int x_remove = clampInt(x - radius, 0, w - 1);
            int x_add    = clampInt(x + radius + 1, 0, w - 1);
            sum += in.at(x_add, y) - in.at(x_remove, y);
        }
    }

    // Vertical pass
#pragma omp parallel for default(none) shared(tmp, out, w, h, radius, window)
    for (int x = 0; x < w; ++x) {
        long long sum = 0;

        // initial window around y=0 (replicate borders)
        for (int dy = -radius; dy <= radius; ++dy) {
            int yy = clampInt(dy, 0, h - 1);
            sum += tmp.at(x, yy);
        }

        for (int y = 0; y < h; ++y) {
            out.at(x, y) = (int)(sum / window);

            int y_remove = clampInt(y - radius, 0, h - 1);
            int y_add    = clampInt(y + radius + 1, 0, h - 1);
            sum += tmp.at(x, y_add) - tmp.at(x, y_remove);
        }
    }

    return out;
}
