# Enhancer for Scanned Images (PPM P3) — Project Structure, Considerations, Resources

## Project Goal
Build a free executable that improves the quality of scanned documents **before printing or sending**, using a **pipeline** of image-processing steps such as:
- Noise reduction
- Background removal / illumination correction
- Optional binarization (for print/OCR)
- Border cleanup

**Input:** Portable Pixmap **PPM color image(s)** in **ASCII format (P3)**  
**Output:** Cleaned **PPM ASCII (P3)**

---

## 1) What you should consider first

### PPM (P3) format realities
Your parser must handle:
- **Comments** starting with `#` (can appear between tokens)
- Arbitrary whitespace (spaces/newlines/tabs)
- `maxval` not always 255 (sometimes larger; treat it as a real parameter)

> Tip: Parse PPM using a *token reader* (read next int/string while skipping whitespace + comments), not line-based parsing.

### What “cleaned” means for scanned documents
Scans often contain:
- Uneven lighting / gray background
- Salt-and-pepper noise / dust specks
- Faint text, low contrast
- Dark scanner borders / shadows

So your “enhancer” should be a **configurable pipeline** of small operations, not one monolithic filter.

---

## 2) Recommended project structure

This layout is clean, testable, and scalable (works for C/C++/Java/Python):

```text
scanned-image-enhancer/
  README.md
  LICENSE
  docs/
    format_notes.md
    algorithms.md
    examples.md

  data/
    input_samples/
    expected_outputs/
    fixtures/              # tiny synthetic images for unit tests

  src/
    main/
      main.*               # CLI entry point

    io/
      ppm_reader.*
      ppm_writer.*
      token_reader.*       # reads ints while skipping whitespace/comments

    image/
      image_rgb.*
      image_gray.*
      colorspace.*         # rgb<->gray utilities, optional gamma notes

    ops/                   # each operation is small & composable
      denoise_median.*
      denoise_bilateral.*  # optional: nicer but slower
      background_estimate.*# estimate illumination/background
      background_remove.*
      contrast_stretch.*
      threshold_otsu.*     # optional binarization
      threshold_sauvola.*  # very good for documents
      morphology.*         # open/close, erode/dilate
      border_cleanup.*     # edge/border shadow handling

    pipeline/
      pipeline.*
      presets.*            # "print", "ocr", "light", "aggressive"

    util/
      clamp.*
      timing.*
      logging.*
      args_parser.*

  tests/
    test_ppm_io.*
    test_ops_unit.*
    test_pipeline_golden.* # compare output to expected output

  scripts/
    run_batch.sh
    compare_with_netpbm.sh

  CMakeLists.txt (or build.gradle / pom.xml / setup.py)

```

# What to Start With (Implementation Order)

The best way to implement your **Scanned Image Enhancer (PPM P3)** is to start with the parts that everything else depends on, then build up the enhancement pipeline step by step.

---

## 1) Build a rock-solid PPM (P3) reader/writer (foundation)

**Why first:** If your IO is buggy, you can’t trust any filter results.

### Do this
- Implement a **token reader**:
    - skip whitespace
    - if you see `#`, ignore until end of line
    - return the next token (string/int)
- Parse header:
    - magic number: must be `P3`
    - read `width`, `height`, `maxval`
    - validate: width/height > 0, maxval > 0
- Read pixels:
    - total values must be `width * height * 3`
    - clamp or error if values are outside `[0..maxval]`
- Write P3 back out:
    - write header + pixel values
    - keep `maxval` the same

✅ **Milestone:** `read -> write` produces an identical image (round-trip test).

---

## 2) Create a tiny “debug pipeline” (no enhancement yet)

**Why:** You want a working CLI + pipeline structure early.

### Do this
- CLI accepts:
    - input path, output path
    - `--verbose`
- Pipeline initially does:
    - load image
    - save image

✅ **Milestone:** `enhance in.ppm out.ppm` works and prints useful messages.

---

## 3) Add grayscale conversion + histogram/stats (understand your scans)

**Why:** Most document-cleaning logic (background removal, thresholding) is based on grayscale.

### Do this
- Convert RGB → Gray (luminance):
    - `gray = 0.299R + 0.587G + 0.114B` (scaled to `maxval`)
- Compute simple stats:
    - min, max, mean
    - optional histogram (e.g., 256 bins if maxval=255)

✅ **Milestone:** You can print stats and verify typical scan values  
(e.g., “background is around 210–240”).

---

## 4) Implement your first real filter: median denoise (3×3)

**Why:** It’s easy and very effective for dust/specks and salt-and-pepper noise.

### Do this
- Implement median filter for:
    - grayscale first (simpler)
    - later optionally apply to each RGB channel

✅ **Milestone:** visible improvement on noisy scans with minimal tuning.

---

## 5) Implement background estimation + background removal (core feature)

Start with a simple method that works well:

### Option A (simple and good): blur-based background estimate
- **Background estimate** = large blur (box blur or Gaussian approximation) on grayscale
- **Background removal**:
    - `clean = gray - bg + target`
    - clamp to `[0..maxval]`
    - choose `target ≈ 0.9 * maxval` so paper becomes near-white

✅ **Milestone:** uneven paper becomes flatter/whiter; text becomes easier to read.

---

## 6) Add a “document mode” (optional): thresholding for print/OCR

If you want black/white output:
- Start with **Otsu** (global threshold) because it’s simpler
- Then add **Sauvola** (better for uneven illumination)

✅ **Milestone:** `--preset ocr` produces clean black text on white background.

---

## 7) Finish with quality and usability features
- Border cleanup (crop or whiten dark edges)
- Presets: `print / ocr / light / aggressive`
- Batch mode (process folders)
- Golden tests (expected output comparisons)

---

# A practical first-week plan
1. **Day 1–2:** P3 token reader + PPM read/write + round-trip test
2. **Day 3:** CLI skeleton + pipeline that just copies
3. **Day 4:** grayscale conversion + print stats
4. **Day 5–6:** median filter + visual testing
5. **Day 7:** background estimate + removal (simple blur-based)

---

If you tell me the language you’re using (C / C++ / Java / Python), I can provide a starter skeleton (folder structure + core structs/classes + token reader pseudocode) tailored to it.


