#pragma once

/**
 * multi_otsu.hpp  —  Header-only, dependency-free 3-class Otsu thresholding
 *
 * Finds two thresholds {t1, t2} (0 < t1 < t2 < 255) that maximise the
 * between-class variance of an 8-bit grayscale histogram, partitioning
 * pixel intensities into three classes:
 *
 *   Class 0 : intensity in [0,    t1]
 *   Class 1 : intensity in [t1+1, t2]
 *   Class 2 : intensity in [t2+1, 255]
 *
 * Algorithm complexity: O(L²) in intensity levels (L = 256).
 * Cumulative prefix sums reduce each inner-loop evaluation to O(1),
 * versus the naive O(L² · N) approach that re-sums the histogram each time.
 *
 * Dependencies: C++17 standard library only (<vector>, <array>, <stdexcept>).
 *
 * Reference:
 *   N. Otsu, "A Threshold Selection Method from Gray-Level Histograms,"
 *   IEEE Trans. Syst. Man Cybern., vol. 9, no. 1, pp. 62–66, 1979.
 *   https://doi.org/10.1109/TSMC.1979.4310076
 */

#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace multi_otsu {

// ─────────────────────────────────────────────────────────────────────────────
// Types
// ─────────────────────────────────────────────────────────────────────────────

/// Pair of thresholds returned by compute_thresholds().
struct Thresholds {
    int t1 = 0; ///< Lower threshold:  class 0 → [0, t1]
    int t2 = 0; ///< Upper threshold:  class 2 → (t2, 255]
};

/// Label assigned to each pixel.
enum class Label : std::uint8_t {
    Low  = 0, ///< Intensity ≤ t1
    Mid  = 1, ///< t1 < intensity ≤ t2
    High = 2, ///< Intensity > t2
};

// ─────────────────────────────────────────────────────────────────────────────
// Core function: compute thresholds from a pre-built histogram
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Compute optimal 3-class Otsu thresholds from a normalised histogram.
 *
 * @param hist  Normalised histogram of length 256 (values sum to 1.0).
 *              Use build_histogram() to obtain this from raw pixel data.
 * @return      Thresholds {t1, t2} maximising between-class variance.
 * @throws      std::invalid_argument if hist.size() != 256.
 */
inline Thresholds compute_thresholds(const std::vector<double>& hist)
{
    constexpr int L = 256;

    if (static_cast<int>(hist.size()) != L)
        throw std::invalid_argument("multi_otsu::compute_thresholds: histogram must have exactly 256 bins.");

    // ── Prefix sums ──────────────────────────────────────────────────────────
    // P[i] = Σ_{k=0}^{i} hist[k]      cumulative probability (zeroth moment)
    // S[i] = Σ_{k=0}^{i} k·hist[k]   cumulative mean         (first  moment)
    std::array<double, L> P{}, S{};
    P[0] = hist[0];
    S[0] = 0.0; // 0 * hist[0]
    for (int i = 1; i < L; ++i) {
        P[i] = P[i - 1] + hist[i];
        S[i] = S[i - 1] + i * hist[i];
    }

    // Class weight and unnormalised mean for intensity interval [u, v]
    auto omega = [&](int u, int v) -> double {
        return (u == 0) ? P[v] : P[v] - P[u - 1];
    };
    auto mu = [&](int u, int v) -> double {
        return (u == 0) ? S[v] : S[v] - S[u - 1];
    };

    // ── Maximise between-class variance ─────────────────────────────────────
    //
    //   σ_B² = μ₁²/ω₁ + μ₂²/ω₂ + μ₃²/ω₃
    //
    Thresholds best{};
    double max_sigma = -1.0;

    for (int t1 = 1; t1 < L - 2; ++t1) {
        for (int t2 = t1 + 1; t2 < L - 1; ++t2) {
            const double w1 = omega(0,      t1);
            const double w2 = omega(t1 + 1, t2);
            const double w3 = omega(t2 + 1, L - 1);

            // Skip degenerate splits (empty class → division by zero)
            if (w1 <= 0.0 || w2 <= 0.0 || w3 <= 0.0)
                continue;

            const double m1 = mu(0,      t1);
            const double m2 = mu(t1 + 1, t2);
            const double m3 = mu(t2 + 1, L - 1);

            const double sigma = (m1 * m1) / w1
                               + (m2 * m2) / w2
                               + (m3 * m3) / w3;

            if (sigma > max_sigma) {
                max_sigma = sigma;
                best      = {t1, t2};
            }
        }
    }

    return best;
}

