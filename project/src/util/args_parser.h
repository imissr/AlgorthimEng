#ifndef ALGENG_UTIL_ARGS_PARSER_H
#define ALGENG_UTIL_ARGS_PARSER_H

#include <string>

struct Args {
    std::string input;
    std::string output;

    bool verbose = false;

    int medianRadius = 0;     // 0 = off, 1 = 3x3
    int bgRadius = 0;         // 0 = off
    double targetPaper = 0.90; // fraction of maxval

    bool contrast = false;
    bool contrastPercentile = false;
    double lowPct = 1.0;
    double highPct = 99.0;
};

Args parseArgs(int argc, char** argv);

#endif
