//
// Created by khale on 13.01.2026.
//

#ifndef ALGENG_BACKGROUND_REMOVE_H
#define ALGENG_BACKGROUND_REMOVE_H
#include "src/image/image_gray.h"


class background_remove {
public:
    // target: desired paper brightness (e.g. (int)(0.90 * maxval))
    static GrayImage remove(const GrayImage& denoised,
                            const GrayImage& background,
                            int target);
};



#endif //ALGENG_BACKGROUND_REMOVE_H