// KirbyRecomp BFRES Parser
// Parses Nintendo BFRES (Binary Resource) files to extract texture data
// Supports Switch (NSW) BFRES format with ASTC compressed textures

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <span>
#include <unordered_map>
#include <cstring>

namespace kirbyrecomp::bfres {

// BFRES file header (Switch/NSW format)
struct FileHeader {
    uint32_t magic;           // "FRES" (0x53455246 little-endian)
    uint32_t padding;         // 4 spaces (0x20202020)
    uint32_t version;         // Version number
    uint32_t bom_alignment;   // BOM (0xFEFF big-endian) | alignment << 16
    uint32_t header_size;     // Size of header
    uint32_t flags;           // Flags
    uint32_t file_size;       // Total file size
    uint32_t root_offset;     // Offset to root section (RelocationTable)
    uint32_t section_count;   // Number of sections
    // Followed by section table entries
} __attribute__((packed));

// Section table entry
struct SectionEntry {
    char name[4];            // Section name (e.g., "FTEX", "FMDL", "FSKA")
    uint32_t offset;         // Offset from start of file
    uint32_t size;           // Size of section
} __attribute__((packed));

// Relocation table entry
struct RelocationEntry {
    uint32_t offset;         // Offset within section
    uint32_t type;           // Relocation type
    uint32_t reference;      // Reference index
} __attribute__((packed));

// Dictionary entry (ResDict)
struct DictEntry {
    uint32_t key_offset;     // Offset to string key
    uint32_t value_offset;   // Offset to value
} __attribute__((packed));

// String entry
struct StringEntry {
    uint32_t offset;         // Offset to string data
    uint32_t length;         // String length
} __attribute__((packed));

// Texture format enum (matches NVN texture formats)
enum class TextureFormat : uint32_t {
    // ASTC formats
    ASTC_4x4_UNORM = 0x93B0,
    ASTC_5x4_UNORM = 0x93B1,
    ASTC_5x5_UNORM = 0x93B2,
    ASTC_6x5_UNORM = 0x93B3,
    ASTC_6x6_UNORM = 0x93B4,
    ASTC_8x5_UNORM = 0x93B5,
    ASTC_8x6_UNORM = 0x93B6,
    ASTC_8x8_UNORM = 0x93B7,
    ASTC_10x5_UNORM = 0x93B8,
    ASTC_10x6_UNORM = 0x93B9,
    ASTC_10x8_UNORM = 0x93BA,
    ASTC_10x10_UNORM = 0x93BB,
    ASTC_12x10_UNORM = 0x93BC,
    ASTC_12x12_UNORM = 0x93BD,
    ASTC_4x4_SRGB = 0x93D0,
    ASTC_5x4_SRGB = 0x93D1,
    ASTC_5x5_SRGB = 0x93D2,
    ASTC_6x5_SRGB = 0x93D3,
    ASTC_6x6_SRGB = 0x93D4,
    ASTC_8x5_SRGB = 0x93D5,
    ASTC_8x6_SRGB = 0x93D6,
    ASTC_8x8_SRGB = 0x93D7,
    ASTC_10x5_SRGB = 0x93D8,
    ASTC_10x6_SRGB = 0x93D9,
    ASTC_10x8_SRGB = 0x93DA,
    ASTC_10x10_SRGB = 0x93DB,
    ASTC_12x10_SRGB = 0x93DC,
    ASTC_12x12_SRGB = 0x93DD,

    // Other formats
    RGBA8_UNORM = 0x1908,
    RGB8_UNORM = 0x1907,
    BC1_UNORM = 0x83F0,
    BC2_UNORM = 0x83F1,
    BC3_UNORM = 0x83F2,
    BC4_UNORM = 0x83F3,
    BC5_UNORM = 0x83F4,
    BC6H_UFLOAT = 0x8E8F,
    BC6H_SFLOAT = 0x8E90,
    BC7_UNORM = 0x8E8C,
    BC7_SRGB = 0x8E8D,
};

// Texture info from FTEX section
struct TextureInfo {
    std::string name;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mip_levels;
    uint32_t array_layers;
    TextureFormat format;
    uint32_t data_offset;    // Offset to texture data within file
    uint32_t data_size;      // Size of texture data
    std::vector<uint32_t> mip_offsets;  // Offsets for each mip level
    std::vector<uint32_t> mip_sizes;    // Sizes for each mip level
};

// BNTX texture info
struct BntxTextureInfo {
    std::string name;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mip_levels;
    uint32_t array_layers;
    TextureFormat format;
    uint32_t data_offset;    // Offset within BNTX section
    uint32_t data_size;      // Size of texture data
    std::vector<uint32_t> mip_offsets;
    std::vector<uint32_t> mip_sizes;

    // NVN texture parameters
    uint32_t dimension;      // Texture dimension (1D, 2D, 3D, Cube, etc.)
    uint32_t swizzle;        // Swizzle pattern
    uint32_t tile_mode;      // Tile mode
    uint32_t alignment;      // Alignment
    uint32_t pitch;          // Row pitch
};

// BFRES file parser
class BfresFile {
public:
    explicit BfresFile(const std::vector<uint8_t>& data);
    explicit BfresFile(const std::string& filepath);

    bool parse();

    // Get all textures (from FTEX section)
    const std::vector<TextureInfo>& get_textures() const { return textures_; }

    // Get all textures from BNTX section
    const std::vector<BntxTextureInfo>& get_bntx_textures() const { return bntx_textures_; }

    // Get texture data by index (FTEX)
    std::optional<std::vector<uint8_t>> get_texture_data(size_t index) const;

    // Get texture data by name (FTEX)
    std::optional<std::vector<uint8_t>> get_texture_data(const std::string& name) const;

    // Get BNTX texture data by index
    std::optional<std::vector<uint8_t>> get_bntx_texture_data(size_t index) const;

    // Get BNTX texture data by name
    std::optional<std::vector<uint8_t>> get_bntx_texture_data(const std::string& name) const;

    // Get section data by name
    std::optional<std::span<const uint8_t>> get_section(const std::string& name) const;

    // Check if file is valid
    bool is_valid() const { return valid_; }

    // Get error message
    const std::string& get_error() const { return error_; }

private:
    std::vector<uint8_t> data_;
    std::string filepath_;
    bool valid_ = false;
    std::string error_;

    FileHeader header_;
    std::vector<SectionEntry> sections_;
    std::unordered_map<std::string, SectionEntry> section_map_;
    std::vector<TextureInfo> textures_;
    std::vector<BntxTextureInfo> bntx_textures_;

    // Parsing helpers
    bool parse_header();
    bool parse_sections();
    bool parse_ftex_section();
    bool parse_bntx_section();
    bool parse_str_section(const uint8_t* data, size_t size, std::vector<std::string>& names);
    bool parse_brti_section(const uint8_t* data, size_t size, const std::vector<std::string>& names, uint32_t texture_data_base);
    bool parse_dict(const uint8_t* data, size_t size, std::unordered_map<std::string, uint32_t>& dict);
    std::string read_string(uint32_t offset);

    // Endian handling - Switch BFRES is always little-endian
    template<typename T>
    T read_le(size_t offset) const {
        if (offset + sizeof(T) > data_.size()) return T{};
        T value;
        memcpy(&value, &data_[offset], sizeof(T));
        return value;
    }
};

} // namespace kirbyrecomp::bfres