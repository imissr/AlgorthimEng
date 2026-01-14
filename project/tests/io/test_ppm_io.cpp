#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>

#include "io/token_reader.h"
#include "io/ppm_reader.h"
#include "io/ppm_writer.h"

TEST_CASE("TokenReader skips whitespace and comments", "[io][token_reader]") {
    std::istringstream in(
        "   # first comment\n"
        "P3   \n"
        "  # between tokens\n"
        "2\t1\n"
        "255\n"
        "0 0 0  255 255 255\n"
    );

    TokenReader tr(in);

    REQUIRE(tr.nextToken() == "P3");
    REQUIRE(tr.nextIntToken() == 2);
    REQUIRE(tr.nextIntToken() == 1);
    REQUIRE(tr.nextIntToken() == 255);
    REQUIRE(tr.nextIntToken() == 0);
    REQUIRE(tr.nextIntToken() == 0);
    REQUIRE(tr.nextIntToken() == 0);
    REQUIRE(tr.nextIntToken() == 255);
    REQUIRE(tr.nextIntToken() == 255);
    REQUIRE(tr.nextIntToken() == 255);
}

TEST_CASE("PPM reader parses valid P3 with comments", "[io][ppm_reader]") {
    std::istringstream in(
        "P3\n"
        "# comment after magic\n"
        "2 1\n"
        "255\n"
        "0 0 0   255 255 255\n"
    );

    ppm_reader r(in);
    auto img = r.readPPMImage();

    REQUIRE(img.width == 2);
    REQUIRE(img.height == 1);
    REQUIRE(img.maxColorValue == 255);
    REQUIRE(img.data.size() == 6);

    REQUIRE(img.data[0] == 0);
    REQUIRE(img.data[1] == 0);
    REQUIRE(img.data[2] == 0);
    REQUIRE(img.data[3] == 255);
    REQUIRE(img.data[4] == 255);
    REQUIRE(img.data[5] == 255);
}

TEST_CASE("PPM reader rejects non-P3 magic", "[io][ppm_reader]") {
    std::istringstream in(
        "P6\n"
        "1 1\n"
        "255\n"
        "0 0 0\n"
    );

    ppm_reader r(in);
    REQUIRE_THROWS_AS(r.readPPMImage(), std::runtime_error);
}

TEST_CASE("PPM reader rejects out-of-range pixel values", "[io][ppm_reader]") {
    std::istringstream in(
        "P3\n"
        "1 1\n"
        "10\n"
        "0 0 11\n" // 11 > maxval=10
    );

    ppm_reader r(in);
    REQUIRE_THROWS_AS(r.readPPMImage(), std::runtime_error);
}

TEST_CASE("PPM writer writes valid P3 and clamps values", "[io][ppm_writer]") {
    ppm_reader::PPMImage img;
    img.width = 2;
    img.height = 1;
    img.maxColorValue = 10;
    img.data = {
        -5, 0, 5,     // will clamp to 0 0 5
        10, 11, 3     // will clamp to 10 10 3
    };

    std::ostringstream out;
    ppm_writer::writePPMImage(out, img);

    // Read back using ppm_reader to verify round-trip content
    std::istringstream in2(out.str());
    ppm_reader r2(in2);
    auto img2 = r2.readPPMImage();

    REQUIRE(img2.width == img.width);
    REQUIRE(img2.height == img.height);
    REQUIRE(img2.maxColorValue == img.maxColorValue);
    REQUIRE(img2.data.size() == img.data.size());

    // Clamped expectations
    REQUIRE(img2.data[0] == 0);
    REQUIRE(img2.data[1] == 0);
    REQUIRE(img2.data[2] == 5);
    REQUIRE(img2.data[3] == 10);
    REQUIRE(img2.data[4] == 10);
    REQUIRE(img2.data[5] == 3);
}

TEST_CASE("PPM writer throws on wrong data size", "[io][ppm_writer]") {
    ppm_reader::PPMImage img;
    img.width = 2;
    img.height = 2;
    img.maxColorValue = 255;

    // Should be 2*2*3=12, but we give 3
    img.data = {0, 0, 0};

    std::ostringstream out;
    REQUIRE_THROWS_AS(ppm_writer::writePPMImage(out, img), std::runtime_error);
}
