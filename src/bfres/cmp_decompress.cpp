// CMP Decompressor for Kirby and the Forgotten Land
// Decompresses .cmp files (Zstd with custom header)

#include <cstdint>
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <zstd.h>

namespace kirbyrecomp::bfres {

bool decompress_cmp_file(const std::string& input_path, std::string& output_path) {
    FILE* f = fopen(input_path.c_str(), "rb");
    if (!f) {
        return false;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size < 4) {
        fclose(f);
        return false;
    }

    // Read decompressed size (first 4 bytes)
    uint32_t decompressed_size;
    size_t read = fread(&decompressed_size, 1, 4, f);
    if (read != 4) {
        fclose(f);
        return false;
    }

    // Read the rest as Zstd data (includes magic at offset 4)
    size_t compressed_size = static_cast<size_t>(file_size) - 4;
    std::vector<uint8_t> compressed_data(compressed_size);
    read = fread(compressed_data.data(), 1, compressed_size, f);
    fclose(f);

    if (read != compressed_size) {
        return false;
    }

    // Verify Zstd magic at start of compressed data
    if (compressed_size < 4) {
        return false;
    }
    uint32_t magic = *reinterpret_cast<uint32_t*>(compressed_data.data());
    if (magic != 0xFD2FB528) {
        return false;
    }

    // Decompress using Zstd
    std::vector<uint8_t> decompressed_data(decompressed_size);
    size_t result = ZSTD_decompress(
        decompressed_data.data(),
        decompressed_size,
        compressed_data.data(),
        compressed_size
    );

    if (ZSTD_isError(result)) {
        return false;
    }

    if (result != decompressed_size) {
        return false;
    }

    // Write output
    FILE* out = fopen(output_path.c_str(), "wb");
    if (!out) {
        return false;
    }

    size_t written = fwrite(decompressed_data.data(), 1, decompressed_data.size(), out);
    fclose(out);

    return written == decompressed_data.size();
}

bool decompress_cmp_buffer(const uint8_t* data, size_t size, std::vector<uint8_t>& output) {
    if (size < 4) return false;

    uint32_t decompressed_size = *reinterpret_cast<const uint32_t*>(data);

    const uint8_t* compressed_data = data + 4;
    size_t compressed_size = size - 4;

    if (compressed_size < 4) return false;
    uint32_t magic = *reinterpret_cast<const uint32_t*>(compressed_data);
    if (magic != 0xFD2FB528) return false;

    output.resize(decompressed_size);
    size_t result = ZSTD_decompress(
        output.data(),
        decompressed_size,
        compressed_data,
        compressed_size
    );

    if (ZSTD_isError(result)) return false;
    if (result != decompressed_size) return false;

    return true;
}

} // namespace kirbyrecomp::bfres

// Simple main for testing
#ifdef CMP_DECOMPRESS_MAIN
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.cmp> <output.bfres>" << std::endl;
        return 1;
    }

    using namespace kirbyrecomp::bfres;
    std::string input_path = argv[1];
    std::string output_path = argv[2];
    if (decompress_cmp_file(input_path, output_path)) {
        std::cout << "Decompressed successfully!" << std::endl;
        return 0;
    } else {
        std::cerr << "Decompression failed!" << std::endl;
        return 1;
    }
}
#endif