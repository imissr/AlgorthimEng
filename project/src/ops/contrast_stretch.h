//
// Created by khale on 13.01.2026.
//

#ifndef ALGENG_CONTRAST_STRETCH_H
#define ALGENG_CONTRAST_STRETCH_H
#include "image/image_gray.h"


class contrast_stretch {
public:
    // Simple full-range stretch using global min/max
    static GrayImage apply(const GrayImage& in);
    static GrayImage applyPercentile(const GrayImage& in,
                                 double lowPct,
                                 double highPct);



};


#endif //ALGENG_CONTRAST_STRETCH_H