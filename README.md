# Multi-Otsu Thresholding

> A lightweight, dependency-free implementation of 3-class Otsu thresholding for tif image segmentation.

This project provides a CLI tool and a header-only algorithm to segment images into three intensity classes using two optimal thresholds (t1, t2).

---

## Overview

Input:
- 8-bit grayscale TIFF
- 16-bit grayscale TIFF
- 24-bit RGB TIFF

Output:
- 8-bit grayscale TIFF segmented into 3 classes:

| Class | Intensity range |
|------|----------------|
| Low  | [0, t1]        |
| Mid  | (t1, t2]       |
| High | (t2, 255]      |

---

## Quickstart
```
cmake -B build
cmake --build build
```
Run:
```
./build/multi_otsu_cli input.tif output.tif
```
---

## GitHub Actions

The CI pipeline:
- builds the project
- processes all TIFFs in examples/input/
- uploads examples/output/ as artifacts

---

## API

```
namespace multi_otsu {

struct Thresholds {
    int t1;
    int t2;
};

enum class Label : uint8_t {
    Low  = 0,
    Mid  = 1,
    High = 2
};

// Build histogram from raw pixel data
std::vector<double> build_histogram(const uint8_t* pixels, size_t count);

// Compute optimal thresholds
Thresholds compute_thresholds(const std::vector<double>& hist);

// Classify single pixel
Label classify(uint8_t intensity, Thresholds t) noexcept;

// Apply segmentation
std::vector<Label> apply_thresholds(const uint8_t* pixels, size_t count, Thresholds t);

// Full pipeline
std::vector<Label> segment(const uint8_t* pixels, size_t count, Thresholds& t);

} // namespace multi_otsu
```
---

## Algorithm

$\sigma_B^2 = \frac{\mu_1^2}{\omega_1} + \frac{\mu_2^2}{\omega_2} + \frac{\mu_3^2}{\omega_3}$

$P[i] = \sum_{k=0}^{i} \text{hist}[k], \quad S[i] = \sum_{k=0}^{i} k \cdot \text{hist}[k]$

$\omega(u,v) = P[v] - P[u-1]$
$\mu(u,v) = S[v] - S[u-1]$

---

## Project structure

```
multi-otsu/
├── include/
│   └── multi_otsu.hpp
│   └── tiff_io.hpp
│
├── src/
│   ├── main.cpp
│   └── tiff_io.cpp
│
├── examples/
│   ├── input/
│   └── output/
│
├── CMakeLists.txt
├── LICENSE
└── README.md
```
---

## License

MIT
