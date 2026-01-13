#include <fstream>
#include <iostream>
#include <string>

#include "src/util/args_parser.h"

#include "src/image/colorspace.h"
#include "src/io/ppm_reader.h"
#include "src/io/ppm_writer.h"
#include "src/ops/background_estimate.h"
#include "src/ops/background_remove.h"
#include "src/ops/contrast_stretch.h"
#include "src/ops/denoise_median.h"

int main(int argc, char **argv)
{
    try
    {
        Args args = parseArgs(argc, argv);

        std::ifstream file(args.input);
        if (!file.is_open())
        {
            std::cerr << "Cannot open: " << args.input << "\n";
            return 1;
        }

        ppm_reader reader(file);
        auto image = reader.readPPMImage();

        if (args.verbose) {
            std::cout << "Loaded PPM: " << image.width << "x" << image.height
                      << " maxval=" << image.maxColorValue << "\n";
        }

        auto gray = rgbToGray(image);

        // median
        auto den = gray;
        if (args.medianRadius > 0) {
            den = median3x3(gray); // radius=1 -> 3x3
            if (args.verbose) std::cout << "Applied median3x3\n";
        }

        // background correction
        auto clean = den;
        if (args.bgRadius > 0) {
            auto bg = background_estimate::boxBlurSeparable(den, args.bgRadius);

            int target = (int)(args.targetPaper * den.maxval);
            if (target < 0) target = 0;
            if (target > den.maxval) target = den.maxval;

            clean = background_remove::remove(den, bg, target);
            if (args.verbose) {
                std::cout << "Background remove: radius=" << args.bgRadius
                          << " target=" << target << "\n";
            }
        }

        // contrast stretch
        auto stretched = clean;
        if (args.contrastPercentile) {
            stretched = contrast_stretch::applyPercentile(clean, args.lowPct, args.highPct);
            if (args.verbose) {
                std::cout << "Contrast percentile: " << args.lowPct << "%.." << args.highPct << "%\n";
            }
        }

        auto outRgb = grayToRgb(stretched);
        ppm_writer::writePPMImage(args.output, outRgb);

        if (args.verbose) {
            std::cout << "Wrote: " << args.output << "\n";
            std::cout << "RGB data size: " << image.data.size() << "\n";
        }

        return 0;
    }
    catch (const std::exception &ex)
    {
        std::cerr << ex.what() << "\n";
        return 1;
    }
}
