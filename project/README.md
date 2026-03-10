# Scanned Image Enhancer (PPM P3)

A **configurable pipeline** CLI tool that improves scanned document quality before printing or OCR. It reads **PPM P3 (ASCII) images**, applies a sequence of image-processing operations, and writes a cleaned PPM P3 output.

The pipeline follows this processing order:

**Denoise → Background Removal → Contrast Stretch → Binarization → Morphology → Border Cleanup**

---

## Features

- **Median denoising** — removes salt-and-pepper noise / dust specks
- **Background estimation & removal** — flattens uneven lighting using separable box blur
- **Percentile contrast stretch** — robust histogram-based stretch, resilient to outliers
- **5 binarization methods** — Otsu, Sauvola, NICK, Su, and a custom Proposed method
- **Morphological operations** — binary open/close with 3×3 structuring element
- **Border cleanup** — whiten fixed-width borders or only dark border pixels
- **OpenMP parallelization** in nearly every operation
- **Integral images** for O(1) per-pixel window statistics (Sauvola, NICK, Su, Proposed)
- All algorithms use **`long double`** accumulators to avoid precision loss on large images

---

## Project Architecture

```
project/
├── CMakeLists.txt
├── data/                          # Sample PPM images
│   ├── in.ppm
│   └── out.ppm
├── src/
│   ├── main/
│   │   └── main.cpp              # CLI entry point, orchestrates the full pipeline
│   ├── image/
│   │   ├── image_gray.h/.cpp     # GrayImage struct (width, height, maxval, flat data)
│   │   └── colorspace.h/.cpp     # RGB ↔ Grayscale conversion
│   ├── io/
│   │   ├── ppm_reader.h/.cpp     # PPM P3 parser
│   │   ├── ppm_writer.h/.cpp     # PPM P3 writer
│   │   └── token_reader.h/.cpp   # Whitespace/comment-aware tokenizer
│   ├── ops/                       # All image processing operations
│   │   ├── denoise_median.h/.cpp          # 3×3 median filter
│   │   ├── background_estimate.h/.cpp     # Separable box blur
│   │   ├── background_remove.h/.cpp       # Subtract background + target level
│   │   ├── contrast_stretch.h/.cpp        # Min/max and percentile stretch
│   │   ├── threshold_otsu.h/.cpp          # Otsu global thresholding
│   │   ├── threshold_sauvola.h/.cpp       # Sauvola adaptive thresholding
│   │   ├── threshold_nick.h/.cpp          # NICK adaptive thresholding
│   │   ├── threshold_su.h/.cpp            # Su, Lu, Tan (2010) binarization
│   │   ├── threshold_proposed.h/.cpp      # Custom hybrid adaptive method
│   │   ├── morphology.h/.cpp              # Binary open/close (3×3)
│   │   └── border_cleanup.h/.cpp          # Edge whitening
│   └── util/
│       ├── args_parser.h/.cpp     # CLI argument parser
│       ├── logging.h/.cpp         # Logging utilities
│       ├── timing.h/.cpp          # Timing utilities
│       └── clamp.h                # Clamp helper
└── tests/                         # Catch2 unit tests
    ├── io/
    │   └── test_ppm_io.cpp        # PPM reader/writer tests
    └── ops/
        ├── test_otsu.cpp          # Otsu thresholding tests
        ├── test_sauvola.cpp       # Sauvola thresholding tests
        └── test_median.cpp        # Median filter tests
```

---

## Build

Requires: **CMake ≥ 4.1**, a **C++23** compiler, and **OpenMP**.

```bash
mkdir build && cd build
cmake ..
make
```

The executable is built at `build/project/project_executable`.

---

## Usage

```
./project/project_executable <input.ppm> <output.ppm> [options]
```

---

## Complete Parameter Reference

### Basic Options

| Flag | Description |
|------|-------------|
| `--verbose` | Enable detailed output |
| `--threads <N>` | Set OpenMP threads (0 = auto) |

### Preprocessing Operations

| Flag | Description |
|------|-------------|
| `--median <0\|1>` | Apply median filter (1 = 3×3, 0 = off) |
| `--bg-radius <R>` | Background blur radius (0 = off) |
| `--target <0..1>` | Paper target fraction (default: 0.90) |
| `--contrast-pct <low> <high>` | Percentile stretch (e.g., 1 99) |

### Binarization Methods (choose ONE)

| Flag | Description |
|------|-------------|
| `--otsu` | Otsu's automatic global thresholding |
| `--sauvola <radius> <k>` | Sauvola adaptive thresholding (e.g., `--sauvola 25 0.34`) |
| `--nick <radius> <k>` | NICK adaptive thresholding (e.g., `--nick 25 -0.10`) |
| `--su [radius Nmin [eps]]` | Su adaptive thresholding (e.g., `--su 25 30 1e-6`) |
| `--proposed <radius>` | Custom proposed method (e.g., `--proposed 10`) |

