// KirbyRecomp Generator - Parser Header

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

namespace kirbyrecomp::gen {

struct KeySet {
    std::unordered_map<std::string, std::string> raw_keys;
    std::string title_id;
    bool keys_parsed = false;
    // Derived keys would go here
};

#pragma pack(push, 1)
struct NsoHeader {
    char magic[4];
    uint32_t version;
    uint32_t flags;
    uint64_t module_offset;
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
    uint8_t build_id[32];
    uint8_t reserved2[32];
    uint8_t text_hash[32];
    uint8_t rodata_hash[32];
    uint8_t data_hash[32];
};
#pragma pack(pop)

struct NsoModule {
    NsoHeader header;
    std::string name;
    fs::path path;
    bool is_main = false;
    std::vector<uint8_t> text_section;
    std::vector<uint8_t> rodata_section;
    std::vector<uint8_t> data_section;
};

bool ParseProdKeys(const fs::path& keys_path, KeySet& keys);
bool ParseNsoModule(const fs::path& nso_path, NsoModule& module);
bool ExtractNca(const fs::path& nca_path, const KeySet& keys, fs::path output_dir);

} // namespace kirbyrecomp::gen