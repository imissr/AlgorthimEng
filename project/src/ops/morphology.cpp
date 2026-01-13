#include "morphology.h"
#include <stdexcept>

#include "src/util/clamp.h"


GrayImage morphology::erode3x3(const GrayImage& in) {
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("morphology::erode3x3: invalid image");

    const std::size_t N = (std::size_t)in.width * (std::size_t)in.height;
    if (in.data.size() != N)
        throw std::runtime_error("morphology::erode3x3: data size mismatch");

    GrayImage out = in;
    out.data.assign(N, in.maxval);

    // Erosion for black foreground:
    // output black (0) only if ALL neighbors are black (0)
    for (int y = 0; y < in.height; ++y) {
        for (int x = 0; x < in.width; ++x) {
            bool allBlack = true;
            for (int dy = -1; dy <= 1 && allBlack; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int xx = clampInt(x + dx, 0, in.width - 1);
                    int yy = clampInt(y + dy, 0, in.height - 1);
                    if (in.at(xx, yy) != 0) { allBlack = false; break; }
                }
            }
            out.at(x, y) = allBlack ? 0 : in.maxval;
        }
    }
    return out;
}

GrayImage morphology::dilate3x3(const GrayImage& in) {
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("morphology::dilate3x3: invalid image");

    const std::size_t N = (std::size_t)in.width * (std::size_t)in.height;
    if (in.data.size() != N)
        throw std::runtime_error("morphology::dilate3x3: data size mismatch");

    GrayImage out = in;
    out.data.assign(N, in.maxval);

    // Dilation for black foreground:
    // output black (0) if ANY neighbor is black (0)
    for (int y = 0; y < in.height; ++y) {
        for (int x = 0; x < in.width; ++x) {
            bool anyBlack = false;
            for (int dy = -1; dy <= 1 && !anyBlack; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int xx = clampInt(x + dx, 0, in.width - 1);
                    int yy = clampInt(y + dy, 0, in.height - 1);
                    if (in.at(xx, yy) == 0) { anyBlack = true; break; }
                }
            }
            out.at(x, y) = anyBlack ? 0 : in.maxval;
        }
    }
    return out;
}

GrayImage morphology::open3x3(const GrayImage& in) {
    return dilate3x3(erode3x3(in));
}

GrayImage morphology::close3x3(const GrayImage& in) {
    return erode3x3(dilate3x3(in));
}
