#include "border_cleanup.h"
#include <stdexcept>

#include "src/util/clamp.h"


GrayImage border_cleanup::whitenEdges(const GrayImage& in, int borderWidth) {
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("border_cleanup::whitenEdges: invalid image");

    const std::size_t N = (std::size_t)in.width * (std::size_t)in.height;
    if (in.data.size() != N)
        throw std::runtime_error("border_cleanup::whitenEdges: data size mismatch");

    if (borderWidth <= 0) return in;

    GrayImage out = in;

    int bw = borderWidth;
    if (bw * 2 >= in.width)  bw = in.width / 2;
    if (bw * 2 >= in.height) bw = in.height / 2;
#pragma omp parallel for
    for (int y = 0; y < in.height; ++y) {
        for (int x = 0; x < in.width; ++x) {
            bool isBorder =
                (x < bw) || (x >= in.width - bw) || (y < bw) || (y >= in.height - bw);

            if (isBorder) out.at(x, y) = in.maxval;
        }
    }
    return out;
}

GrayImage border_cleanup::whitenDarkEdges(const GrayImage& in, int borderWidth, double thresholdFrac) {
    if (in.width <= 0 || in.height <= 0 || in.maxval <= 0)
        throw std::runtime_error("border_cleanup::whitenDarkEdges: invalid image");

    const std::size_t N = (std::size_t)in.width * (std::size_t)in.height;
    if (in.data.size() != N)
        throw std::runtime_error("border_cleanup::whitenDarkEdges: data size mismatch");

    if (borderWidth <= 0) return in;

    GrayImage out = in;

    int bw = borderWidth;
    if (bw * 2 >= in.width)  bw = in.width / 2;
    if (bw * 2 >= in.height) bw = in.height / 2;

    int thr = (int)(thresholdFrac * (double)in.maxval);
    thr = clampInt(thr, 0, in.maxval);
#pragma omp parallel for default(none) shared(in, out, bw, thr)
    for (int y = 0; y < in.height; ++y) {
        for (int x = 0; x < in.width; ++x) {
            bool isBorder =
                (x < bw) || (x >= in.width - bw) || (y < bw) || (y >= in.height - bw);

            if (isBorder && out.at(x, y) < thr) {
                out.at(x, y) = in.maxval;
            }
        }
    }
    return out;
}
