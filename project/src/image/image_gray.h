#ifndef ALGENG_IMAGE_GRAY_H
#define ALGENG_IMAGE_GRAY_H

#include <vector>

struct GrayImage {
    int width = 0;
    int height = 0;
    int maxval = 255;
    std::vector<int> data; // size = width*height (0..maxval)

    int& at(int x, int y) { return data[y * width + x]; }
    int  at(int x, int y) const { return data[y * width + x]; }
};

#endif
