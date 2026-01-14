#ifndef ALGENG_COLORSPACE_H
#define ALGENG_COLORSPACE_H

#include "image_gray.h"
#include "io/ppm_reader.h"

GrayImage rgbToGray(const ppm_reader::PPMImage& rgb);
ppm_reader::PPMImage grayToRgb(const GrayImage& g); // optional (to save as P3)

#endif
