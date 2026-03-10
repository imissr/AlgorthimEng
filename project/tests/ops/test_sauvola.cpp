#include <catch2/catch_test_macros.hpp>
#include "ops/threshold_sauvola.h"
#include "image/image_gray.h"

TEST_CASE("Sauvola: keeps black stroke on white background", "[sauvola]") {
    GrayImage img;
    img.width = 7;
    img.height = 7;
    img.maxval = 255;
    img.data.assign((size_t)img.width * (size_t)img.height, 240); // near-white paper

    // Add a vertical black line in the middle
    for (int y = 0; y < img.height; ++y) {
        img.at(3, y) = 0;
    }

    // Typical Sauvola parameters
    int radius = 2;
    double k = 0.34;

    GrayImage bw = threshold_sauvola::binarize(img, radius, k);

    REQUIRE(bw.data.size() == img.data.size());

    // The stroke pixels should remain black
    for (int y = 0; y < img.height; ++y) {
        REQUIRE(bw.at(3, y) == 0);
    }

    // Most background should become white
    REQUIRE(bw.at(0, 0) == 255);
    REQUIRE(bw.at(6, 6) == 255);
}
