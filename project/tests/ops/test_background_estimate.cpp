#include <catch2/catch_test_macros.hpp>
#include "ops/background_estimate.h"
#include "image/image_gray.h"

static GrayImage makeConstantImage(int w, int h, int val)
{
    GrayImage img;
    img.width = w;
    img.height = h;
    img.maxval = 255;
    img.data.assign((size_t)w * (size_t)h, val);
    return img;
}

TEST_CASE("BoxBlur: constant image stays constant", "[background_estimate]")
{
    GrayImage img = makeConstantImage(5, 5, 120);
    GrayImage out = background_estimate::boxBlurSeparable(img, 1);

    REQUIRE(out.width == img.width);
    REQUIRE(out.height == img.height);
    REQUIRE(out.data.size() == img.data.size());

    for (int v : out.data)
    {
        REQUIRE(v == 120);
    }
}

TEST_CASE("BoxBlur: radius 0 returns identical image", "[background_estimate]")
{
    GrayImage img = makeConstantImage(4, 4, 77);
    // set one pixel differently
    img.at(1, 1) = 200;

    GrayImage out = background_estimate::boxBlurSeparable(img, 0);

    REQUIRE(out.data.size() == img.data.size());
    for (size_t i = 0; i < img.data.size(); ++i)
    {
        REQUIRE(out.data[i] == img.data[i]);
    }
}

TEST_CASE("BoxBlur: smooths out single bright pixel", "[background_estimate]")
{
    GrayImage img = makeConstantImage(5, 5, 100);
    img.at(2, 2) = 200; // single bright pixel in center

    GrayImage out = background_estimate::boxBlurSeparable(img, 1);

    // The center should be brought closer to 100 (smoothed down)
    REQUIRE(out.at(2, 2) < 200);
    // Corner pixels far from center should remain close to 100
    REQUIRE(out.at(0, 0) >= 95);
    REQUIRE(out.at(0, 0) <= 110);
}

TEST_CASE("BoxBlur: output dimensions match input", "[background_estimate]")
{
    GrayImage img = makeConstantImage(10, 7, 50);
    GrayImage out = background_estimate::boxBlurSeparable(img, 3);

    REQUIRE(out.width == 10);
    REQUIRE(out.height == 7);
    REQUIRE(out.data.size() == 70);
}