> **Window size:** The local window is $(2r+1) \times (2r+1)$ where $r$ is the radius. For example, `r = 7` gives a $15 \times 15$ window.

### Morphological Operations (binary cleanup)

| Flag | Description |
|------|-------------|
| `--open` | Opening: erode then dilate (3×3) — removes small foreground noise |
| `--close` | Closing: dilate then erode (3×3) — fills small holes in foreground |

### Border Cleanup

| Flag | Description |
|------|-------------|
| `--border <width>` | Whiten fixed border width |
| `--border-dark <width> <thrFrac>` | Whiten only dark border pixels (e.g., `--border-dark 15 0.6`) |

---

## Example Usage

### Basic Otsu thresholding
```bash
./project/project_executable input.ppm output.ppm --otsu
```

### Advanced pipeline with all preprocessing steps
Window size = $2r+1 = 15$, $R$ is fixed at 128 (dynamic range of standard deviation).
```bash
./project/project_executable input.ppm output.ppm \
    --median 1 \
    --bg-radius 45 \
    --contrast-pct 1 99 \
    --sauvola 7 0.20 \
    --open \
    --border-dark 15 0.6 \
    --threads 8 \
    --verbose
```

### NICK thresholding with morphology
```bash
./project/project_executable input.ppm output.ppm \
    --median 1 \
    --nick 9 -0.1 \
    --close \
    --border-dark 15 0.6 \
    --threads 8
```

### Su method with custom parameters
```bash
./project/project_executable input.ppm output.ppm \
    --su 25 30 1e-6 \
    --border 10 \
    --verbose
```

### Proposed method with full preprocessing
```bash
./project/project_executable input.ppm output.ppm \
    --median 1 \
    --bg-radius 45 \
    --contrast-pct 1 99 \
    --proposed 10 \
    --border-dark 15 0.6 \
    --threads 8
```

---

## Parameter Details

### Sauvola Parameters
- **radius:** Neighborhood size (typical: 15–30)
- **k:** Sensitivity parameter (typical: 0.2–0.5)
- Threshold: $T = \mu_W \cdot \left(1 + k \cdot \left(\frac{\sigma_W}{R} - 1\right)\right)$ where $R = 125$

### NICK Parameters
- **radius:** Neighborhood size (typical: 10–30)
- **k:** Sensitivity parameter (typical: −0.1 to −0.2)
- Threshold: $T = \mu_W + k \cdot \sigma_W$

### Su Parameters
- **radius:** Neighborhood size (typical: 20–30)
- **Nmin:** Minimum edge pixels (typical: 20–40)
- **eps:** Contrast epsilon (typical: 1e-6)
- Multi-step: contrast image → Otsu on contrast → stroke width estimation → integral-image classification

### Background Removal
- **bg-radius:** Blur radius for background estimation (typical: 30–50)
- **target:** Target paper brightness as fraction of maxval (typical: 0.85–0.95)

### Border Cleanup
- **border-dark width:** Border width to process (typical: 10–20)
- **border-dark thrFrac:** Threshold fraction for "dark" pixels (typical: 0.5–0.7)

---

## Tests

Tests use the **Catch2** framework and cover comprehensive image processing functionality.

### Requirements
- **Catch2 v3** must be installed system-wide
- **CMake version ≥ 3.20** (fix any `VERSION 4.1` entries to `VERSION 3.20`)

### Running Tests

```bash
# 1. Fix CMake version (if needed)
# Change "cmake_minimum_required(VERSION 4.1)" to "cmake_minimum_required(VERSION 3.20)" 
# in CMakeLists.txt files

# 2. Configure with testing enabled
cd build

cmake .. -DBUILD_TESTING=ON

# 3. Build project and tests
make 

# 4. Run tests (choose one method)
./project/tests/project_tests        

```

### Test Coverage

The test suite includes **56 test cases** with **1038 assertions** covering:

| Test Module | Coverage |
|-------------|----------|
| `test_ppm_io` | TokenReader, PPM parsing, round-trip write, error handling |
| `test_otsu` | Bimodal separation, constant image handling, global thresholding |
| `test_sauvola` | Adaptive thresholding, stroke detection on near-white background |
| `test_nick` | NICK adaptive thresholding algorithm validation |
| `test_threshold_proposed` | Custom hybrid adaptive thresholding method |
| `test_threshold_su` | Su, Lu, Tan (2010) binarization algorithm |
| `test_median` | 3×3 median filter, salt-noise removal verification |
| `test_background_estimate` | Separable box blur background estimation |
| `test_background_remove` | Background subtraction and target level adjustment |
| `test_border_cleanup` | Edge whitening and border processing |
| `test_contrast_stretch` | Percentile-based contrast enhancement |
| `test_morphology` | Binary opening/closing operations with 3×3 structuring element |

All tests validate algorithm correctness, edge cases, and proper handling of various image conditions.
