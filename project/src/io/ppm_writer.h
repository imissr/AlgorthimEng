//
// Created by khale on 13.01.2026.
//

#ifndef ALGENG_PPM_WRITER_H
#define ALGENG_PPM_WRITER_H
#include "ppm_reader.h"


class ppm_writer {
public:

    static void writePPMImage(const std::string &fileName, const ppm_reader::PPMImage &img);

     static void writePPMImage(std::ostream& out,
                              const ppm_reader::PPMImage& img);};


#endif //ALGENG_PPM_WRITER_H