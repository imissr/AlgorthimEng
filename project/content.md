Now I have all the information. Here is the detailed summary:

---

## Project Summary: Scanned Image Enhancer (PPM P3)

### Overall Architecture ([projectArchticure.md](project/projectArchticure.md))

A **configurable pipeline** CLI tool that improves scanned document quality before printing or OCR. It reads **PPM P3 (ASCII) images**, applies a sequence of image-processing operations (denoise → background removal → contrast stretch → binarization → morphology → border cleanup), and writes a cleaned PPM P3 output. The architecture is modular: each operation is a small, composable function operating on a `GrayImage`.

---

### Core Data Structure: [image_gray.h](project/src/image/image_gray.h)

A simple struct holding `width`, `height`, `maxval`, and a flat `std::vector<int> data` with row-major layout. Provides `at(x, y)` accessors (mutable and const). All ops consume and return `GrayImage` values.

---

### CLI / Pipeline: [main.cpp](project/src/main/main.cpp)

The `main()` function orchestrates the full pipeline:
1. Parse CLI args (input/output paths, flags for each operation, parameters)
2. Read PPM via `ppm_reader` / `TokenReader`
3. Convert RGB → Grayscale (`rgbToGray`)
4. **Median denoise** (optional, `--medianRadius`)
5. **Background estimation + removal** (optional, `--bgRadius`, `--targetPaper`)
6. **Contrast stretch** (optional, percentile-based)
7. **Binarization** — mutually exclusive: Otsu / Sauvola / NICK / Su / Proposed (selected via flags)
8. **Morphology** — optional open/close
9. **Border cleanup** — optional whitening of edges
10. Convert gray → RGB, write PPM output

Supports `--verbose` logging and `--threads` to set OpenMP thread count.

---

### Operations (all in `project/src/ops/`)

#### 1. [denoise_median.cpp](project/src/ops/denoise_median.cpp)
- **Algorithm:** 3×3 median filter. Collects 9 neighbors (clamped at borders), uses `std::nth_element` to find median.
- **OpenMP:** `#pragma omp parallel for` over rows.
- **Purpose:** Removes salt-and-pepper noise / dust specks.

#### 2. [background_estimate.cpp](project/src/ops/background_estimate.cpp)
- **Algorithm:** Separable box blur (two-pass: horizontal then vertical) with configurable `radius`. Uses a sliding-window running sum for O(n) per row/column. Border pixels use clamped (replicated) boundary.
- **OpenMP:** `#pragma omp parallel for` on the horizontal pass (over rows) and vertical pass (over columns).
- **Purpose:** Estimates the illumination/background of the scan as a large-blur approximation.

#### 3. [background_remove.cpp](project/src/ops/background_remove.cpp)
- **Algorithm:** Simple subtraction: `clean = pixel - background + target`. Clamps result to `[0, maxval]`. The `target` parameter (typically ~90% of maxval) sets the desired paper white level.
- **OpenMP:** None (simple loop).
- **Purpose:** Flattens uneven lighting; makes paper uniformly bright.

#### 4. [contrast_stretch.cpp](project/src/ops/contrast_stretch.cpp)
- **Two modes:**
  - `apply()`: Full-range linear stretch using actual min/max pixel values.
  - `applyPercentile()`: Histogram-based percentile stretch. Builds a histogram, finds the `lowPct` and `highPct` percentile values, clamps pixels into that range, then linearly maps to `[0, maxval]`. Robust to outliers.
- **OpenMP:** `#pragma omp parallel for` on the final mapping loop in both modes.

#### 5. [threshold_otsu.cpp](project/src/ops/threshold_otsu.cpp)
- **Algorithm:** Classic **Otsu's method** — global thresholding. Builds a histogram, sweeps through all possible thresholds, maximizes between-class variance $\sigma_B^2 = w_B \cdot w_F \cdot (\mu_B - \mu_F)^2$. Single optimal threshold for the whole image.
- **OpenMP:** `#pragma omp parallel for` on the final binarization pass.
- **Convention:** pixel ≤ threshold → black (0), else white (maxval).

#### 6. [threshold_sauvola.cpp](project/src/ops/threshold_sauvola.cpp)
- **Algorithm:** **Sauvola adaptive thresholding**. Per-pixel threshold: $T = \mu_W \cdot \left(1 + k \cdot \left(\frac{\sigma_W}{R} - 1\right)\right)$ where $R = 125$, $\mu_W$ and $\sigma_W$ are local mean/stddev in a $(2r+1) \times (2r+1)$ window.
- **Key parameters:** `r` (window radius), `k` (sensitivity, typically 0.2–0.5).
- **Special features:** Uses **integral images** (sum and sum-of-squares) for O(1) per-pixel window statistics. Uses `long double` precision.
- **OpenMP:** `#pragma omp parallel for` over rows for the thresholding pass.

#### 7. [threshold_nick.cpp](project/src/ops/threshold_nick.cpp)
- **Algorithm:** **NICK thresholding** — a variant of Niblack's method. Per-pixel threshold: $T = \mu_W + k \cdot \sigma_W$ (with $k$ typically negative to shift threshold down).
- **Key parameters:** `r` (window radius), `k` (typically negative, e.g. -0.2).
- **Special features:** Integral images for O(1) window statistics, `long double` precision.
- **OpenMP:** `#pragma omp parallel for` over rows.

