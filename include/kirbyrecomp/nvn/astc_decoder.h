#pragma once

// ASTC LDR Decoder
// Pure C++ implementation of ASTC LDR decoding for all 2D block sizes (4x4 through 12x12)

#include <cstdint>

namespace kirbyrecomp::nvn::astc {

// Decode an ASTC LDR block to RGBA8
// block_data: 16 bytes of ASTC block data
// block_w, block_h: ASTC block dimensions (4-12)
// output_rgba: output buffer (4 bytes per pixel, RGBA order)
// output_stride: bytes per row in output buffer
// Returns true on success, false on failure (invalid block mode, etc.)
bool decode_block(const uint8_t* block_data, uint32_t block_w, uint32_t block_h,
                  uint8_t* output_rgba, uint32_t output_stride);

} // namespace kirbyrecomp::nvn::astc