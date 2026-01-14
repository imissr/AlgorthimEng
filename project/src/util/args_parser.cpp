#include "args_parser.h"
#include <stdexcept>
#include <string>

Args parseArgs(int argc, char** argv) {
    Args a;


    //C:\Users\khale\CLionProjects\AlgEng\project\data\in.ppm C:\Users\khale\CLionProjects\AlgEng\project\data\out.ppm --median 1 --bg-radius 45 --contrast-pct 1 99 --otsu --border-dark 15 0.6 --threads 8
    // C:\Users\khale\CLionProjects\AlgEng\project\data\in.ppm C:\Users\khale\CLionProjects\AlgEng\project\data\out.ppm --median 1 --bg-radius 45 --contrast-pct 1 99 --sauvola 25 0.21 --threads 8
    //C:\Users\khale\CLionProjects\AlgEng\project\data\in.ppm C:\Users\khale\CLionProjects\AlgEng\project\data\out.ppm --median 1 --nick 10 -0.35 --threads 8 --border-dark 15 0.6
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