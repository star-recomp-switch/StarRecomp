// CMP Decompressor for Kirby and the Forgotten Land
// Decompresses .cmp files (Zstd with custom header)

#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace kirbyrecomp::bfres {

// Decompress a .cmp file to a .bfres file
// Returns true on success
bool decompress_cmp_file(const std::string& input_path, std::string& output_path);

// Decompress a .cmp buffer in memory
// Returns true on success, output contains decompressed data
bool decompress_cmp_buffer(const uint8_t* data, size_t size, std::vector<uint8_t>& output);

} // namespace kirbyrecomp::bfres