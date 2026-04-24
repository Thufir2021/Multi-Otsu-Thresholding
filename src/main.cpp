/**
 * main.cpp  —  CLI demo for multi_otsu.hpp
 *
 * Reads a raw 8-bit grayscale binary file (width × height bytes, row-major),
 * computes the two Otsu thresholds, prints them and per-class statistics.
 *
 * This keeps the demo dependency-free: no image codec, no OpenCV.
 * To use with PNG/JPEG images, pipe through ImageMagick or replace
 * load_raw() with any decoder of your choice.
 *
 * Usage:
 *   ./multi_otsu_demo <raw_file> <width> <height>
 *
 * Example (create a test file with ImageMagick):
 *   convert input.png -depth 8 gray:image.raw
 *   ./multi_otsu_demo image.raw 512 512
 *
 * Build (no dependencies beyond the C++17 stdlib):
 *   cmake -B build && cmake --build build
 *   — or —
 *   g++ -std=c++17 -O2 -Iinclude src/main.cpp -o multi_otsu_demo
 */

#include "multi_otsu.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
static std::vector<std::uint8_t> load_raw(const std::string& path,
                                          std::size_t         expected_bytes)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
        throw std::runtime_error("Cannot open file: " + path);

    std::vector<std::uint8_t> buf(expected_bytes);
    if (!f.read(reinterpret_cast<char*>(buf.data()),
                static_cast<std::streamsize>(expected_bytes)))
        throw std::runtime_error("File too small or read error: " + path);

    return buf;
}

// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <raw_file> <width> <height>\n"
                  << "\n"
                  << "  raw_file : flat 8-bit grayscale binary (width*height bytes)\n"
                  << "  width    : image width  in pixels\n"
                  << "  height   : image height in pixels\n";
        return 1;
    }

    const std::string path   = argv[1];
    const std::size_t width  = static_cast<std::size_t>(std::stoul(argv[2]));
    const std::size_t height = static_cast<std::size_t>(std::stoul(argv[3]));
    const std::size_t count  = width * height;

    // ── Load ──────────────────────────────────────────────────────────────────
    std::vector<std::uint8_t> pixels;
    try {
        pixels = load_raw(path, count);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    // ── Segment ───────────────────────────────────────────────────────────────
    multi_otsu::Thresholds t;
    std::vector<multi_otsu::Label> labels;

    try {
        labels = multi_otsu::segment(pixels.data(), count, t);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    // ── Report ────────────────────────────────────────────────────────────────
    std::cout << "Thresholds : t1 = " << t.t1 << ", t2 = " << t.t2 << "\n";

    std::size_t class_count[3] = {0, 0, 0};
    for (const auto& l : labels)
        ++class_count[static_cast<int>(l)];

    const char* names[3] = {"Low  [0,   t1]",
                             "Mid  (t1,  t2]",
                             "High (t2, 255]"};
    for (int c = 0; c < 3; ++c) {
        const double pct = 100.0 * static_cast<double>(class_count[c])
                                 / static_cast<double>(count);
        std::cout << "  Class " << c << "  " << names[c]
                  << " : " << class_count[c]
                  << " px  (" << pct << " %)\n";
    }

    return 0;
}
