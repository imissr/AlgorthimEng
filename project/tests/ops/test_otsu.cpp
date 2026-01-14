
#include <catch2/catch_test_macros.hpp>

#include "project/src/image/image_gray.h"
#include "project/src/ops/threshold_otsu.h"


TEST_CASE("Otsu: simple bimodal image is separated correctly", "[otsu]") {
    GrayImage img;
    img.width = 4;
    img.height = 1;
    img.maxval = 255;
    img.data = {0, 0, 255, 255};

    GrayImage bw = threshold_otsu::binarize(img);

    REQUIRE(bw.width == img.width);
    REQUIRE(bw.height == img.height);
    REQUIRE(bw.maxval == img.maxval);
    REQUIRE(bw.data.size() == img.data.size());

    // Convention: <= threshold -> black (0), else white (maxval)
    REQUIRE(bw.data[0] == 0);
    REQUIRE(bw.data[1] == 0);
    REQUIRE(bw.data[2] == 255);
    REQUIRE(bw.data[3] == 255);
}

TEST_CASE("Otsu: constant image stays all white (or all black depending on rule)", "[otsu]") {
    GrayImage img;
    img.width = 3;
    img.height = 3;
    img.maxval = 255;
    img.data.assign(9, 120);

    GrayImage bw = threshold_otsu::binarize(img);

    REQUIRE(bw.data.size() == img.data.size());

    // Your current implementation picks bestT=0 for constant hist (often),
    // so 120 > 0 => white (255).
    // If you later change behavior, adjust this expectation.
    for (int v : bw.data) {
        REQUIRE(v == 255);
    }
}
