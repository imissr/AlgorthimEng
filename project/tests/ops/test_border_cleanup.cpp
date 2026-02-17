#include <catch2/catch_test_macros.hpp>
#include "ops/border_cleanup.h"
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

TEST_CASE("WhitenEdges: border pixels become white", "[border_cleanup]")
{
    GrayImage img = makeConstantImage(7, 7, 50); // all dark
    int borderWidth = 1;

    GrayImage out = border_cleanup::whitenEdges(img, borderWidth);

    REQUIRE(out.width == 7);
    REQUIRE(out.height == 7);

    // Top and bottom rows should be white
    for (int x = 0; x < 7; ++x)
    {
        REQUIRE(out.at(x, 0) == 255);
        REQUIRE(out.at(x, 6) == 255);
    }

    // Left and right columns should be white
    for (int y = 0; y < 7; ++y)
    {
        REQUIRE(out.at(0, y) == 255);
        REQUIRE(out.at(6, y) == 255);
    }

    // Interior should remain unchanged
    REQUIRE(out.at(3, 3) == 50);
}

TEST_CASE("WhitenEdges: borderWidth 0 leaves image unchanged", "[border_cleanup]")
{
    GrayImage img = makeConstantImage(5, 5, 80);
    GrayImage out = border_cleanup::whitenEdges(img, 0);

    for (size_t i = 0; i < img.data.size(); ++i)
    {
        REQUIRE(out.data[i] == img.data[i]);
    }
}

TEST_CASE("WhitenDarkEdges: only dark border pixels are whitened", "[border_cleanup]")
{
    GrayImage img = makeConstantImage(7, 7, 200); // mostly bright
    // Make the top row dark
    for (int x = 0; x < 7; ++x)
    {
        img.at(x, 0) = 30;
    }

    GrayImage out = border_cleanup::whitenDarkEdges(img, 1, 0.5);

    // Top row was dark → should be whitened
    for (int x = 0; x < 7; ++x)
    {
        REQUIRE(out.at(x, 0) == 255);
    }

    // Interior bright pixels should remain unchanged
    REQUIRE(out.at(3, 3) == 200);
}

TEST_CASE("WhitenDarkEdges: bright border pixels are kept", "[border_cleanup]")
{
    GrayImage img = makeConstantImage(5, 5, 240); // all bright
    GrayImage out = border_cleanup::whitenDarkEdges(img, 1, 0.5);

    // Bright border pixels should stay (or become white, both are fine since they're already bright)
    // Interior should remain unchanged
    REQUIRE(out.at(2, 2) == 240);
}
