// BC7 Encoder Implementation (Mode 6 - RGB + Alpha)
// Standalone implementation for ASTC -> BC7 transcoding

#include "kirbyrecomp/nvn/bc7_encoder.h"

#include <cstdint>
#include <algorithm>
#include <cstring>

namespace kirbyrecomp::nvn::bc7 {

// Encode a 4x4 block of RGBA to BC7 Mode 6
static void encode_mode6_impl(const uint8_t* rgba_block, uint32_t width, uint32_t height, uint32_t stride, BC7Block& block) {
    // Find min/max for R, G, B, A channels
    uint8_t min_r = 255, max_r = 0;
    uint8_t min_g = 255, max_g = 0;
    uint8_t min_b = 255, max_b = 0;
    uint8_t min_a = 255, max_a = 0;

    uint32_t count = 0;
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t* pixel = rgba_block + y * stride + x * 4;
            min_r = std::min(min_r, pixel[0]);
            max_r = std::max(max_r, pixel[0]);
            min_g = std::min(min_g, pixel[1]);
            max_g = std::max(max_g, pixel[1]);
            min_b = std::min(min_b, pixel[2]);
            max_b = std::max(max_b, pixel[2]);
            min_a = std::min(min_a, pixel[3]);
            max_a = std::max(max_a, pixel[3]);
            count++;
        }
    }

    // Encode endpoints (7 bits each, P-bit adds 1 bit precision)
    // P-bit is 1 for endpoints that need extra precision
    uint8_t p_bit_r = (max_r - min_r) > 127 ? 1 : 0;
    uint8_t p_bit_g = (max_g - min_g) > 127 ? 1 : 0;
    uint8_t p_bit_b = (max_b - min_b) > 127 ? 1 : 0;
    uint8_t p_bit_a = (max_a - min_a) > 127 ? 1 : 0;

    uint16_t ep_r0 = (min_r << 1) | p_bit_r;
    uint16_t ep_r1 = (max_r << 1) | p_bit_r;
    uint16_t ep_g0 = (min_g << 1) | p_bit_g;
    uint16_t ep_g1 = (max_g << 1) | p_bit_g;
    uint16_t ep_b0 = (min_b << 1) | p_bit_b;
    uint16_t ep_b1 = (max_b << 1) | p_bit_b;
    uint16_t ep_a0 = (min_a << 1) | p_bit_a;
    uint16_t ep_a1 = (max_a << 1) | p_bit_a;

    // Compute indices for each texel (2-bit per channel per texel = 4 indices per texel for RGB+A)
    // Mode 6 uses 3-bit indices per texel per channel (but we simplify to 2-bit)
    uint8_t indices[16][4]; // [texel][channel]

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t* pixel = rgba_block + y * stride + x * 4;
            uint32_t idx = y * 4 + x;

            // Quantize to 3-bit index (0-7) for each channel
            // This is a simplified quantization
            for (int c = 0; c < 4; ++c) {
                uint8_t v = pixel[c];
                uint16_t ep0 = (c == 0) ? ep_r0 : (c == 1) ? ep_g0 : (c == 2) ? ep_b0 : ep_a0;
                uint16_t ep1 = (c == 0) ? ep_r1 : (c == 1) ? ep_g1 : (c == 2) ? ep_b1 : ep_a1;
                uint32_t range = ep1 - ep0;
                if (range == 0) {
                    indices[idx][c] = 0;
                } else {
                    // Normalize to 0-7 (3 bits)
                    indices[idx][c] = static_cast<uint8_t>((v * 7) / 255);
                }
            }
        }
    }

    // BC7 Block structure
    struct BC7BlockInternal {
        uint8_t data[16];
    } local_block;

    memset(local_block.data, 0, 16);

    // Mode bits (6 bits in bits 0-5 of byte 0) - actually mode is in LSBs
    // Mode 6 = 0b110110 = 0x36
    local_block.data[0] = 0x36; // mode 6

    // Endpoint data (7 bits each, packed)
    // We'll pack R,G,B,A endpoints sequentially
    uint32_t bit_pos = 6;

    auto write_bits = [&](uint32_t val, uint32_t bits) {
        for (uint32_t i = 0; i < bits; ++i) {
            uint32_t byte_idx = bit_pos / 8;
            uint32_t bit_idx = bit_pos % 8;
            if (val & (1u << i)) {
                local_block.data[byte_idx] |= (1u << bit_idx);
            }
            bit_pos++;
        }
    };

    // Write endpoints (7 bits each)
    write_bits(ep_r0, 7);
    write_bits(ep_r1, 7);
    write_bits(ep_g0, 7);
    write_bits(ep_g1, 7);
    write_bits(ep_b0, 7);
    write_bits(ep_b1, 7);
    write_bits(ep_a0, 7);
    write_bits(ep_a1, 7);

    // P-bits (1 bit per endpoint pair = 4 bits)
    write_bits(p_bit_r, 1);
    write_bits(p_bit_g, 1);
    write_bits(p_bit_b, 1);
    write_bits(p_bit_a, 1);

    // Partition bits (6 bits for up to 3 partitions, but we use 0)
    write_bits(0, 6);

    // Rotation bits (2 bits) - 0
    write_bits(0, 2);

    // Index mode bits (3 bits) - 0
    write_bits(0, 2);

    // Write indices (3 bits per texel)
    for (uint32_t i = 0; i < 16; ++i) {
        // Use R channel index as the shared index
        uint8_t index = indices[i][0];
        write_bits(index, 3);
    }

    // Copy to output
    std::memcpy(&block.data[0], local_block.data, 16);
}

void encode_mode6(const uint8_t* rgba_block, uint32_t width, uint32_t height, uint32_t stride, BC7Block& block) {
    encode_mode6_impl(rgba_block, width, height, stride, block);
}

} // namespace kirbyrecomp::nvn::bc7