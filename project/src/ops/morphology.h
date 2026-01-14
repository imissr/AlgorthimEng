#ifndef ALGENG_MORPHOLOGY_H
#define ALGENG_MORPHOLOGY_H
#include "image/image_gray.h"


class morphology {
public:
    static GrayImage erode3x3(const GrayImage& in);
    static GrayImage dilate3x3(const GrayImage& in);
    static GrayImage open3x3(const GrayImage& in);   // erode then dilate
    static GrayImage close3x3(const GrayImage& in);  // dilate then erode

};

#endif
