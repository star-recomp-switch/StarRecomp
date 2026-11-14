// KirbyRecomp Generator - Parser Implementations
// NSO parsing, NCA extraction, prod.keys parsing

#include "parser.h"
#include <fstream>
#include <vector>
#include <cstring>
#include <iostream>

namespace kirbyrecomp::gen {

// ============================================================================
// prod.keys parsing
// ============================================================================

bool ParseProdKeys(const fs::path& keys_path, KeySet& keys) {
    std::ifstream file(keys_path);
    if (!file) {
        std::cerr << "[Parser] Cannot open keys file: " << keys_path << "\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        // Parse key=value pairs
        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos) continue;

        std::string key = line.substr(0, eq_pos);
        std::string value = line.substr(eq_pos + 1);

        // Trim whitespace
        auto trim = [](std::string& s) {
            size_t start = s.find_first_not_of(" \t\r\n");
            size_t end = s.find_last_not_of(" \t\r\n");
            if (start == std::string::npos) { s.clear(); return; }
            s = s.substr(start, end - start + 1);
        };
        trim(key);
        trim(value);

        // Store keys (simplified - real implementation would parse hex strings)
        keys.raw_keys[key] = value;
    }

    // Derive title keys for Kirby and the Forgotten Land (01003FB00C5A8000)
    // TODO: Implement proper key derivation using AES-XTS/KW
    keys.title_id = "01003FB00C5A8000";
    keys.keys_parsed = true;

    std::cout << "[Parser] Parsed " << keys.raw_keys.size() << " keys from prod.keys\n";
    return true;
}

// ============================================================================
// NSO Parsing
// ============================================================================

// NSO Header structure (from SwitchRecomp)
#pragma pack(push, 1)
struct NsoHeader {
    char magic[4];           // "NSO0"
    uint32_t version;        // 0
    uint32_t flags;          // Compression flags
    uint64_t module_offset;  // Offset to module name
    uint32_t module_name_size;
    uint32_t reserved1;
    uint64_t text_offset;
    uint64_t text_size;
    uint64_t text_compressed_size;
    uint64_t rodata_offset;
    uint64_t rodata_size;
    uint64_t rodata_compressed_size;
    uint64_t data_offset;
    uint64_t data_size;
    uint64_t data_compressed_size;
    uint8_t build_id[32];    // SHA-256
    uint8_t reserved2[32];
    uint8_t text_hash[32];
    uint8_t rodata_hash[32];
    uint8_t data_hash[32];
    uint8_t module_name[0];  // Variable length
};
#pragma pack(pop)

bool ParseNsoHeader(std::istream& stream, NsoModule& module) {
    NsoHeader header;
    stream.read(reinterpret_cast<char*>(&header), sizeof(NsoHeader));
    if (!stream) {
        std::cerr << "[Parser] Failed to read NSO header\n";
        return false;
    }

    // Validate magic
    if (std::memcmp(header.magic, "NSO0", 4) != 0) {
        std::cerr << "[Parser] Invalid NSO magic\n";
        return false;
    }

    module.header = header;
    module.name = std::string(header.module_name, header.module_name_size);

    // Read module name
    if (header.module_name_size > 0) {
        std::vector<char> name_buf(header.module_name_size);
        stream.read(name_buf.data(), header.module_name_size);
        module.name = std::string(name_buf.data(), header.module_name_size);
    }

    std::cout << "[Parser] NSO: " << module.name
              << " text=" << header.text_size
              << " rodata=" << header.rodata_size
              << " data=" << header.data_size << "\n";

    return true;
}

bool ReadNsoSection(std::istream& stream, const NsoHeader& header,
                    uint64_t offset, uint64_t size, uint64_t compressed_size,
                    std::vector<uint8_t>& output) {
    // Seek to section
    stream.seekg(offset, std::ios::beg);
    if (!stream) {
        std::cerr << "[Parser] Failed to seek to section\n";
        return false;
    }

    if (compressed_size == size || compressed_size == 0) {
        // Uncompressed
        output.resize(size);
        stream.read(reinterpret_cast<char*>(output.data()), size);
    } else {
        // LZ4 compressed
        std::vector<uint8_t> compressed(compressed_size);
        stream.read(reinterpret_cast<char*>(compressed.data()), compressed_size);
        // TODO: Decompress with LZ4
        std::cerr << "[Parser] LZ4 decompression not yet implemented\n";
        return false;
    }

    return stream.good();
}

bool ParseNsoModule(const fs::path& nso_path, NsoModule& module) {
    std::ifstream file(nso_path, std::ios::binary);
    if (!file) {
        std::cerr << "[Parser] Cannot open NSO: " << nso_path << "\n";
        return false;
    }

    if (!ParseNsoHeader(file, module)) {
        return false;
    }

    // Read sections
    if (!ReadNsoSection(file, module.header,
                        module.header.text_offset,
                        module.header.text_size,
                        module.header.text_compressed_size,
                        module.text_section)) {
        return false;
    }

    if (!ReadNsoSection(file, module.header,
                        module.header.rodata_offset,
                        module.header.rodata_size,
                        module.header.rodata_compressed_size,
                        module.rodata_section)) {
        return false;
    }

    if (!ReadNsoSection(file, module.header,
                        module.header.data_offset,
                        module.header.data_size,
                        module.header.data_compressed_size,
                        module.data_section)) {
        return false;
    }

    return true;
}

// ============================================================================
// NCA Extraction (simplified)
// ============================================================================

bool ExtractNca(const fs::path& nca_path, const KeySet& keys, fs::path output_dir) {
    // TODO: Implement NCA extraction using hactool/nscb as reference
    // This requires:
    // 1. Parse NCA header
    // 2. Decrypt sections using keys
    // 3. Extract FS (RomFS/ExeFS)
    std::cerr << "[Parser] NCA extraction not yet implemented\n";
    return false;
}

} // namespace kirbyrecomp::gen