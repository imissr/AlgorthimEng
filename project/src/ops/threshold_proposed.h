#pragma once
#include "image/image_gray.h"

namespace threshold_proposed {

    // Window size is still required (paper used 20x20).
    // Here we use radius r => window size (2r+1)x(2r+1).
    GrayImage binarize(const GrayImage& in, int r = 10);

}
