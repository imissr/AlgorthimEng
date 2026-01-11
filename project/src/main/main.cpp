//
// Created by khale on 11.01.2026.
//

#include <fstream>
#include <iostream>
#include <string>

#include "io/ppm_reader.h"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    const std::string ppmPath = "C:\\Users\\khale\\CLionProjects\\AlgEng\\project\\data\\colorP3File.ppm";
    try {
        std::ifstream file(ppmPath);
        if (!file.is_open()) {
            std::cerr << "Cannot open: " << ppmPath << "\n";
            return 1;
        }

        ppm_reader reader(file);
        auto image = reader.readPPMImage(ppmPath); // or refactor readPPMImage to read from stream

        std::cout << "PPM " << image.width << "x" << image.height << "\n";
        std::cout << "Data size: " << image.data.size() << "\n";


    } catch (const std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }
}
