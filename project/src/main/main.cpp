#include <fstream>
#include <iostream>
#include <string>

#include "ops/morphology.h"
#include "ops/threshold_nick.h"
#include "ops/threshold_otsu.h"
#include "ops/threshold_sauvola.h"
#include "util/args_parser.h"

#include "image/colorspace.h"
#include "io/ppm_reader.h"
#include "io/ppm_writer.h"
#include "ops/background_estimate.h"
#include "ops/background_remove.h"
#include "ops/contrast_stretch.h"
#include "ops/denoise_median.h"
#include "ops/threshold_su.h"
#include "ops/border_cleanup.h"
#include <omp.h>

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

        auto result = stretched;

        if (args.otsu) {
            result = threshold_otsu::binarize(result);
        } else if (args.sauvola) {
            result = threshold_sauvola::binarize(result, args.sauvolaRadius, args.sauvolaK);
        } else if (args.nick) {
            result = threshold_nick::binarize(result, args.nickRadius, args.nickK);
            if (args.verbose) {
                std::cout << "Applied NICK: r=" << args.nickRadius << " k=" << args.nickK << "\n";
            }
        } else if (args.su) {   // <<< ADD THIS
            result = threshold_su::binarize(result, args.suRadius, args.suNmin, args.suEps);
            if (args.verbose) {
                std::cout << "Applied SU: r=" << args.suRadius
                          << " Nmin=" << args.suNmin
                          << " eps=" << args.suEps << "\n";
            }
        }

        // morphology (binary cleanup)
        if (args.morphOpen) {
            result = morphology::open3x3(result);
            if (args.verbose) std::cout << "Applied morphology open3x3\n";
        }
        if (args.morphClose) {
            result = morphology::close3x3(result);
            if (args.verbose) std::cout << "Applied morphology close3x3\n";
        }
        if (args.threads > 0) {
            omp_set_num_threads(args.threads);
            if (args.verbose) std::cout << "OpenMP threads: " << args.threads << "\n";
        }

        // border cleanup
        if (args.borderDark) {
            result = border_cleanup::whitenDarkEdges(result, args.borderDarkWidth, args.borderDarkThresholdFrac);
            if (args.verbose) {
                std::cout << "Applied border-dark: w=" << args.borderDarkWidth
                          << " thr=" << args.borderDarkThresholdFrac << "\n";
            }
        } else if (args.border) {
            result = border_cleanup::whitenEdges(result, args.borderWidth);
            if (args.verbose) std::cout << "Applied border whiten: w=" << args.borderWidth << "\n";
        }

        auto outRgb = grayToRgb(result);
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
