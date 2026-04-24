#include "tiff_io.hpp"
#include <tiffio.h>
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
static uint8_t rgb_to_gray(uint8_t r, uint8_t g, uint8_t b) {
    return static_cast<uint8_t>(0.299*r + 0.587*g + 0.114*b);
}

// ─────────────────────────────────────────────────────────────────────────────
Image8 load_tiff_8bit(const std::string& path)
{
    TIFF* tif = TIFFOpen(path.c_str(), "r");
    if (!tif)
        throw std::runtime_error("Cannot open TIFF: " + path);

    uint32_t width, height;
    uint16_t spp, bps;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH,  &width);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE,  &bps);

    std::vector<uint8_t> out(width * height);

    // ── 8-bit grayscale ──────────────────────────────────────────────────────
    if (bps == 8 && spp == 1) {
        for (uint32_t y = 0; y < height; ++y)
            TIFFReadScanline(tif, out.data() + y*width, y);
    }

    // ── 16-bit grayscale ─────────────────────────────────────────────────────
    else if (bps == 16 && spp == 1) {
        std::vector<uint16_t> buf(width);
        for (uint32_t y = 0; y < height; ++y) {
            TIFFReadScanline(tif, buf.data(), y);
            for (uint32_t x = 0; x < width; ++x)
                out[y*width + x] = static_cast<uint8_t>(buf[x] >> 8);
        }
    }

    // ── RGB (8-bit) ──────────────────────────────────────────────────────────
    else if (bps == 8 && spp >= 3) {
        std::vector<uint8_t> buf(width * spp);
        for (uint32_t y = 0; y < height; ++y) {
            TIFFReadScanline(tif, buf.data(), y);
            for (uint32_t x = 0; x < width; ++x) {
                out[y*width + x] = rgb_to_gray(
                    buf[x*spp + 0],
                    buf[x*spp + 1],
                    buf[x*spp + 2]
                );
            }
        }
    }

    else {
        TIFFClose(tif);
        throw std::runtime_error("Unsupported TIFF format");
    }

    TIFFClose(tif);
    return {width, height, std::move(out)};
}

// ─────────────────────────────────────────────────────────────────────────────
void save_tiff_8bit(const std::string& path, const Image8& img)
{
    TIFF* tif = TIFFOpen(path.c_str(), "w");
    if (!tif)
        throw std::runtime_error("Cannot write TIFF: " + path);

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, img.width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, img.height);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

    for (uint32_t y = 0; y < img.height; ++y)
        TIFFWriteScanline(tif,
            (void*)(img.data.data() + y*img.width), y);

    TIFFClose(tif);
}