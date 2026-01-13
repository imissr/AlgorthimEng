#include "colorspace.h"
#include <stdexcept>
#include <algorithm>
#include <cmath>

static int clampInt(int v, int lo, int hi) {
    return std::max(lo, std::min(v, hi));
}

GrayImage rgbToGray(const ppm_reader::PPMImage& rgb) {
    if (rgb.width <= 0 || rgb.height <= 0 || rgb.maxColorValue <= 0)
        throw std::runtime_error("rgbToGray: invalid rgb image");

    const std::size_t expected = (std::size_t)rgb.width * (std::size_t)rgb.height * 3;
    if (rgb.data.size() != expected)
        throw std::runtime_error("rgbToGray: rgb data size mismatch");

    GrayImage g;
    g.width = rgb.width;
    g.height = rgb.height;
    g.maxval = rgb.maxColorValue;
    g.data.resize((std::size_t)g.width * (std::size_t)g.height);

    for (int y = 0; y < g.height; ++y) {
        for (int x = 0; x < g.width; ++x) {
            std::size_t i = ((std::size_t)y * g.width + x) * 3;
            int R = rgb.data[i + 0];
            int G = rgb.data[i + 1];
            int B = rgb.data[i + 2];

            // Luma (Rec.601 style): 0.299R + 0.587G + 0.114B
            double gray = 0.299 * R + 0.587 * G + 0.114 * B;
            int gv = (int)std::lround(gray);
            g.at(x, y) = clampInt(gv, 0, g.maxval);
        }
    }
    return g;
}

ppm_reader::PPMImage grayToRgb(const GrayImage& g) {
    if (g.width <= 0 || g.height <= 0 || g.maxval <= 0)
        throw std::runtime_error("grayToRgb: invalid gray image");

    const std::size_t expected = (std::size_t)g.width * (std::size_t)g.height;
    if (g.data.size() != expected)
        throw std::runtime_error("grayToRgb: gray data size mismatch");

    ppm_reader::PPMImage rgb;
    rgb.width = g.width;
    rgb.height = g.height;
    rgb.maxColorValue = g.maxval;
    rgb.data.resize(expected * 3);

    for (std::size_t p = 0; p < expected; ++p) {
        int v = clampInt(g.data[p], 0, g.maxval);
        rgb.data[p * 3 + 0] = v;
        rgb.data[p * 3 + 1] = v;
        rgb.data[p * 3 + 2] = v;
    }
    return rgb;
}
