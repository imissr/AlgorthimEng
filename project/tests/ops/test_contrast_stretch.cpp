#include <catch2/catch_test_macros.hpp>
#include "ops/contrast_stretch.h"
#include "image/image_gray.h"

static GrayImage makeImage(int w, int h, const std::vector<int> &data)
{
    GrayImage img;
    img.width = w;
    img.height = h;
    img.maxval = 255;
    img.data = data;
    return img;
}

TEST_CASE("ContrastStretch: already full-range image is unchanged", "[contrast_stretch]")
{
    GrayImage img = makeImage(4, 1, {0, 85, 170, 255});
    GrayImage out = contrast_stretch::apply(img);

    REQUIRE(out.data.size() == 4);
    REQUIRE(out.data[0] == 0);
    REQUIRE(out.data[3] == 255);
}

TEST_CASE("ContrastStretch: stretches narrow range to full range", "[contrast_stretch]")
{
    // All values between 100 and 200 should stretch to [0, 255]
    GrayImage img = makeImage(3, 1, {100, 150, 200});
    GrayImage out = contrast_stretch::apply(img);

    REQUIRE(out.data.size() == 3);
    REQUIRE(out.data[0] == 0);   // min maps to 0
    REQUIRE(out.data[2] == 255); // max maps to 255
    // Middle value should be somewhere in between
    REQUIRE(out.data[1] > 0);
    REQUIRE(out.data[1] < 255);
}

TEST_CASE("ContrastStretch: constant image", "[contrast_stretch]")
{
    GrayImage img = makeImage(3, 3, std::vector<int>(9, 128));
    GrayImage out = contrast_stretch::apply(img);

    // When min == max, behavior is implementation-defined; just check no crash
    REQUIRE(out.data.size() == 9);
}

TEST_CASE("ContrastStretch: percentile stretch clamps outliers", "[contrast_stretch]")
{
    // 10 pixels: mostly 100, with one outlier at 0 and one at 255
    GrayImage img = makeImage(5, 2, {0, 100, 100, 100, 100, 100, 100, 100, 100, 255});

    // Stretch between 10% and 90% percentiles — outliers get clipped
    GrayImage out = contrast_stretch::applyPercentile(img, 0.1, 0.9);

    REQUIRE(out.data.size() == 10);
    // The outlier at 0 should be clipped to 0
    REQUIRE(out.data[0] == 0);
    // The outlier at 255 should be clipped to 255
    REQUIRE(out.data[9] == 255);
}

TEST_CASE("ContrastStretch: percentile output dimensions match", "[contrast_stretch]")
{
    GrayImage img = makeImage(4, 3, std::vector<int>(12, 50));
    GrayImage out = contrast_stretch::applyPercentile(img, 0.05, 0.95);

    REQUIRE(out.width == 4);
    REQUIRE(out.height == 3);
    REQUIRE(out.data.size() == 12);
}
