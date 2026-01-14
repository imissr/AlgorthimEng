//
// Created by khale on 13.01.2026.
//

#ifndef ALGENG_BACKGROUND_ESTIMATE_H
#define ALGENG_BACKGROUND_ESTIMATE_H
#include "image/image_gray.h"


class background_estimate {
public:
    // radius: 0 = no blur, 10..50 typical for scanned documents
    static GrayImage boxBlurSeparable(const GrayImage& in, int radius);

};

#endif //ALGENG_BACKGROUND_ESTIMATE_H