//
// Created by khale on 11.01.2026.
//

#include "ppm_reader.h"
#include "token_reader.h"
#include <fstream>
#include <stdexcept>


ppm_reader::ppm_reader(std::istream& inputStream) : tokenReader(inputStream) {}

ppm_reader::PPMImage ppm_reader::readPPMImage(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file");
    }

   TokenReader tokenStream(file);

    PPMImage image;

    std::string magic = tokenStream.nextToken();
    if (magic != "P3") {
        std::cout << "No magic found" << magic << std::endl;
        throw std::runtime_error("Invalid PPM format: expected P3");
    }
    image.width = tokenStream.nextIntToken();
    image.height = tokenStream.nextIntToken();
    image.maxColorValue = tokenStream.nextIntToken();

    const long long totalPixels = static_cast<long long>(image.width) * static_cast<long long>(image.height) * 3;
    image.data.reserve(static_cast<size_t>(totalPixels));
    for (long long i = 0; i < totalPixels; ++i) {
        int value = tokenStream.nextIntToken();
        if (value < 0 || value > image.maxColorValue) {
            throw std::runtime_error("Pixel value out of range: " + std::to_string(value));
        }
        image.data.push_back(value);
    }
    return image;

}


