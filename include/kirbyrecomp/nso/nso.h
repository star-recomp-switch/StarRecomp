// NSO Parser for Nintendo Switch Object files
// Based on SwitchRecomp's NSO format specification
// NOTE: Uses C-style string paths to avoid std::filesystem linking issues on mingw64

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <span>
#include <array>

namespace kirbyrecomp::nso {

// NSO Section types
enum class SectionType : uint8_t {
    Text    = 0,  // .text - executable code
    Rodata  = 1,  // .rodata - read-only data
    Data    = 2,  // .data - read-write data
    Bss     = 3,  // .bss - zero-initialized data
    Count   = 4
};

// NSO Compression types
enum class CompressionType : uint8_t {
    None    = 0,
    LZ4     = 1,
    Unknown = 0xFF
};

// Relocation types (AArch64 ELF relocations)
enum class RelocType : uint32_t {
    None                    = 0,
    Absolute64              = 257,  // R_AARCH64_ABS64
    Absolute32              = 258,  // R_AARCH64_ABS32
    Absolute16              = 259,  // R_AARCH64_ABS16
    PreL31                  = 260,  // R_AARCH64_PREL31
    PreL64                  = 261,  // R_AARCH64_PREL64
    PreL16                  = 262,  // R_AARCH64_PREL16
    MovW_Uabs_G0            = 263,  // R_AARCH64_MOVW_UABS_G0
    MovW_Uabs_G0_NC         = 264,  // R_AARCH64_MOVW_UABS_G0_NC
    MovW_Uabs_G1            = 265,  // R_AARCH64_MOVW_UABS_G1
    MovW_Uabs_G1_NC         = 266,  // R_AARCH64_MOVW_UABS_G1_NC
    MovW_Uabs_G2            = 267,  // R_AARCH64_MOVW_UABS_G2
    MovW_Uabs_G2_NC         = 268,  // R_AARCH64_MOVW_UABS_G2_NC
    MovW_Uabs_G3            = 269,  // R_AARCH64_MOVW_UABS_G3
    MovW_Sabs_G0            = 270,  // R_AARCH64_MOVW_SABS_G0
    MovW_Sabs_G1            = 271,  // R_AARCH64_MOVW_SABS_G1
    MovW_Sabs_G2            = 272,  // R_AARCH64_MOVW_SABS_G2
    LdPreL19                = 273,  // R_AARCH64_LD_PREL_19
    AddAbs                  = 274,  // R_AARCH64_ADR_PREL_LO21
    LdPreL64                = 275,  // R_AARCH64_LD64_GOTOFF_LO15
};

// NSO Header (256 bytes, 0x100 bytes total) - matches SwitchRecomp format
#pragma pack(push, 1)
struct NsoHeader {
    // Magic: "NSO0" at offset 0x00
    char magic[4];

    // Version at offset 0x04
    uint32_t version;

    // Flags at offset 0x0C
    uint32_t flags;

    // Reserved at 0x10 (4 bytes)
    uint32_t reserved_0x10;

    // Module name size (parser compatibility)
    uint32_t module_name_size;

    // BSS size at 0x3C
    uint32_t bss_size;

    // Section info (Text, Rodata, Data) - each 16 bytes at 0x10, 0x20, 0x30
    // Parser expects these member names
    struct SectionInfo {
        uint32_t offset;          // file offset
        uint32_t mem_offset;      // memory offset
        uint32_t dec_size;        // decompressed size
        uint32_t reserved;

        // Parser compatibility - expose as size and compressed_size
        uint32_t size() const { return dec_size; }
        uint32_t compressed_size() const { return 0; }  // Will be set from compressed_sizes array
    } sections[3];

    // Compressed sizes at offset 0x60 (4 bytes each)
    uint32_t compressed_sizes[3];

    // Build ID (SHA-256) at 0x40
    uint8_t build_id[32];

    // Reserved
    uint8_t reserved2[32];

    // Section hashes (SHA-256 each) at 0x80, 0xA0, 0xC0
    uint8_t text_hash[32];
    uint8_t rodata_hash[32];
    uint8_t data_hash[32];

    // Padding to 0x100 bytes
    uint8_t padding[0x100 - (4 + 4 + 4 + 4 + 4 + 48 + 12 + 12 + 32 + 32 + 32 + 32 + 32)];
};
#pragma pack(pop)

// Relocation entry
struct Relocation {
    RelocType type;
    uint32_t section;      // Which section (0=text, 1=rodata, 2=data)
    uint64_t offset;       // Offset within section
    uint64_t addend;       // Addend value
    uint32_t symbol_index; // Symbol index (if applicable)
};

// Section data
struct Section {
    SectionType type;
    uint64_t file_offset;
    uint64_t virtual_offset;  // Relative to module base
    uint64_t size;
    uint64_t compressed_size;
    CompressionType compression;
    std::vector<uint8_t> data;
    std::vector<Relocation> relocations;
};

// Parsed NSO module
struct NsoModule {
    NsoHeader header;
    std::string module_name;
    std::array<Section, 3> sections;  // Text, Rodata, Data
    uint64_t bss_size = 0;

    // Get section by type
    const Section* get_section(SectionType type) const {
        size_t idx = static_cast<size_t>(type);
        if (idx < sections.size()) return &sections[idx];
        return nullptr;
    }

    Section* get_section(SectionType type) {
        size_t idx = static_cast<size_t>(type);
        if (idx < sections.size()) return &sections[idx];
        return nullptr;
    }
};

// NSO Parser class
class NsoParser {
public:
    NsoParser() = default;
    ~NsoParser() = default;

    // Parse NSO from file (C-style string path - avoids std::filesystem linking issues)
    std::optional<NsoModule> parse_file(const char* path);

    // Parse NSO from memory buffer
    std::optional<NsoModule> parse_buffer(std::span<const uint8_t> buffer);

    // Verify section hashes
    bool verify_hashes(const NsoModule& module);

    // Apply relocations to sections
    bool apply_relocations(NsoModule& module, uint64_t base_address);

    // Get last error
    std::string get_last_error() const { return last_error_; }

private:
    std::string last_error_;

    // Read and validate header
    bool read_header(std::span<const uint8_t> buffer, NsoModule& module, size_t& offset);

    // Read section data
    bool read_section(std::span<const uint8_t> buffer, const NsoHeader::SectionInfo& info,
                      uint32_t compressed_size, Section& section, size_t& offset);

    // Read relocations
    bool read_relocations(std::span<const uint8_t> buffer, Section& section);

    // Decompress LZ4
    bool decompress_lz4(const std::vector<uint8_t>& compressed,
                        std::vector<uint8_t>& output, uint64_t expected_size);

    // Compute SHA-256 hash
    std::array<uint8_t, 32> compute_sha256(std::span<const uint8_t> data);
};

// Section alignment
constexpr uint64_t SECTION_ALIGNMENT = 0x1000;  // 4KB page alignment

// Module base address (standard Switch NSO load address)
constexpr uint64_t DEFAULT_NSO_BASE_ADDRESS = 0x7100000000ULL;

} // namespace kirbyrecomp::nso