#### 8. [threshold_su.cpp](project/src/ops/threshold_su.cpp)
- **Algorithm:** **Su, Lu, Tan (2010)** document binarization. Multi-step:
  1. Compute **contrast image** $D(x,y) = \frac{f_{max} - f_{min}}{f_{max} + f_{min} + \varepsilon}$ in a 3×3 neighborhood.
  2. **Otsu** on the contrast image to identify high-contrast (edge) pixels.
  3. **Stroke width estimation** — scans rows for peak-to-peak distances in the contrast image, builds a histogram, finds the mode (with a minimum distance filter `SW_MIN_D = 7` to ignore noise). Auto-derives `r` and `Nmin` from stroke width.
  4. Build **4 integral images**: edge-pixel count, edge-pixel sum, edge-pixel sum-of-squares, and all-pixel sum.
  5. **Classification:** If the window has ≥ `Nmin` edge pixels, threshold is $T = \mu_e + \sigma_e / 2$ (Eq. 2 from the paper). Otherwise, a **fallback** uses local mean of all pixels minus an offset (prevents white holes in uniform regions).
- **Key parameters:** `r`, `Nmin` (auto if ≤ 0), `eps` (denominator guard).
- **OpenMP:** `#pragma omp parallel for` on contrast computation, edge mask creation, and classification.

#### 9. [threshold_proposed.cpp](project/src/ops/threshold_proposed.cpp) / [threshold_proposed.h](project/src/ops/threshold_proposed.h)
- **Algorithm:** A **unit-consistent hybrid** adaptive thresholding method. Two-pass:
  1. **Pass 1:** Compute per-pixel local stddev $\sigma_W$ using integral images; track global $\sigma_{min}$, $\sigma_{max}$.
  2. **Pass 2:** For each pixel, compute $\sigma_{adaptive} = \frac{\sigma_W - \sigma_{min}}{\sigma_{max} - \sigma_{min}} \in [0,1]$, convert back to RAW scale ($\sigma_{adaptive} \cdot \sigma_{max}$), then apply: $T = \mu_W - \frac{\mu_W^2 - \sigma_W}{(\mu_g + \sigma_W)(\sigma_{adaptive,raw} + \sigma_W)}$ where $\mu_g$ is the global mean.
- **Key parameters:** `r` (window radius, default 10 → 21×21 window).
- **Special features:** Integral images, `long double` precision, extensive **diagnostic output** to stderr (sigmaMin/Max, threshold range, black/white ratio, denominator clamp counts). OpenMP reductions for min/max.
- **OpenMP:** `#pragma omp parallel for` with `reduction(min:...)` and `reduction(max:...)` on both passes.

#### 10. [morphology.cpp](project/src/ops/morphology.cpp)
- **Algorithm:** Binary morphological operations with a 3×3 structuring element (full square kernel):
  - `erode3x3`: output black only if **all** 3×3 neighbors are black.
  - `dilate3x3`: output black if **any** 3×3 neighbor is black.
  - `open3x3`: erode then dilate (removes small foreground noise).
  - `close3x3`: dilate then erode (fills small holes in foreground).
- **OpenMP:** `#pragma omp parallel for` over rows for erode and dilate.

#### 11. [border_cleanup.cpp](project/src/ops/border_cleanup.cpp)
- **Two modes:**
  - `whitenEdges`: Unconditionally sets all pixels within `borderWidth` of any edge to white (maxval).
  - `whitenDarkEdges`: Only whitens border pixels that are **darker** than `thresholdFrac × maxval` — preserves content that extends to the edge.
- **Safety:** Clamps `borderWidth` so it can't exceed half the image dimension.
- **OpenMP:** `#pragma omp parallel for` over rows in both modes.

---

### Tests ([project/tests/](project/tests/))

All tests use **Catch2** framework.

| Test File | What It Covers |
|---|---|
| [test_ppm_io.cpp](project/tests/io/test_ppm_io.cpp) | `TokenReader` whitespace/comment skipping; `ppm_reader` parsing valid P3 with comments, rejection of non-P3 magic, rejection of out-of-range pixels; `ppm_writer` round-trip correctness with value clamping and data-size validation. (6 test cases) |
| [test_otsu.cpp](project/tests/ops/test_otsu.cpp) | Otsu on a simple bimodal image (0,0,255,255 → correctly separated); constant image remains all-white. (2 test cases) |
| [test_sauvola.cpp](project/tests/ops/test_sauvola.cpp) | Sauvola on a 7×7 image with a vertical black stroke on near-white background — verifies stroke pixels stay black and background corners become white. (1 test case) |
| [test_median.cpp](project/tests/ops/test_median.cpp) | Median filter removes a single salt-noise pixel (center 255 surrounded by 100s → median restores to 100). (1 test case) |

Tests are built as a single `project_tests` executable linked against `project_lib` and `Catch2::Catch2WithMain`.

---

### Summary of Special Features

- **Integral images** are used in Sauvola, NICK, Su, and Proposed thresholding for O(1) per-pixel window statistics regardless of window size.
- **OpenMP** parallelization is used in nearly every operation (median, background estimate, contrast stretch, all thresholds, morphology, border cleanup).
- **Fallback strategy** in Su's method handles regions with too few edge pixels by using local mean of all pixels.
- **Stroke width auto-estimation** in Su's method derives window parameters from the document content itself.
- **Diagnostic stderr output** in the Proposed method aids tuning and debugging.
- All algorithms use **`long double`** for integral image accumulators to avoid precision loss on large images.