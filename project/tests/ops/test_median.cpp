#include <catch2/catch_test_macros.hpp>
#include "ops/denoise_median.h"
#include "image/image_gray.h"

TEST_CASE("Median3x3: removes single salt noise pixel", "[median]") {
    GrayImage img;
    img.width = 3;
    img.height = 3;
    img.maxval = 255;

    // Mostly 100, one noisy 255 in the center
    img.data = {
        100, 100, 100,
        100, 255, 100,
        100, 100, 100
    };

    GrayImage den = median3x3(img);

    REQUIRE(den.data.size() == img.data.size());
    REQUIRE(den.at(1, 1) == 100);
}
