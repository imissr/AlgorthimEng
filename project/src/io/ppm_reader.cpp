#include "ppm_reader.h"
#include <stdexcept>
#include <string>

ppm_reader::ppm_reader(std::istream& inputStream)
    : tokenReader(inputStream) {}

ppm_reader::PPMImage ppm_reader::readPPMImage() {
    PPMImage img;

    const std::string magic = tokenReader.nextToken();
    if (magic != "P3") {
        throw std::runtime_error("Not a P3 PPM file (magic=" + magic + ")");
    }

    img.width = tokenReader.nextIntToken();
    img.height = tokenReader.nextIntToken();
    img.maxColorValue = tokenReader.nextIntToken();

    if (img.width <= 0 || img.height <= 0 || img.maxColorValue <= 0) {
        throw std::runtime_error("Invalid PPM header values");
    }

    const std::size_t total = static_cast<std::size_t>(img.width) *
                              static_cast<std::size_t>(img.height) * 3;

    img.data.resize(total);
    for (std::size_t i = 0; i < total; ++i) {
        int v = tokenReader.nextIntToken();
        if (v < 0 || v > img.maxColorValue) {
            throw std::runtime_error("PPM pixel value out of range: " + std::to_string(v));
        }
        img.data[i] = v;
    }

    return img;
}
