#include "ppm_writer.h"

#include <fstream>
#include <stdexcept>
#include <algorithm>

static int clampInt(int v, int lo, int hi) {
    return std::max(lo, std::min(v, hi));
}

void ppm_writer::writePPMImage(const std::string& fileName,
                               const ppm_reader::PPMImage& img) {
    std::ofstream out(fileName, std::ios::binary); // text output is fine in binary mode
    if (!out) {
        throw std::runtime_error("Cannot open output file: " + fileName);
    }
    writePPMImage(out, img);
}

void ppm_writer::writePPMImage(std::ostream& out,
                               const ppm_reader::PPMImage& img) {
    if (img.width <= 0 || img.height <= 0) {
        throw std::runtime_error("Invalid image dimensions");
    }
    if (img.maxColorValue <= 0) {
        throw std::runtime_error("Invalid maxColorValue");
    }

    const std::size_t expected = static_cast<std::size_t>(img.width) *
                                 static_cast<std::size_t>(img.height) * 3;

    if (img.data.size() != expected) {
        throw std::runtime_error("Invalid data size: expected " +
                                 std::to_string(expected) + " but got " +
                                 std::to_string(img.data.size()));
    }

    // Header
    out << "P3\n";
    out << img.width << " " << img.height << "\n";
    out << img.maxColorValue << "\n";

    // Pixel values (pretty formatting)
    const int valuesPerLine = 12;
    int col = 0;

    for (std::size_t i = 0; i < img.data.size(); i++) {
        int v = clampInt(img.data[i], 0, img.maxColorValue);
        out << v;

        col++;
        if (col == valuesPerLine) {
            out << "\n";
            col = 0;
        } else {
            out << " ";
        }
    }
    if (col != 0) out << "\n";

    if (!out) {
        throw std::runtime_error("Failed while writing PPM output");
    }
}
