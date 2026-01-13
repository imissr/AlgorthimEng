#ifndef ALGENG_THRESHOLD_OTSU_H
#define ALGENG_THRESHOLD_OTSU_H


#include "src/image/image_gray.h"

class threshold_otsu {
public:
    // Returns binary image: 0 (black) or maxval (white)
    static GrayImage binarize(const GrayImage& in);

};

#endif