// ─────────────────────────────────────────────────────────────────────────────
// Histogram builder: works with any flat array of uint8_t pixels
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Build a normalised histogram from a flat array of 8-bit pixel values.
 *
 * @param pixels  Pointer to pixel data (row-major, single channel, 8-bit).
 * @param count   Total number of pixels (width × height).
 * @return        Normalised histogram of length 256 (sums to 1.0).
 * @throws        std::invalid_argument if pixels is nullptr or count is 0.
 */
inline std::vector<double> build_histogram(const std::uint8_t* pixels, std::size_t count)
{
    if (!pixels || count == 0)
        throw std::invalid_argument("multi_otsu::build_histogram: null pointer or zero pixel count.");

    constexpr int L = 256;
    std::vector<double> hist(L, 0.0);

    for (std::size_t i = 0; i < count; ++i)
        hist[pixels[i]] += 1.0;

    const double inv = 1.0 / static_cast<double>(count);
    for (double& h : hist)
        h *= inv;

    return hist;
}

// ─────────────────────────────────────────────────────────────────────────────
// Label assignment
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Assign a Label to a single pixel intensity.
 *
 * @param intensity  8-bit pixel value.
 * @param t          Threshold pair from compute_thresholds().
 * @return           Label::Low, Label::Mid, or Label::High.
 */
inline Label classify(std::uint8_t intensity, Thresholds t) noexcept
{
    if (intensity <= static_cast<std::uint8_t>(t.t1)) return Label::Low;
    if (intensity <= static_cast<std::uint8_t>(t.t2)) return Label::Mid;
    return Label::High;
}

/**
 * @brief Apply thresholds to a flat pixel array, producing a label vector.
 *
 * @param pixels  Pointer to 8-bit pixel data.
 * @param count   Total number of pixels.
 * @param t       Thresholds from compute_thresholds().
 * @return        Vector of Label values, same length as pixel count.
 * @throws        std::invalid_argument if pixels is nullptr or count is 0.
 */
inline std::vector<Label> apply_thresholds(const std::uint8_t* pixels,
                                            std::size_t          count,
                                            Thresholds           t)
{
    if (!pixels || count == 0)
        throw std::invalid_argument("multi_otsu::apply_thresholds: null pointer or zero pixel count.");

    std::vector<Label> labels(count);
    for (std::size_t i = 0; i < count; ++i)
        labels[i] = classify(pixels[i], t);

    return labels;
}

// ─────────────────────────────────────────────────────────────────────────────
// Convenience: all-in-one
// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief Compute thresholds and label all pixels in one call.
 *
 * Equivalent to:
 * @code
 *   auto hist   = build_histogram(pixels, count);
 *   auto t      = compute_thresholds(hist);
 *   auto labels = apply_thresholds(pixels, count, t);
 * @endcode
 *
 * @param pixels  Pointer to 8-bit pixel data.
 * @param count   Total number of pixels.
 * @param t       Output: the computed thresholds.
 * @return        Vector of Label values.
 */
inline std::vector<Label> segment(const std::uint8_t* pixels,
                                   std::size_t          count,
                                   Thresholds&          t)
{
    const auto hist = build_histogram(pixels, count);
    t               = compute_thresholds(hist);
    return apply_thresholds(pixels, count, t);
}

} // namespace multi_otsu
