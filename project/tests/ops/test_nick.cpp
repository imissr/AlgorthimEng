#include <catch2/catch_test_macros.hpp>
#include "ops/threshold_nick.h"
#include "image/image_gray.h"

// NICK: T = mean + k * stddev.  v <= T → foreground (0), else background (maxval).
// With k < 0 on a constant region (stddev=0): T = mean, so v == T → foreground.

TEST_CASE("NICK: stroke pixels classified as foreground", "[nick]")
{
    GrayImage img;
    img.width = 7;
    img.height = 7;
    img.maxval = 255;
    img.data.assign(49, 240); // near-white paper

    for (int y = 0; y < img.height; ++y)
    {
        img.at(3, y) = 0;
    }

    GrayImage bw = threshold_nick::binarize(img, 2, -0.2);

    REQUIRE(bw.data.size() == img.data.size());

    // Stroke pixels (value 0) should be classified as foreground
    for (int y = 0; y < img.height; ++y)
    {
        REQUIRE(bw.at(3, y) == 0);
    }
}

TEST_CASE("NICK: constant black image stays black", "[nick]")
{
    // Constant 0: mean=0, stddev=0, T=0. v=0 <= 0 → foreground (0)
    GrayImage img;
    img.width = 5;
    img.height = 5;
    img.maxval = 255;
    img.data.assign(25, 0);

    GrayImage bw = threshold_nick::binarize(img, 2, -0.2);

    REQUIRE(bw.data.size() == 25);
    for (int v : bw.data)
    {
        REQUIRE(v == 0);
    }
}

TEST_CASE("NICK: output is always binary", "[nick]")
{
    GrayImage img;
    img.width = 9;
    img.height = 9;
    img.maxval = 255;
    img.data.assign(81, 128);
    img.at(0, 0) = 10;
    img.at(8, 8) = 250;
    img.at(4, 4) = 30;

    GrayImage bw = threshold_nick::binarize(img, 3, -0.2);

    for (int v : bw.data)
    {
        REQUIRE((v == 0 || v == 255));
    }
}

TEST_CASE("NICK: output dimensions match input", "[nick]")
{
    GrayImage img;
    img.width = 10;
    img.height = 8;
    img.maxval = 255;
    img.data.assign(80, 128);

    GrayImage bw = threshold_nick::binarize(img, 3, -0.2);

    REQUIRE(bw.width == 10);
    REQUIRE(bw.height == 8);
    REQUIRE(bw.data.size() == 80);
}

TEST_CASE("NICK: bimodal image separates foreground and background", "[nick]")
{
    // Large enough image with clear bimodal distribution
    GrayImage img;
    img.width = 11;
    img.height = 11;
    img.maxval = 255;
    img.data.assign(121, 220);

    // Draw a thick black block in the center (3x11)
    for (int y = 0; y < 11; ++y)
    {
        for (int x = 4; x <= 6; ++x)
        {
            img.at(x, y) = 10;
        }
    }

    GrayImage bw = threshold_nick::binarize(img, 4, -0.2);

    // Center of the block should be foreground
    REQUIRE(bw.at(5, 5) == 0);
}
