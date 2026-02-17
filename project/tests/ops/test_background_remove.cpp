#include <catch2/catch_test_macros.hpp>
#include "ops/background_remove.h"
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

TEST_CASE("BackgroundRemove: foreground == background yields target", "[background_remove]")
{
    GrayImage denoised = makeConstantImage(5, 5, 150);
    GrayImage background = makeConstantImage(5, 5, 150);
    int target = 200;

    GrayImage out = background_remove::remove(denoised, background, target);

    REQUIRE(out.width == 5);
    REQUIRE(out.height == 5);
    REQUIRE(out.data.size() == 25);

    // When denoised == background, the ratio is 1.0, so result ≈ target
    for (int v : out.data)
    {
        REQUIRE(v >= target - 5);
        REQUIRE(v <= target + 5);
    }
}

TEST_CASE("BackgroundRemove: dark foreground on bright background", "[background_remove]")
{
    GrayImage denoised = makeConstantImage(3, 3, 200);
    GrayImage background = makeConstantImage(3, 3, 200);
    denoised.at(1, 1) = 50; // dark foreground pixel

    GrayImage out = background_remove::remove(denoised, background, 200);

    // The dark pixel should remain darker than the background pixels
    REQUIRE(out.at(1, 1) < out.at(0, 0));
}

TEST_CASE("BackgroundRemove: output dimensions match input", "[background_remove]")
{
    GrayImage d = makeConstantImage(8, 6, 100);
    GrayImage b = makeConstantImage(8, 6, 120);

    GrayImage out = background_remove::remove(d, b, 200);

    REQUIRE(out.width == 8);
    REQUIRE(out.height == 6);
    REQUIRE(out.data.size() == 48);
}
