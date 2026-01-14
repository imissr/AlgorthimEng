#ifndef ALGENG_UTIL_ARGS_PARSER_H
#define ALGENG_UTIL_ARGS_PARSER_H

#include <string>

struct Args {
    std::string input;
    std::string output;

    bool verbose = false;

    int medianRadius = 0;        // 0 = off, 1 = 3x3
    int bgRadius = 0;            // 0 = off
    double targetPaper = 0.90;   // fraction of maxval

    bool contrast = false;
    bool contrastPercentile = false;
    double lowPct = 1.0;
    double highPct = 99.0;

    // ---- OCR / binarization ----
    bool otsu = false;

    bool sauvola = false;
    int sauvolaRadius = 25;      // typical 15..30
    double sauvolaK = 0.34;      // typical 0.2..0.5

    // ---- morphology (binary) ----
    bool morphOpen = false;
    bool morphClose = false;

    // ---- border cleanup ----
    bool border = false;
    int borderWidth = 15;

    bool borderDark = false;
    int borderDarkWidth = 15;
    double borderDarkThresholdFrac = 0.60; // 0..1


    bool nick = false;
    int nickRadius = 25;
    double nickK = -0.10;
    int threads = 0; // 0 = use OpenMP default
};

Args parseArgs(int argc, char** argv);

#endif
