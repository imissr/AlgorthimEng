#include <catch2/catch_test_macros.hpp>
#include "ops/threshold_proposed.h"
#include "image/image_gray.h"

// Proposed threshold: p < T → foreground (0), else background (maxval).
// Uses strict less-than, so constant images (where T = f(mean)) tend to stay background.

TEST_CASE("Proposed: stroke pixels classified as foreground", "[proposed]")
{
    GrayImage img;
    img.width = 9;
    img.height = 9;
    img.maxval = 255;
    img.data.assign(81, 230);

    // Vertical black stroke in the center
    for (int y = 0; y < img.height; ++y)
    {
        img.at(4, y) = 10;
    }

    GrayImage bw = threshold_proposed::binarize(img, 3);

    REQUIRE(bw.data.size() == img.data.size());

    // Stroke pixels should be classified as foreground
    for (int y = 0; y < img.height; ++y)
    {
        REQUIRE(bw.at(4, y) == 0);
    }
}

TEST_CASE("Proposed: constant white image stays white", "[proposed]")
{
    // Constant 255: sigma=0, T formula collapses. p=255, and p < T is false → white.
    GrayImage img;
    img.width = 5;
    img.height = 5;
    img.maxval = 255;
    img.data.assign(25, 255);

    GrayImage bw = threshold_proposed::binarize(img, 2);

    REQUIRE(bw.data.size() == 25);
    for (int v : bw.data)
    {
        REQUIRE(v == 255);
    }
}

TEST_CASE("Proposed: constant black image becomes white (strict less-than)", "[proposed]")
{
    // Constant 0: T=0, and 0 < 0 is false → background (255)
    GrayImage img;
    img.width = 5;
    img.height = 5;
    img.maxval = 255;
    img.data.assign(25, 0);

    GrayImage bw = threshold_proposed::binarize(img, 2);

    REQUIRE(bw.data.size() == 25);
    for (int v : bw.data)
    {
        REQUIRE(v == 255);
    }
}

TEST_CASE("Proposed: output is binary", "[proposed]")
{
    GrayImage img;
    img.width = 6;
    img.height = 6;
    img.maxval = 255;
    img.data.assign(36, 128);
    img.at(0, 0) = 10;
    img.at(5, 5) = 250;

    GrayImage bw = threshold_proposed::binarize(img, 2);

    for (int v : bw.data)
    {
        REQUIRE((v == 0 || v == 255));
    }
}

TEST_CASE("Proposed: output dimensions match input", "[proposed]")
{
    GrayImage img;
    img.width = 10;
    img.height = 8;
    img.maxval = 255;
    img.data.assign(80, 100);

    GrayImage bw = threshold_proposed::binarize(img, 4);

    REQUIRE(bw.width == 10);
    REQUIRE(bw.height == 8);
    REQUIRE(bw.data.size() == 80);
}
