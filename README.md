# Algorithm Engineering Project

This repository contains various algorithm engineering assignments and a main project focused on image processing and computational techniques.

## Prerequisites

- **CMake** (version 3.26 or higher)
- **C++ Compiler** with C++20 support (GCC 11+ recommended)
- **OpenMP** support
- **Git** (for fetching dependencies)

### Ubuntu/Debian Installation:
```bash
sudo apt update
sudo apt install build-essential cmake libgomp1 git
```

## Project Structure

```
AlgorthimEng/
├── CMakeLists.txt          # Main CMake configuration
├── main.cpp               # Main entry point (if needed)
├── assigment1/            # Monte Carlo π estimation
├── assigment2/            # Monte Carlo with OpenMP
├── assigment3/            # Monte Carlo with Linear Congruential Generator
├── assigment4/            # Quicksort with min/max optimization + benchmarking
├── assigment5/            # FizzBuzz and image quantization
├── assigment6-11/         # Additional assignments (documentation only)
└── project/               # Main image processing project
```

## Building the Project

1. **Clone and navigate to the project directory:**
   ```bash
   git clone <repository-url>
   cd AlgorthimEng
   ```

2. **Create and enter build directory:**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake:**
   ```bash
   cmake ..
   ```

4. **Build the project:**
   ```bash
   make -j$(nproc)
   ```
   This will build all executables using all available CPU cores.

## Running Assignments

### Assignment 1 - Monte Carlo π Estimation
```bash
./main_assigment1
```
**Output:** Estimates π using Monte Carlo method with timing information.

### Assignment 2 - Monte Carlo with OpenMP
```bash
./main_assigment2
```
**Output:** Parallel Monte Carlo π estimation using OpenMP.

### Assignment 3 - Monte Carlo with LCG
```bash
./main_assigment3
```
**Output:** Monte Carlo estimation using Linear Congruential Generator.

### Assignment 4 - Quicksort Benchmarking
```bash
./main_assigment4
```
**Output:** Performance benchmarks of optimized quicksort algorithm.

### Assignment 5 - FizzBuzz and Image Quantization

#### FizzBuzz Tests:
```bash
./assigment5/catch/catch_tests_fizzbuzz
```

#### Image Quantization:
```bash
./assigment5/quantpng/quantpng <input.png> <output.png> <colors>
```

## Main Project - Image Processing

### Build and Run:
```bash
./project/project_executable <input.ppm> <output.ppm> [options]
```

### Complete Parameter Reference:

#### Basic Options:
```bash
--verbose                        # Enable detailed output
--threads <N>                    # Set OpenMP threads (0 = auto)
```

#### Preprocessing Operations:
```bash
--median <0|1>                   # Apply median filter (1=3x3, 0=off)
--bg-radius <R>                  # Background blur radius (0=off)
--target <0..1>                  # Paper target fraction (default: 0.90)
--contrast-pct <low> <high>      # Percentile stretch (e.g., 1 99)
```

#### Binarization Methods (choose ONE):
```bash
--otsu                           # Otsu's automatic thresholding
--sauvola <radius> <k>           # Sauvola adaptive thresholding
                                # Example: --sauvola 25 0.34
--nick <radius> <k>              # Nick adaptive thresholding  
                                # Example: --nick 25 -0.10
--su <radius> <Nmin> [eps]       # Su adaptive thresholding
                                # Example: --su 25 30 1e-6
--proposed <radius>              # Custom proposed method
                                # Example: --proposed 10
```

#### Morphological Operations (binary cleanup):
```bash
--open                           # Opening (erode then dilate) 3x3
--close                          # Closing (dilate then erode) 3x3
```

#### Border Cleanup:
```bash
--border <width>                 # Whiten fixed border width
--border-dark <width> <thrFrac>  # Whiten only dark border pixels
                                # Example: --border-dark 15 0.6
```
window size=(2r+1)×(2r+1) !! and r ist the radius
2r+1=15
r = 7


### Complete Example Usage:
```bash
# Basic Otsu thresholding
./project/project_executable input.ppm output.ppm --otsu



# Advanced pipeline with all preprocessing steps
# windows size = 2r+1 = 15 and R is fixed at 125 dynamic range of standard deviation
./project/project_executable input.ppm output.ppm \
    --median 1 \
    --bg-radius 45 \
    --contrast-pct 1 99 \
    --sauvola 7 0.20 \
    --open \
    --border-dark 15 0.6 \
    --threads 8 \
    --verbose

# NICK thresholding with morphology
./project/project_executable input.ppm output.ppm \
    --median 1 \
    --nick 9 -0.1 \
    --close \
    --border-dark 15 0.6 \
    --threads 8

# Su method with custom parameters
./project/project_executable input.ppm output.ppm \
    --su 25 30 1e-6 \
    --border 10 \
    --verbose

# Proposed method with full preprocessing
./project/project_executable input.ppm output.ppm \
    --median 1 \
    --bg-radius 45 \
    --contrast-pct 1 99 \
    --proposed 10 \
    --border-dark 15 0.6 \
    --threads 8
```

### Parameter Details:

#### Sauvola Parameters:
- **radius**: Neighborhood size (typical: 15-30)
- **k**: Sensitivity parameter (typical: 0.2-0.5)

#### Nick Parameters:
- **radius**: Neighborhood size (typical: 10-30)  
- **k**: Sensitivity parameter (typical: -0.1 to -0.2)

#### Su Parameters:
- **radius**: Neighborhood size (typical: 20-30)
- **Nmin**: Minimum edge pixels (typical: 20-40)
- **eps**: Contrast epsilon (typical: 1e-6)

#### Background Removal:
- **bg-radius**: Blur radius for background estimation (typical: 30-50)
- **target**: Target paper brightness as fraction of maxval (0.85-0.95)

#### Border Cleanup:
- **border-dark width**: Border width to process (typical: 10-20)
- **border-dark thrFrac**: Threshold fraction for "dark" pixels (0.5-0.7)

### Running Project Tests:
```bash
./project/project_tests
```

## Performance Notes

- **OpenMP Support:** The project is compiled with OpenMP support for parallel processing
- **Optimization:** Built with compiler optimizations enabled
- **Large Datasets:** Assignment 3 includes very large iteration counts (may take significant time)

## Troubleshooting

### Build Issues:
1. **CMake version too old:** Ensure CMake 3.26+ is installed
2. **Missing OpenMP:** Install `libgomp1-dev` on Ubuntu/Debian
3. **Compiler errors:** Ensure GCC 11+ or equivalent C++20 compiler

### Runtime Issues:
1. **Segmentation faults:** Check input file formats (PPM files for image processing)
2. **Performance issues:** Large iteration counts in assignments may require patience
3. **Missing files:** Ensure you're running executables from the `build/` directory

## Assignment Documentation

Each assignment directory contains a `bericht.md` file with detailed documentation about:
- Algorithm implementation
- Performance analysis
- Results and conclusions
- Theoretical background

## Contributing

When adding new assignments or modifying existing code:
1. Follow the existing CMake structure
2. Add appropriate tests where applicable
3. Update this README if new executables are added
4. Maintain C++20 standards compliance

## License

[Add appropriate license information here]