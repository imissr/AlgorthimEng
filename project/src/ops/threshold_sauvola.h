#ifndef ALGENG_THRESHOLD_SAUVOLA_H
#define ALGENG_THRESHOLD_SAUVOLA_H


#include "src/image/image_gray.h"

class threshold_sauvola {
public:
    // windowRadius: e.g. 15..30
    // k: e.g. 0.2..0.5
    static GrayImage binarize(const GrayImage& in, int windowRadius, double k);

};

#endif
