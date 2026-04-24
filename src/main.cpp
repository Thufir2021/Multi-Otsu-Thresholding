#include "tiff_io.hpp"
#include "multi_otsu.hpp"
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
static uint8_t label_to_pixel(multi_otsu::Label l)
{
    switch (l) {
        case multi_otsu::Label::Low:  return 0;
        case multi_otsu::Label::Mid:  return 127;
        case multi_otsu::Label::High: return 255;
    }
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <input.tif> <output.tif>\n";
        return 1;
    }

    try {
        auto img = load_tiff_8bit(argv[1]);

        multi_otsu::Thresholds t;
        auto labels = multi_otsu::segment(
            img.data.data(), img.data.size(), t);

        Image8 out;
        out.width  = img.width;
        out.height = img.height;
        out.data.resize(img.data.size());

        for (size_t i = 0; i < labels.size(); ++i)
            out.data[i] = label_to_pixel(labels[i]);

        save_tiff_8bit(argv[2], out);

        std::cout << "t1=" << t.t1
                  << " t2=" << t.t2 << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}