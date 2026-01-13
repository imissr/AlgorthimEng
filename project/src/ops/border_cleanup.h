#ifndef ALGENG_BORDER_CLEANUP_H
#define ALGENG_BORDER_CLEANUP_H


#include "src/image/image_gray.h"

class border_cleanup {
public:
    static GrayImage whitenEdges(const GrayImage& in, int borderWidth);

    // Only whiten border pixels that are darker than (thresholdFrac * maxval)
    static GrayImage whitenDarkEdges(const GrayImage& in, int borderWidth, double thresholdFrac);

};

#endif
