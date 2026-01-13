#include "args_parser.h"
#include <stdexcept>
#include <string>

Args parseArgs(int argc, char** argv) {
    Args a;

    if (argc < 3) {
        throw std::runtime_error(
            "Usage: enhance <input.ppm> <output.ppm> [--verbose] "
            "[--median 1] [--bg-radius 30] [--target 0.90] "
            "[--contrast] [--contrast-pct 1 99] "
            "[--otsu | --sauvola <radius> <k>] "
            "[--open] [--close] "
            "[--border <w> | --border-dark <w> <thrFrac>]"
        );
    }

    a.input = argv[1];
    a.output = argv[2];

    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--verbose") {
            a.verbose = true;

        } else if (arg == "--median") {
            if (i + 1 >= argc) throw std::runtime_error("--median needs a value");
            a.medianRadius = std::stoi(argv[++i]);

        } else if (arg == "--bg-radius") {
            if (i + 1 >= argc) throw std::runtime_error("--bg-radius needs a value");
            a.bgRadius = std::stoi(argv[++i]);

        } else if (arg == "--target") {
            if (i + 1 >= argc) throw std::runtime_error("--target needs a value (e.g. 0.90)");
            a.targetPaper = std::stod(argv[++i]);

        } else if (arg == "--contrast") {
            a.contrast = true;

        } else if (arg == "--contrast-pct") {
            if (i + 2 >= argc) throw std::runtime_error("--contrast-pct needs two values");
            a.contrastPercentile = true;
            a.lowPct = std::stod(argv[++i]);
            a.highPct = std::stod(argv[++i]);

        // -------- OCR / binarization --------
        } else if (arg == "--otsu") {
            a.otsu = true;

        } else if (arg == "--sauvola") {
            if (i + 2 >= argc) throw std::runtime_error("--sauvola needs: <radius> <k>");
            a.sauvola = true;
            a.sauvolaRadius = std::stoi(argv[++i]);
            a.sauvolaK = std::stod(argv[++i]);

        // -------- morphology --------
        } else if (arg == "--open") {
            a.morphOpen = true;

        } else if (arg == "--close") {
            a.morphClose = true;

        // -------- border cleanup --------
        } else if (arg == "--border") {
            if (i + 1 >= argc) throw std::runtime_error("--border needs: <width>");
            a.border = true;
            a.borderWidth = std::stoi(argv[++i]);

        } else if (arg == "--border-dark") {
            if (i + 2 >= argc) throw std::runtime_error("--border-dark needs: <width> <thrFrac>");
            a.borderDark = true;
            a.borderDarkWidth = std::stoi(argv[++i]);
            a.borderDarkThresholdFrac = std::stod(argv[++i]);

        } else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }

    // prevent using both thresholds at once
    if (a.otsu && a.sauvola) {
        throw std::runtime_error("Choose only one binarization: --otsu OR --sauvola");
    }

    return a;
}
