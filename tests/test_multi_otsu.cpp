/**
 * tests/test_multi_otsu.cpp
 *
 * Lightweight self-contained test suite — no external test framework.
 * Build and run:
 *   cmake -B build -DBUILD_TESTS=ON && cmake --build build
 *   ./build/test_multi_otsu
 *
 * Exit code 0 = all tests passed.
 */

#include "multi_otsu.hpp"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Minimal test harness
// ─────────────────────────────────────────────────────────────────────────────
static int g_pass = 0, g_fail = 0;

#define CHECK(cond) \
    do { \
        if (cond) { ++g_pass; } \
        else { \
            ++g_fail; \
            std::cerr << "FAIL  " << __FILE__ << ":" << __LINE__ \
                      << "  " << #cond << "\n"; \
        } \
    } while (false)

#define CHECK_THROWS(expr) \
    do { \
        bool threw = false; \
        try { expr; } catch (...) { threw = true; } \
        if (threw) { ++g_pass; } \
        else { \
            ++g_fail; \
            std::cerr << "FAIL  " << __FILE__ << ":" << __LINE__ \
                      << "  expected exception from: " << #expr << "\n"; \
        } \
    } while (false)

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

/// Build a synthetic image with three perfectly separated intensity bands.
/// pixels[0 .. band_size-1]            = low_val
/// pixels[band_size .. 2*band_size-1]  = mid_val
/// pixels[2*band_size .. 3*band_size-1]= high_val
static std::vector<std::uint8_t> make_trimodal(std::size_t band_size,
                                                std::uint8_t low_val,
                                                std::uint8_t mid_val,
                                                std::uint8_t high_val)
{
    std::vector<std::uint8_t> v(3 * band_size);
    for (std::size_t i = 0; i < band_size; ++i)  v[i]              = low_val;
    for (std::size_t i = 0; i < band_size; ++i)  v[band_size  + i] = mid_val;
    for (std::size_t i = 0; i < band_size; ++i)  v[2*band_size+i]  = high_val;
    return v;
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests
// ─────────────────────────────────────────────────────────────────────────────

static void test_trimodal_separation()
{
    // Three equal-sized classes at intensities 50, 128, 200.
    // Optimal thresholds should fall between them.
    constexpr std::size_t N = 1000;
    auto pixels = make_trimodal(N, 50, 128, 200);

    multi_otsu::Thresholds t;
    multi_otsu::segment(pixels.data(), pixels.size(), t);

    // t1 is the last index of the low class: all low pixels (value=50) must be
    // in class 0, so t1 >= 50; and t1 must be below the mid class, so t1 < 128
    CHECK(t.t1 >= 50 && t.t1 < 128);
    // t2 is the last index of the mid class: all mid pixels (value=128) must be
    // in class 1, so t2 >= 128; and t2 must be below the high class, so t2 < 200
    CHECK(t.t2 >= 128 && t.t2 < 200);
    CHECK(t.t1 < t.t2);
}

static void test_all_pixels_same_class()
{
    // Uniform image → all pixels should land in the same class
    std::vector<std::uint8_t> pixels(500, 100);

    multi_otsu::Thresholds t;
    auto labels = multi_otsu::segment(pixels.data(), pixels.size(), t);

    // All labels must be identical
    const auto first = labels[0];
    bool all_same = true;
    for (const auto& l : labels)
        if (l != first) { all_same = false; break; }
    CHECK(all_same);
}

static void test_classify_boundaries()
{
    multi_otsu::Thresholds t{80, 160};

    CHECK(multi_otsu::classify(  0, t) == multi_otsu::Label::Low);
    CHECK(multi_otsu::classify( 80, t) == multi_otsu::Label::Low);   // inclusive
    CHECK(multi_otsu::classify( 81, t) == multi_otsu::Label::Mid);
    CHECK(multi_otsu::classify(160, t) == multi_otsu::Label::Mid);   // inclusive
    CHECK(multi_otsu::classify(161, t) == multi_otsu::Label::High);
    CHECK(multi_otsu::classify(255, t) == multi_otsu::Label::High);
}

static void test_label_count()
{
    // 300 pixels: first 100 = 20, next 100 = 100, last 100 = 200
    auto pixels = make_trimodal(100, 20, 100, 200);

    multi_otsu::Thresholds t;
    auto labels = multi_otsu::segment(pixels.data(), pixels.size(), t);

    CHECK(labels.size() == 300);

    std::size_t cnt[3] = {0, 0, 0};
    for (const auto& l : labels)
        ++cnt[static_cast<int>(l)];

    // Each class must contain exactly 100 pixels
    CHECK(cnt[0] == 100);
    CHECK(cnt[1] == 100);
    CHECK(cnt[2] == 100);
}

static void test_histogram_normalisation()
{
    std::vector<std::uint8_t> pixels = {0, 0, 255, 255};
    auto hist = multi_otsu::build_histogram(pixels.data(), pixels.size());

    CHECK(hist.size() == 256);
    // Sum must be 1.0 within floating-point tolerance
    double sum = 0.0;
    for (double h : hist) sum += h;
    CHECK(std::fabs(sum - 1.0) < 1e-12);

    CHECK(std::fabs(hist[  0] - 0.5) < 1e-12);
    CHECK(std::fabs(hist[255] - 0.5) < 1e-12);
}

static void test_invalid_inputs()
{
    CHECK_THROWS(multi_otsu::build_histogram(nullptr, 100));
    CHECK_THROWS(multi_otsu::build_histogram(reinterpret_cast<const std::uint8_t*>("x"), 0));

    std::vector<double> bad_hist(100, 0.01); // wrong size
    CHECK_THROWS(multi_otsu::compute_thresholds(bad_hist));

    std::uint8_t px = 42;
    CHECK_THROWS(multi_otsu::apply_thresholds(nullptr, 1, {80, 160}));
    CHECK_THROWS(multi_otsu::apply_thresholds(&px, 0, {80, 160}));
}

// ─────────────────────────────────────────────────────────────────────────────
int main()
{
    test_trimodal_separation();
    test_all_pixels_same_class();
    test_classify_boundaries();
    test_label_count();
    test_histogram_normalisation();
    test_invalid_inputs();

    const int total = g_pass + g_fail;
    std::cout << (g_fail == 0 ? "PASSED" : "FAILED")
              << "  " << g_pass << "/" << total << " tests\n";

    return g_fail == 0 ? 0 : 1;
}
