#include <catch2/catch_test_macros.hpp>
#include "ops/threshold_su.h"
#include "image/image_gray.h"

// Su threshold uses contrast-based edge detection + local Otsu.
// v <= T → foreground (0), else background (maxval).
// For constant regions: contrast=0, tOtsu=0, so ALL pixels are edge pixels,
// and T = mean (stddev=0), so v == mean <= T → foreground.

TEST_CASE("Su: stroke pixels classified as foreground", "[su]")
{
    GrayImage img;
    img.width = 11;
    img.height = 11;
    img.maxval = 255;
    img.data.assign(121, 230);

    // Draw a vertical black stroke of width 3
    for (int y = 0; y < img.height; ++y)
    {
        img.at(4, y) = 10;
        img.at(5, y) = 10;
        img.at(6, y) = 10;
    }

    GrayImage bw = threshold_su::binarize(img, 3, 3, 1e-6);

    REQUIRE(bw.data.size() == img.data.size());

    // Stroke center should be foreground (black)
    for (int y = 1; y < img.height - 1; ++y)
    {
        REQUIRE(bw.at(5, y) == 0);
    }
}

TEST_CASE("Su: output is binary", "[su]")
{
    GrayImage img;
    img.width = 9;
    img.height = 9;
    img.maxval = 255;
    img.data.assign(81, 128);
    img.at(0, 0) = 10;
    img.at(8, 8) = 250;
    img.at(4, 4) = 30;

    GrayImage bw = threshold_su::binarize(img, 3, 3, 1e-6);

    for (int v : bw.data)
    {
        REQUIRE((v == 0 || v == 255));
    }
}

TEST_CASE("Su: output dimensions match input", "[su]")
{
    GrayImage img;
    img.width = 12;
    img.height = 8;
    img.maxval = 255;
    img.data.assign(96, 100);

    GrayImage bw = threshold_su::binarize(img, 4, 4, 1e-6);

    REQUIRE(bw.width == 12);
    REQUIRE(bw.height == 8);
    REQUIRE(bw.data.size() == 96);
}

TEST_CASE("Su: constant black image becomes all foreground", "[su]")
{
    // Constant 0: contrast=0, tOtsu=0, all edge pixels, mean=0, T=0.
    // 0 <= 0 → foreground.
    GrayImage img;
    img.width = 7;
    img.height = 7;
    img.maxval = 255;
    img.data.assign(49, 0);

    GrayImage bw = threshold_su::binarize(img, 2, 2, 1e-6);

    for (int v : bw.data)
    {
        REQUIRE(v == 0);
    }
}

TEST_CASE("Su: dark text on lighter background", "[su]")
{
    // Build a bigger image so there are genuine edge pixels
    GrayImage img;
    img.width = 15;
    img.height = 15;
    img.maxval = 255;
    img.data.assign(225, 200);

    // Draw a thick cross
    for (int i = 0; i < 15; ++i)
    {
        img.at(7, i) = 20;
        img.at(i, 7) = 20;
    }

    GrayImage bw = threshold_su::binarize(img, 3, 3, 1e-6);

    // Center of the cross should be foreground
    REQUIRE(bw.at(7, 7) == 0);
}
