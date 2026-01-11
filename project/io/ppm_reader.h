//
// Created by khale on 11.01.2026.
//

#ifndef ALGENG_PPM_READER_H
#define ALGENG_PPM_READER_H
#include <iosfwd>
#include <vector>

#include "token_reader.h"


class ppm_reader {
public:
    ppm_reader(std::istream& inputStream) ;

    struct PPMImage {
        int width = 0;
        int height = 0;
        int maxColorValue = 255;
        std::vector<int> data ;
    };

    PPMImage readPPMImage(const std::string &fileName);

private:

TokenReader tokenReader;




};


#endif //ALGENG_PPM_READER_H