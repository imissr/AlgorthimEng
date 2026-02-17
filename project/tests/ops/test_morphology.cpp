#include <catch2/catch_test_macros.hpp>
#include "ops/morphology.h"
#include "image/image_gray.h"

// Morphology operates on BINARY images: 0 = foreground (black), maxval = background (white).
// erode3x3:  pixel → 0 only if ALL 3x3 neighbors are 0  (shrinks foreground)
// dilate3x3: pixel → 0 if ANY 3x3 neighbor is 0          (grows foreground)

static GrayImage makeBinaryImage(int w, int h, int val)
{
    GrayImage img;
    img.width = w;
    img.height = h;
    img.maxval = 255;
    img.data.assign((size_t)w * (size_t)h, val);
    return img;
}

// ---------- Erosion ----------

TEST_CASE("Erode: all-black image stays all-black", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 0);
    GrayImage out = morphology::erode3x3(img);

    REQUIRE(out.data.size() == img.data.size());
    for (int v : out.data)
    {
        REQUIRE(v == 0);
    }
}

TEST_CASE("Erode: all-white image stays all-white", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 255);
    GrayImage out = morphology::erode3x3(img);

    REQUIRE(out.data.size() == img.data.size());
    for (int v : out.data)
    {
        REQUIRE(v == 255);
    }
}

TEST_CASE("Erode: single black pixel surrounded by white becomes white", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 255);
    img.at(2, 2) = 0; // single black pixel

    GrayImage out = morphology::erode3x3(img);

    // Erode requires ALL neighbors to be 0; center had white neighbors → becomes white
    REQUIRE(out.at(2, 2) == 255);
}

TEST_CASE("Erode: shrinks black foreground region", "[morphology]")
{
    // 5x5 all black with white border
    GrayImage img = makeBinaryImage(5, 5, 0);
    for (int x = 0; x < 5; ++x)
    {
        img.at(x, 0) = 255;
        img.at(x, 4) = 255;
    }
    for (int y = 0; y < 5; ++y)
    {
        img.at(0, y) = 255;
        img.at(4, y) = 255;
    }
    // Inner 3x3 is black, border is white

    GrayImage out = morphology::erode3x3(img);

    // Pixels adjacent to white border (1,1) have white neighbors → eroded to white
    REQUIRE(out.at(1, 1) == 255);
    REQUIRE(out.at(3, 3) == 255);
    // Center (2,2) has ALL 3x3 neighbors black → stays black
    REQUIRE(out.at(2, 2) == 0);
}

// ---------- Dilation ----------

TEST_CASE("Dilate: all-white image stays all-white", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 255);
    GrayImage out = morphology::dilate3x3(img);

    REQUIRE(out.data.size() == img.data.size());
    for (int v : out.data)
    {
        REQUIRE(v == 255);
    }
}

TEST_CASE("Dilate: all-black image stays all-black", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 0);
    GrayImage out = morphology::dilate3x3(img);

    REQUIRE(out.data.size() == img.data.size());
    for (int v : out.data)
    {
        REQUIRE(v == 0);
    }
}

TEST_CASE("Dilate: single black pixel spreads to 3x3", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 255);
    img.at(2, 2) = 0; // single black pixel

    GrayImage out = morphology::dilate3x3(img);

    // Dilation: ANY neighbor is 0 → pixel becomes 0
    // Center and all direct neighbors should be black
    REQUIRE(out.at(2, 2) == 0);
    REQUIRE(out.at(1, 2) == 0);
    REQUIRE(out.at(3, 2) == 0);
    REQUIRE(out.at(2, 1) == 0);
    REQUIRE(out.at(2, 3) == 0);
    // Diagonal neighbors also in 3x3
    REQUIRE(out.at(1, 1) == 0);
    REQUIRE(out.at(3, 3) == 0);

    // Far corners should remain white
    REQUIRE(out.at(0, 0) == 255);
    REQUIRE(out.at(4, 4) == 255);
}

// ---------- Open (erode then dilate) ----------

TEST_CASE("Open: removes single black noise pixel", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 255);
    img.at(2, 2) = 0; // single black noise pixel

    GrayImage out = morphology::open3x3(img);

    // Erode removes isolated black pixel (not all neighbors black) → all white
    // Dilate on all-white → still all white
    REQUIRE(out.at(2, 2) == 255);
}

TEST_CASE("Open: constant black image is unchanged", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 0);
    GrayImage out = morphology::open3x3(img);

    for (int v : out.data)
    {
        REQUIRE(v == 0);
    }
}

TEST_CASE("Open: constant white image is unchanged", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 255);
    GrayImage out = morphology::open3x3(img);

    for (int v : out.data)
    {
        REQUIRE(v == 255);
    }
}

// ---------- Close (dilate then erode) ----------

TEST_CASE("Close: fills single white hole in black region", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 0);
    img.at(2, 2) = 255; // single white hole

    GrayImage out = morphology::close3x3(img);

    // Dilate on all-black-except-center: all neighbors of (2,2) are black → anyBlack
    // So dilate fills the hole → all black. Erode on all-black → all-black.
    REQUIRE(out.at(2, 2) == 0);
}

TEST_CASE("Close: constant black image is unchanged", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 0);
    GrayImage out = morphology::close3x3(img);

    for (int v : out.data)
    {
        REQUIRE(v == 0);
    }
}

TEST_CASE("Close: constant white image is unchanged", "[morphology]")
{
    GrayImage img = makeBinaryImage(5, 5, 255);
    GrayImage out = morphology::close3x3(img);

    for (int v : out.data)
    {
        REQUIRE(v == 255);
    }
}

TEST_CASE("Morphology: output dimensions always match input", "[morphology]")
{
    GrayImage img = makeBinaryImage(8, 6, 0);

    GrayImage e = morphology::erode3x3(img);
    GrayImage d = morphology::dilate3x3(img);
    GrayImage o = morphology::open3x3(img);
    GrayImage c = morphology::close3x3(img);

    REQUIRE(e.width == 8);
    REQUIRE(e.height == 6);
    REQUIRE(d.width == 8);
    REQUIRE(d.height == 6);
    REQUIRE(o.width == 8);
    REQUIRE(o.height == 6);
    REQUIRE(c.width == 8);
    REQUIRE(c.height == 6);
}

TEST_CASE("Morphology: output is always binary", "[morphology]")
{
    GrayImage img = makeBinaryImage(7, 7, 255);
    // Draw a thick black cross
    for (int i = 0; i < 7; ++i)
    {
        img.at(3, i) = 0;
        img.at(i, 3) = 0;
    }

    GrayImage e = morphology::erode3x3(img);
    GrayImage d = morphology::dilate3x3(img);
    GrayImage o = morphology::open3x3(img);
    GrayImage c = morphology::close3x3(img);

    for (int v : e.data)
    {
        REQUIRE((v == 0 || v == 255));
    }
    for (int v : d.data)
    {
        REQUIRE((v == 0 || v == 255));
    }
    for (int v : o.data)
    {
        REQUIRE((v == 0 || v == 255));
    }
    for (int v : c.data)
    {
        REQUIRE((v == 0 || v == 255));
    }
}
