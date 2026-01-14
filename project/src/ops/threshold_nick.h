#ifndef ALGENG_THRESHOLD_NICK_H
#define ALGENG_THRESHOLD_NICK_H
#include "image/image_gray.h"


class threshold_nick {
public:
    // windowRadius: e.g. 15..30
    // k: typically in [-0.2 .. -0.1], e.g. -0.1
    static GrayImage binarize(const GrayImage& in, int windowRadius, double k);

};

#endif
