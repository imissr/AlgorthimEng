#include "args_parser.h"
#include <stdexcept>
#include <string>

Args parseArgs(int argc, char** argv) {
    Args a;
/*
    *--verbose
    Print extra logs (steps, parameter values, progress).

    --median 1
    Apply a median filter. The 1 is typically the radius/window size (or “strength”). Median filtering reduces salt-and-pepper noise.

    --bg-radius 30
    Background estimation/removal using a neighborhood of radius 30 pixels (often used to correct uneven lighting/shadows).

    --target 0.90
    Target level for normalization (often “make the bright background reach ~90% intensity”).

    --contrast
    Enable contrast enhancement (some default method).

    --contrast-pct 1 99
    Contrast stretching using percentiles: map the 1st percentile to black and the 99th percentile to white (clipping extremes).

    --otsu
    Use Otsu’s thresholding (global threshold). Typically converts to black/white (binarization).

    --sauvola <radius> <k>
    Use Sauvola adaptive thresholding (local threshold).
    radius = window radius; k = tuning factor (often around 0.2–0.5).

    --open
    Apply morphological opening (erode then dilate). Removes small bright noise / breaks thin connections.

    --close
    Apply morphological closing (dilate then erode). Fills small holes / connects nearby dark regions.

    --border <w>
    Add a border of width w pixels (often white or a default color).

    --border-dark <w> <thrFrac>
    Add a border (width w) but decide “dark vs light” based on a threshold fraction thrFrac (e.g., if edges are dark, add dark border or handle dark background scans).
 **/
    if (argc < 3) {
        throw std::runtime_error(
            "Usage: enhance <input.ppm> <output.ppm> [options]\n"
            "\n"
            "Options:\n"
            "  --verbose\n"
            "  --threads <N>                 Set OpenMP threads (optional)\n"
            "\n"
            "Preprocessing:\n"
            "  --median <0|1>                1 = median3x3, 0 = off\n"
            "  --bg-radius <R>               background blur radius (0 = off)\n"
            "  --target <0..1>               paper target fraction (default 0.90)\n"
            "  --contrast-pct <low> <high>   percentile stretch (e.g. 1 99)\n"
            "\n"
            "Binarization (choose ONE):\n"
            "  --otsu\n"
            "  --sauvola <radius> <k>        e.g. --sauvola 25 0.34\n"
            "  --nick <radius> <k>           e.g. --nick 25 -0.10\n"
            "\n"
            "Morphology (binary):\n"
            "  --open                        open3x3 (erode then dilate)\n"
            "  --close                       close3x3 (dilate then erode)\n"
            "\n"
            "Border cleanup:\n"
            "  --border <w>                  whiten fixed border width\n"
            "  --border-dark <w> <thrFrac>   whiten only dark border pixels (e.g. 15 0.6)\n"
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

        }
     else if (arg == "--threads") {
        if (i + 1 >= argc) throw std::runtime_error("--threads needs a value");
        a.threads = std::stoi(argv[++i]);
    }
        else if (arg == "--nick") {
            if (i + 2 >= argc) throw std::runtime_error("--nick needs: <radius> <k> (k usually -0.1)");
            a.nick = true;
            a.nickRadius = std::stoi(argv[++i]);
            a.nickK = std::stod(argv[++i]);
        }else {
            throw std::runtime_error("Unknown argument: " + arg);
        }
    }

    int threshCount =
        (a.otsu ? 1 : 0) +
        (a.sauvola ? 1 : 0) +
        (a.nick ? 1 : 0);

    if (threshCount > 1) {
        throw std::runtime_error(
            "Choose only one binarization: --otsu OR --sauvola OR --nick"
        );
    }

    return a;
};