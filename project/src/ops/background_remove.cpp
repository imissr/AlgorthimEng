#include "background_remove.h"

#include <stdexcept>

#include "src/util/clamp.h"


GrayImage background_remove::remove(const GrayImage& denoised,
                                    const GrayImage& background,
                                    int target) {
    if (denoised.width != background.width ||
        denoised.height != background.height ||
        denoised.maxval != background.maxval) {
        throw std::runtime_error("background_remove: images must match (w/h/maxval)");
        }

    const std::size_t expected =
        (std::size_t)denoised.width * (std::size_t)denoised.height;

    if (denoised.data.size() != expected || background.data.size() != expected)
        throw std::runtime_error("background_remove: data size mismatch");

    GrayImage out = denoised;
    out.data.assign(expected, 0);

    const int maxv = denoised.maxval;
    target = clampInt(target, 0, maxv);

    for (std::size_t i = 0; i < expected; ++i) {
        int clean = denoised.data[i] - background.data[i] + target;
        out.data[i] = clampInt(clean, 0, maxv);
    }
    return out;
}
