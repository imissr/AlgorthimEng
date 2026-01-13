#include "args_parser.h"
#include <stdexcept>
#include <string>

static bool isFlag(const std::string& s) { return !s.empty() && s[0] == '-'; }

Args parseArgs(int argc, char** argv) {
    Args a;

    if (argc < 3) {
        throw std::runtime_error(
            "Usage: enhance <input.ppm> <output.ppm> [--verbose] "
            "[--median 1] [--bg-radius 30] [--target 0.90] "
            "[--contrast] [--contrast-pct 1 99]"
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
        } else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }

    return a;
}
