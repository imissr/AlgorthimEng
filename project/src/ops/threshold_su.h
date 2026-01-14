// threshold_su.h
// Su, Lu, Tan (2010) - contrast-based binarization using local max/min + Otsu on contrast
// Created by khale on 14.01.2026.

#pragma once

#include "image/image_gray.h"

class threshold_su {
public:
    /**
     * Contrast-based historical document binarization (Su, Lu, Tan 2010).
     *
     * Steps:
     *  1) Build contrast image D(x,y) from local max/min in a 3x3 window.
     *  2) Otsu threshold on D to detect high-contrast (stroke-boundary) pixels (edge mask).
     *  3) For each pixel, compute stats (mean/stddev) of ORIGINAL intensities of edge pixels
     *     in a neighborhood window of radius r (size (2r+1)^2), and classify:
     *       if Ne >= Nmin and I(x,y) <= mean + stddev/2 -> black, else white.
     *
     * @param in    Input grayscale image.
     * @param r     Neighborhood window radius (>0).
     * @param Nmin  Minimum number of edge pixels in the window to enable classification (>0).
     * @param eps   Small epsilon to avoid division by zero in contrast computation.
     */
    static GrayImage binarize(const GrayImage& in, int r, int Nmin, double eps = 1e-6);
};
