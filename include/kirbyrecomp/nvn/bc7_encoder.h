#pragma once

// BC7 Encoder (Mode 6 - RGB + Alpha)
// Pure C++ implementation of BC7 Mode 6 encoding for 4x4 blocks

#include <cstdint>
#include <cstring>

namespace kirbyrecomp::nvn::bc7 {

// BC7 Block structure (16 bytes)
struct BC7Block {
    uint8_t data[16];
};

// Encode a 4x4 RGBA block to BC7 Mode 6
// rgba_block: 4x4 block of RGBA8 pixels (stride = bytes per row)
// width, height: dimensions of the block (must be 4x4)
// stride: bytes per row in the source buffer
// block: output BC7 block
void encode_mode6(const uint8_t* rgba_block, uint32_t width, uint32_t height, uint32_t stride, BC7Block& block);

} // namespace kirbyrecomp::nvn::bc7