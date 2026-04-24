# Multi-Otsu Thresholding

> **An algorithm that segments an image based on its gray-level intensities**

input: 8-, 16-, 24-bit image
output: 8-bit image segmented into three classes with two thresholds (t1 and t2)

---

## Quickstart

Copy `include/multi_otsu.hpp` into your project. That's it.

```cpp
#include "multi_otsu.hpp"

// pixels: flat array of uint8_t, width * height bytes
multi_otsu::Thresholds t;
std::vector<multi_otsu::Label> labels =
    multi_otsu::segment(pixels, width * height, t);

// t.t1, t.t2  : the two thresholds
// labels[i]   : Label::Low / Label::Mid / Label::High
```

---

## API

```cpp
namespace multi_otsu {

struct Thresholds { int t1; int t2; };
enum class Label : uint8_t { Low = 0, Mid = 1, High = 2 };

// Build normalised histogram from raw pixel data
std::vector<double> build_histogram(const uint8_t* pixels, size_t count);

// Compute optimal thresholds from a normalised histogram
Thresholds compute_thresholds(const std::vector<double>& hist);

// Classify a single pixel
Label classify(uint8_t intensity, Thresholds t) noexcept;

// Apply thresholds to a pixel array → label vector
std::vector<Label> apply_thresholds(const uint8_t* pixels, size_t count, Thresholds t);

// All-in-one convenience wrapper
std::vector<Label> segment(const uint8_t* pixels, size_t count, Thresholds& t);

} // namespace multi_otsu
```

Intensity ranges per label:

| Label | Range |
|-------|-------|
| `Low`  | `[0,   t1]`    |
| `Mid`  | `(t1,  t2]`    |
| `High` | `(t2, 255]`    |

---

## CMake integration

### Standalone build

```bash
cmake -B build
cmake --build build        # builds demo + tests
ctest --test-dir build     # run tests
```

### As a subdirectory in your project

```cmake
add_subdirectory(multi-otsu)
target_link_libraries(your_target PRIVATE multi_otsu::multi_otsu)
```

---

## Algorithm

Extends the classic Otsu criterion to three classes by maximising:

$$\sigma_B^2 = \frac{\mu_1^2}{\omega_1} + \frac{\mu_2^2}{\omega_2} + \frac{\mu_3^2}{\omega_3}$$

where $\omega_c$ and $\mu_c$ are the class weight and unnormalised mean of class $c$.

**Complexity: O(L²)** with L = 256, using prefix sums over the histogram.
For any interval `[u, v]`, weight and mean are computed in O(1):

```
P[i] = Σ_{k=0}^{i}   hist[k]    →  ω(u,v) = P[v] − P[u−1]
S[i] = Σ_{k=0}^{i} k·hist[k]    →  μ(u,v) = S[v] − S[u−1]
```

This avoids the naive O(L² × N) approach that re-sums the histogram on every iteration.

**Reference:**
N. Otsu, "A Threshold Selection Method from Gray-Level Histograms,"
*IEEE Trans. Syst. Man Cybern.*, vol. 9, no. 1, pp. 62–66, 1979.
[doi:10.1109/TSMC.1979.4310076](https://doi.org/10.1109/TSMC.1979.4310076)

---

## CLI demo

The optional demo reads a raw 8-bit grayscale binary file:

```bash
# Convert with ImageMagick
convert input.png -depth 8 gray:image.raw

# Run
./build/multi_otsu_demo image.raw 512 512
```

```
Thresholds : t1 = 87, t2 = 163
  Class 0  Low  [0,   t1] : 142350 px  (55.6 %)
  Class 1  Mid  (t1,  t2] :  89120 px  (34.8 %)
  Class 2  High (t2, 255] :  24530 px  ( 9.6 %)
```

---

## Project structure

```
multi-otsu/
├── include/
│   └── multi_otsu.hpp    ← the only file you need
├── src/
│   └── main.cpp          ← CLI demo
├── tests/
│   └── test_multi_otsu.cpp
├── CMakeLists.txt
├── LICENSE
└── README.md
```

---

## License

MIT — see [LICENSE](LICENSE).
