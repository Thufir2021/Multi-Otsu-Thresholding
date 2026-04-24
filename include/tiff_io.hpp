#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Image8 {
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> data;
};

Image8 load_tiff_8bit(const std::string& path);
void   save_tiff_8bit(const std::string& path, const Image8& img);