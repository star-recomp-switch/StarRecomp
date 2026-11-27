// KirbyRecomp BFRES Parser Implementation

#include "kirbyrecomp/bfres/bfres.h"
#include <fstream>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <iomanip>

namespace kirbyrecomp::bfres {

// Convert BNTX format value to our TextureFormat enum
static TextureFormat bntx_format_to_texture_format(uint32_t bntx_format) {
    // BNTX format encoding (observed from KFL files):
    // 0x00010007 = ASTC_4x4_UNORM
    // 0x00010008 = ASTC_4x4_SRGB
    // 0x00020007 = ASTC_5x4_UNORM
    // 0x00020008 = ASTC_5x4_SRGB
    // 0x00030007 = ASTC_5x5_UNORM
    // 0x00030008 = ASTC_5x5_SRGB
    // 0x00040007 = ASTC_6x5_UNORM
    // 0x00040008 = ASTC_6x5_SRGB
    // 0x00050007 = ASTC_6x6_UNORM
    // 0x00050008 = ASTC_6x6_SRGB
    // 0x00060007 = ASTC_8x5_UNORM
    // 0x00060008 = ASTC_8x5_SRGB
    // 0x00070007 = ASTC_8x6_UNORM
    // 0x00070008 = ASTC_8x6_SRGB
    // 0x00080007 = ASTC_8x8_UNORM
    // 0x00080008 = ASTC_8x8_SRGB
    // 0x00090007 = ASTC_10x5_UNORM
    // 0x00090008 = ASTC_10x5_SRGB
    // 0x000A0007 = ASTC_10x6_UNORM
    // 0x000A0008 = ASTC_10x6_SRGB
    // 0x000B0007 = ASTC_10x8_UNORM
    // 0x000B0008 = ASTC_10x8_SRGB
    // 0x000C0007 = ASTC_10x10_UNORM
    // 0x000C0008 = ASTC_10x10_SRGB
    // 0x000D0007 = ASTC_12x10_UNORM
    // 0x000D0008 = ASTC_12x10_SRGB
    // 0x000E0007 = ASTC_12x12_UNORM
    // 0x000E0008 = ASTC_12x12_SRGB

    // Extract block dimensions from upper 16 bits
    uint16_t block_info = (bntx_format >> 16) & 0xFFFF;
    bool is_srgb = (bntx_format & 0xFFFF) == 0x0008;

    switch (block_info) {
        case 0x0001: return is_srgb ? TextureFormat::ASTC_4x4_SRGB : TextureFormat::ASTC_4x4_UNORM;
        case 0x0002: return is_srgb ? TextureFormat::ASTC_5x4_SRGB : TextureFormat::ASTC_5x4_UNORM;
        case 0x0003: return is_srgb ? TextureFormat::ASTC_5x5_SRGB : TextureFormat::ASTC_5x5_UNORM;
        case 0x0004: return is_srgb ? TextureFormat::ASTC_6x5_SRGB : TextureFormat::ASTC_6x5_UNORM;
        case 0x0005: return is_srgb ? TextureFormat::ASTC_6x6_SRGB : TextureFormat::ASTC_6x6_UNORM;
        case 0x0006: return is_srgb ? TextureFormat::ASTC_8x5_SRGB : TextureFormat::ASTC_8x5_UNORM;
        case 0x0007: return is_srgb ? TextureFormat::ASTC_8x6_SRGB : TextureFormat::ASTC_8x6_UNORM;
        case 0x0008: return is_srgb ? TextureFormat::ASTC_8x8_SRGB : TextureFormat::ASTC_8x8_UNORM;
        case 0x0009: return is_srgb ? TextureFormat::ASTC_10x5_SRGB : TextureFormat::ASTC_10x5_UNORM;
        case 0x000A: return is_srgb ? TextureFormat::ASTC_10x6_SRGB : TextureFormat::ASTC_10x6_UNORM;
        case 0x000B: return is_srgb ? TextureFormat::ASTC_10x8_SRGB : TextureFormat::ASTC_10x8_UNORM;
        case 0x000C: return is_srgb ? TextureFormat::ASTC_10x10_SRGB : TextureFormat::ASTC_10x10_UNORM;
        case 0x000D: return is_srgb ? TextureFormat::ASTC_12x10_SRGB : TextureFormat::ASTC_12x10_UNORM;
        case 0x000E: return is_srgb ? TextureFormat::ASTC_12x12_SRGB : TextureFormat::ASTC_12x12_UNORM;
        default: return TextureFormat::RGBA8_UNORM;
    }
}

BfresFile::BfresFile(const std::vector<uint8_t>& data)
    : data_(data) {}

BfresFile::BfresFile(const std::string& filepath)
    : filepath_(filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) {
        error_ = "Failed to open file: " + filepath;
        return;
    }

    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    data_.resize(size);
    file.read(reinterpret_cast<char*>(data_.data()), size);
    file.close();
}

bool BfresFile::parse() {
    if (data_.size() < sizeof(FileHeader)) {
        error_ = "File too small for BFRES header";
        return false;
    }

    if (!parse_header()) return false;
    if (!parse_sections()) return false;
    if (!parse_ftex_section()) return false;
    if (!parse_bntx_section()) return false;

    valid_ = true;
    return true;
}

bool BfresFile::parse_header() {
    // Read header as little-endian (Switch BFRES is always little-endian)
    std::memcpy(&header_, data_.data(), sizeof(FileHeader));

    // Check magic
    if (header_.magic != 0x53455246) {  // "FRES" in little-endian
        error_ = "Invalid BFRES magic: expected 'FRES', got 0x" + std::to_string(header_.magic);
        return false;
    }

    // Verify padding (4 spaces)
    if (header_.padding != 0x20202020) {
        // Not fatal, just a warning
    }

    // Extract BOM and alignment from bom_alignment
    uint16_t bom = header_.bom_alignment & 0xFFFF;
    uint16_t alignment = (header_.bom_alignment >> 16) & 0xFFFF;

    if (bom != 0xFEFF) {
        error_ = "Invalid BOM: expected 0xFEFF (big-endian), got 0x" + std::to_string(bom);
        return false;
    }

    // File size should match
    if (header_.file_size != data_.size()) {
        // Not fatal, file might have extra data
    }

    return true;
}

std::string BfresFile::read_string(uint32_t offset) {
    if (offset >= data_.size()) return "";

    // Find null terminator
    size_t len = 0;
    while (offset + len < data_.size() && data_[offset + len] != '\0') {
        ++len;
    }

    return std::string(reinterpret_cast<const char*>(&data_[offset]), len);
}

bool BfresFile::parse_sections() {
    std::fprintf(stderr, "=== PARSE_SECTIONS START ===\n");
    std::fprintf(stderr, "header_.section_count = %u, FileHeader size = %zu\n", header_.section_count, sizeof(FileHeader));

    // First try: use section table from header
    size_t section_table_offset = sizeof(FileHeader);
    if (section_table_offset + header_.section_count * sizeof(SectionEntry) <= data_.size() && header_.section_count > 0 && header_.section_count < 1000) {
        sections_.resize(header_.section_count);
        bool valid_table = true;
        for (uint32_t i = 0; i < header_.section_count; ++i) {
            size_t offset = section_table_offset + i * sizeof(SectionEntry);
            std::memcpy(&sections_[i], &data_[offset], sizeof(SectionEntry));

            // Validate section entry - offset and size should be reasonable
            if (sections_[i].offset >= data_.size() || sections_[i].size > data_.size() - sections_[i].offset) {
                valid_table = false;
                break;
            }

            // Create section name string
            std::string name(sections_[i].name, 4);
            // Reject entries with empty names or zero size
            if (name.find_first_not_of('\0') == std::string::npos || sections_[i].size == 0) {
                valid_table = false;
                break;
            }
            std::fprintf(stderr, "  Section table entry %u: name=%s, offset=%u, size=%u\n", i, name.c_str(), sections_[i].offset, sections_[i].size);
            section_map_[name] = sections_[i];
        }
        if (valid_table && !section_map_.empty()) {
            std::fprintf(stderr, "=== PARSE_SECTIONS: using header table ===\n");
            return true;
        }
    }

    // Fallback: scan file for known section signatures
    static const char* known_sections[] = {
        "FTEX", "FMDL", "FSKA", "FMAA", "FSHA", "FSCN", "FBNV", "FSHPA",
        "TEXPAT", "BNTX", "_STR", "FVTX", "FRAG", "FVSH", "FPRO", "FLOD",
        "FEDG", "FPTN", "FSEQ", "FCRV", "FANM", "FVIS", "FSRT", "FPAT",
        "FVCL", "FSCL", "FTRG", "FENV", "FOCC", "FPRM", "FUSR"
    };

    // Only scan in reasonable regions: after header, and at 16-byte aligned offsets
    size_t scan_start = sizeof(FileHeader);
    size_t scan_end = std::min(data_.size(), scan_start + 65536);  // Scan first 8KB after header

    for (size_t i = scan_start; i + 4 <= scan_end; i += 4) {
        // Check for section signature (4 ASCII chars) at aligned offset
        if (data_[i] >= 'A' && data_[i] <= 'Z' &&
            data_[i+1] >= 'A' && data_[i+1] <= 'Z' &&
            data_[i+2] >= 'A' && data_[i+2] <= 'Z' &&
            data_[i+3] >= 'A' && data_[i+3] <= 'Z') {

            std::string sig(reinterpret_cast<const char*>(&data_[i]), 4);

            // Check if it's a known section type
            bool known = false;
            for (const char* known_sig : known_sections) {
                if (sig == known_sig) {
                    known = true;
                    break;
                }
            }

            if (known && section_map_.find(sig) == section_map_.end()) {
                // Found a known section signature, verify by checking the section magic at the entry offset
                // For BNTX, we need special handling since the section entry in the section table may be invalid
                if (sig == "BNTX") {
                    // For BNTX, verify the magic at this offset and read the size from the BNTX header
                    std::fprintf(stderr, "[DEBUG] Checking BNTX at offset %zu\n", i);
                    if (i + 8 <= data_.size()) {
                        uint32_t bntx_magic = *reinterpret_cast<const uint32_t*>(&data_[i]);
                        uint32_t bntx_size = *reinterpret_cast<const uint32_t*>(&data_[i + 4]);
                        std::fprintf(stderr, "[DEBUG] BNTX check at %zu: magic=0x%08x, header_size=%u, remaining_file=%zu\n", i, bntx_magic, bntx_size, data_.size() - i);
                        if (bntx_magic == 0x58544E42) {
                            // If header size is 0 or too small, use remaining file size
                            if (bntx_size == 0 || bntx_size > data_.size() - i) {
                                bntx_size = static_cast<uint32_t>(data_.size() - i);
                                std::fprintf(stderr, "[DEBUG] BNTX header size invalid, using remaining file size: %u\n", bntx_size);
                            }
                            if (bntx_size > 0 && i + bntx_size <= data_.size()) {
                                SectionEntry entry;
                                entry.name[0] = 'B'; entry.name[1] = 'N'; entry.name[2] = 'T'; entry.name[3] = 'X';
                                entry.offset = static_cast<uint32_t>(i);
                                entry.size = bntx_size;
                                std::fprintf(stderr, "  Found BNTX via scan: offset=%u, size=%u\n", entry.offset, entry.size);
                                section_map_["BNTX"] = entry;
                                sections_.push_back(entry);
                            }
                        }
                    }
                } else {
                    // For other sections, use the standard SectionEntry validation
                    if (i + sizeof(SectionEntry) <= data_.size()) {
                        SectionEntry entry;
                        std::memcpy(&entry, &data_[i], sizeof(SectionEntry));

                        // Validate: offset should match current scan position, size should be reasonable
                        if (entry.offset == static_cast<uint32_t>(i) && entry.size > 0 && entry.size <= data_.size() - entry.offset) {
                            // Verify section magic at the entry offset
                            if (entry.offset + 4 <= data_.size()) {
                                uint32_t section_magic = *reinterpret_cast<const uint32_t*>(&data_[entry.offset]);
                                uint32_t expected_magic = 0;
                                if (sig == "FTEX") expected_magic = 0x58455446;
                                else if (sig == "FMDL") expected_magic = 0x4C444D46;
                                else if (sig == "FTEX") expected_magic = 0x58455446; // FTEX
                                else if (sig == "FMDL") expected_magic = 0x4C444D46; // FMDL

                                if (expected_magic == 0 || section_magic == expected_magic) {
                                    std::fprintf(stderr, "  Found section via scan: name=%s, offset=%u, size=%u\n", sig.c_str(), entry.offset, entry.size);
                                    section_map_[sig] = entry;
                                    sections_.push_back(entry);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    std::fprintf(stderr, "=== PARSE_SECTIONS END: found %zu sections ===\n", section_map_.size());
    for (const auto& [name, entry] : section_map_) {
        std::fprintf(stderr, "  %s: offset=%u, size=%u\n", name.c_str(), entry.offset, entry.size);
    }

    return true;
}

bool BfresFile::parse_dict(const uint8_t* data, size_t size,
                           std::unordered_map<std::string, uint32_t>& dict) {
    if (size < 8) return false;

    // Dict header: count, string_table_offset
    uint32_t count = *reinterpret_cast<const uint32_t*>(data);
    uint32_t string_table_offset = *reinterpret_cast<const uint32_t*>(data + 4);

    // Dict entries follow header
    const DictEntry* entries = reinterpret_cast<const DictEntry*>(data + 8);
    if (size < 8 + count * sizeof(DictEntry)) return false;

    for (uint32_t i = 0; i < count; ++i) {
        DictEntry entry = entries[i];
        std::string key = read_string(entry.key_offset);
        dict[key] = entry.value_offset;
    }

    return true;
}

bool BfresFile::parse_ftex_section() {
    auto it = section_map_.find("FTEX");
    if (it == section_map_.end()) {
        // No texture section
        return true;
    }

    const SectionEntry& ftex_section = it->second;
    if (ftex_section.offset + ftex_section.size > data_.size()) {
        error_ = "FTEX section extends beyond file";
        return false;
    }

    const uint8_t* ftex_data = &data_[ftex_section.offset];
    size_t ftex_size = ftex_section.size;

    // FTEX header: texture count, dict offset
    if (ftex_size < 8) {
        error_ = "FTEX section too small";
        return false;
    }

    uint32_t texture_count = *reinterpret_cast<const uint32_t*>(ftex_data);
    uint32_t dict_offset = *reinterpret_cast<const uint32_t*>(ftex_data + 4);

    // Parse texture name dictionary
    std::unordered_map<std::string, uint32_t> texture_dict;
    if (dict_offset < ftex_size) {
        if (!parse_dict(ftex_data + dict_offset, ftex_size - dict_offset, texture_dict)) {
            error_ = "Failed to parse FTEX dictionary";
            return false;
        }
    }

    // Texture entries follow the header
    const uint8_t* texture_entries = ftex_data + 8;

    for (uint32_t i = 0; i < texture_count; ++i) {
        size_t entry_offset = i * 56;  // Standard entry size
        if (entry_offset + 56 > ftex_size) break;

        // Parse texture entry
        uint32_t name_offset = *reinterpret_cast<const uint32_t*>(texture_entries + entry_offset);
        uint32_t width = *reinterpret_cast<const uint32_t*>(texture_entries + entry_offset + 4);
        uint32_t height = *reinterpret_cast<const uint32_t*>(texture_entries + entry_offset + 8);
        uint32_t depth = *reinterpret_cast<const uint32_t*>(texture_entries + entry_offset + 12);
        uint32_t mip_levels = *reinterpret_cast<const uint32_t*>(texture_entries + entry_offset + 16);
        uint32_t array_layers = *reinterpret_cast<const uint32_t*>(texture_entries + entry_offset + 20);
        uint32_t format = *reinterpret_cast<const uint32_t*>(texture_entries + entry_offset + 24);
        uint32_t data_offset = *reinterpret_cast<const uint32_t*>(texture_entries + entry_offset + 28);
        uint32_t data_size = *reinterpret_cast<const uint32_t*>(texture_entries + entry_offset + 32);

        // Find texture name from dict
        std::string name = "texture_" + std::to_string(i);
        for (const auto& [key, val] : texture_dict) {
            if (val == name_offset) {
                name = key;
                break;
            }
        }

        TextureInfo info;
        info.name = name;
        info.width = width;
        info.height = height;
        info.depth = depth;
        info.mip_levels = mip_levels;
        info.array_layers = array_layers;
        info.format = static_cast<TextureFormat>(format);
        info.data_offset = ftex_section.offset + data_offset;
        info.data_size = data_size;

        // Parse mip offsets if present
        if (mip_levels > 1 && entry_offset + 40 + mip_levels * 8 <= ftex_size) {
            for (uint32_t m = 0; m < mip_levels; ++m) {
                uint32_t mip_offset = *reinterpret_cast<const uint32_t*>(texture_entries + entry_offset + 40 + m * 8);
                uint32_t mip_size = *reinterpret_cast<const uint32_t*>(texture_entries + entry_offset + 40 + m * 8 + 4);
                info.mip_offsets.push_back(ftex_section.offset + mip_offset);
                info.mip_sizes.push_back(mip_size);
            }
        } else {
            info.mip_offsets.push_back(info.data_offset);
            info.mip_sizes.push_back(info.data_size);
        }

        textures_.push_back(std::move(info));
    }

    return true;
}

bool BfresFile::parse_bntx_section() {
    auto it = section_map_.find("BNTX");
    if (it == section_map_.end()) {
        std::fprintf(stderr, "=== NO BNTX SECTION IN MAP ===\n");
        // No BNTX section
        return true;
    }

    std::fprintf(stderr, "=== PARSE_BNTX_SECTION CALLED ===\n");

    const SectionEntry& bntx_section = it->second;
    if (bntx_section.offset + bntx_section.size > data_.size()) {
        error_ = "BNTX section extends beyond file";
        return false;
    }

    const uint8_t* bntx_data = &data_[bntx_section.offset];
    size_t bntx_size = bntx_section.size;

    // BNTX header
    if (bntx_size < 64) {
        error_ = "BNTX section too small";
        return false;
    }

    // Check magic
    uint32_t magic = *reinterpret_cast<const uint32_t*>(bntx_data);
    if (magic != 0x58544E42) {  // "BNTX" in little-endian
        error_ = "Invalid BNTX magic: 0x" + std::to_string(magic);
        return false;
    }

    std::fprintf(stderr, "BNTX at offset %zu, size %zu, magic=0x%08x\n",
                 bntx_section.offset, bntx_size, magic);

    // Find ALL BRTI sections and the _STR section
    struct BrtiInfo {
        uint32_t offset;
        uint32_t size;
    };
    std::vector<BrtiInfo> brti_sections;
    uint32_t str_offset = 0, str_size = 0;

    // Scan for BRTI and _STR sections (4-byte aligned)
    for (size_t offset = 64; offset + 4 <= bntx_size; offset += 4) {
        uint32_t sig = *reinterpret_cast<const uint32_t*>(bntx_data + offset);
        if (sig == 0x49545242) {  // "BRTI"
            uint32_t candidate_size = 0;
            if (offset + 8 <= bntx_size) {
                candidate_size = *reinterpret_cast<const uint32_t*>(bntx_data + offset + 4);
            }
            // Validate: size should be reasonable
            if (candidate_size >= 20 && offset + candidate_size <= bntx_size) {
                brti_sections.push_back({static_cast<uint32_t>(offset), candidate_size});
                std::fprintf(stderr, "[DEBUG] Found BRTI at offset %u, size %u\n", (unsigned)offset, candidate_size);
            }
        } else if (sig == 0x5254535F) {  // "_STR"
            if (offset + 8 <= bntx_size) {
                uint32_t sub_size = *reinterpret_cast<const uint32_t*>(bntx_data + offset + 4);
                if (sub_size >= 16 && offset + sub_size <= bntx_size) {
                    uint32_t str_magic = *reinterpret_cast<const uint32_t*>(bntx_data + offset);
                    if (str_magic == 0x5254535F) {  // "_STR"
                        str_offset = static_cast<uint32_t>(offset);
                        str_size = sub_size;
                        std::fprintf(stderr, "[DEBUG] Found _STR at offset %u, size %u\n", (unsigned)offset, sub_size);
                    }
                }
            }
        }
    }

    // Use the first _STR section for texture names
    std::vector<std::string> texture_names;
    if (str_offset > 0 && str_offset < bntx_size && str_size > 0) {
        const uint8_t* str_data = bntx_data + str_offset;
        parse_str_section(str_data, str_size, texture_names);
    }

    // Find texture data section - scan for ASTC magic in the BNTX section data
    // (need access to full file data)
    uint32_t texture_data_base = 0;
    for (size_t offset = 64; offset + 4 <= bntx_size; ++offset) {
        uint32_t sig = *reinterpret_cast<const uint32_t*>(bntx_data + offset);
        if ((sig & 0xFFFF) >= 0x93B0 && (sig & 0xFFFF) <= 0x93DD) {
            texture_data_base = bntx_section.offset + offset;
            std::fprintf(stderr, "[DEBUG] Found texture data at file offset %u (ASTC magic 0x%08x)\n", texture_data_base, sig);
            break;
        }
    }

    if (texture_data_base == 0) {
        std::fprintf(stderr, "[DEBUG] Warning: Could not find texture data section\n");
        // Fallback to after the first BRTI section
        if (!brti_sections.empty()) {
            texture_data_base = bntx_section.offset + brti_sections[0].offset + brti_sections[0].size;
        }
    } else {
        std::fprintf(stderr, "[DEBUG] Texture data section at file offset %u\n", texture_data_base);
    }

    // Parse ALL BRTI sections for texture info
    for (const auto& brti : brti_sections) {
        if (brti.offset < bntx_size && brti.size > 0) {
            parse_brti_section(bntx_data + brti.offset, brti.size, texture_names, texture_data_base);
        }
    }

    return true;
}

bool BfresFile::parse_str_section(const uint8_t* data, size_t size, std::vector<std::string>& names) {
    if (size < 16) return false;

    // _STR header (16 bytes):
    // offset 0: magic "_STR"
    // offset 4: section size
    // offset 8: unknown (often same as size)
    // offset 12: string count (often 0 in Nintendo files)

    uint32_t magic = *reinterpret_cast<const uint32_t*>(data);
    uint32_t str_size = *reinterpret_cast<const uint32_t*>(data + 4);
    uint32_t unk8 = *reinterpret_cast<const uint32_t*>(data + 8);
    uint32_t count = *reinterpret_cast<const uint32_t*>(data + 12);

    std::fprintf(stderr, "[DEBUG] _STR: magic=0x%08x, size=%u, unk8=%u, count=%u\n",
                 magic, str_size, unk8, count);

    if (magic != 0x5254535F) {  // "_STR" in little-endian
        return false;
    }

    // If count is 0 or unreasonable, parse strings sequentially
    if (count == 0 || count > 10000 || count * 4 > size - 16) {
        std::fprintf(stderr, "[DEBUG] _STR count %u is zero/unreasonable, parsing strings sequentially\n", count);
        // Parse strings sequentially from the end of the header
        size_t pos = 16;
        while (pos < size) {
            // Skip null bytes
            while (pos < size && data[pos] == 0) pos++;
            if (pos >= size) break;

            // Read string
            size_t start = pos;
            while (pos < size && data[pos] != 0) pos++;
            if (pos > start) {
                std::string name(reinterpret_cast<const char*>(&data[start]), pos - start);
                if (!name.empty()) {
                    std::fprintf(stderr, "[DEBUG]   String: %s\n", name.c_str());
                    names.push_back(name);
                }
            }
        }
        return true;
    }

    std::fprintf(stderr, "[DEBUG] _STR: magic=0x%08x, size=%u, unk8=%u, count=%u\n",
                 magic, str_size, unk8, count);

    if (magic != 0x5254535F) {  // "_STR" in little-endian
        return false;
    }

    // String offset table follows header (16 bytes)
    const uint32_t* offsets = reinterpret_cast<const uint32_t*>(data + 16);
    if (16 + count * 4 > size) return false;

    for (uint32_t i = 0; i < count; ++i) {
        uint32_t offset = offsets[i];
        if (offset < size) {
            std::string name = read_string(offset);
            if (!name.empty()) {
                std::fprintf(stderr, "[DEBUG]   String %u: %s\n", i, name.c_str());
                names.push_back(name);
            }
        }
    }

    return true;
}

bool BfresFile::parse_brti_section(const uint8_t* data, size_t size, const std::vector<std::string>& names, uint32_t texture_data_base) {
    if (size < 32) return false;

    // BRTI section format (observed from KFL files):
    // Standard section header (32 bytes):
    // offset 0: magic "BRTI" (0x49545242)
    // offset 4: size
    // offset 8: data_offset
    // offset 12: unknown (0)
    // offset 16: unknown (not count - unreliable)
    // offset 20: unknown
    // offset 24: unknown
    // offset 28: unknown
    // offset 32: entries start

    uint32_t magic = *reinterpret_cast<const uint32_t*>(data);
    if (magic != 0x49545242) {  // "BRTI"
        std::fprintf(stderr, "[DEBUG] BRTI: magic not at start (0x%08x)\n", magic);
        return false;
    }

    uint32_t brti_size = *reinterpret_cast<const uint32_t*>(data + 4);
    uint32_t data_offset = *reinterpret_cast<const uint32_t*>(data + 8);
    uint32_t unk12 = *reinterpret_cast<const uint32_t*>(data + 12);

    std::fprintf(stderr, "[DEBUG] BRTI: magic=0x%08x, size=%u, data_offset=%u\n",
                 magic, brti_size, data_offset);

    // The BRTI section size includes the 32-byte header
    // Each entry is 88 bytes. Calculate max possible entries.
    if (brti_size < 32) return false;
    uint32_t max_entries = (brti_size >= 32) ? (brti_size - 32) / 88 : 0;

    // Count from section size, clamped to available names
    uint32_t count = std::min<uint32_t>(max_entries, static_cast<uint32_t>(names.size()));

    // Validate count
    if (count == 0 || count > 10000) {
        std::fprintf(stderr, "[DEBUG] BRTI: invalid count %u\n", count);
        return false;
    }

    std::fprintf(stderr, "[DEBUG] BRTI: using count=%u\n", count);

    // Texture info entries start after 32-byte header
    const uint8_t* entries = data + 32;
    std::fprintf(stderr, "[DEBUG] entries ptr=%p, entries[0..7]=%02x %02x %02x %02x %02x %02x %02x %02x\n",
                 (void*)entries, entries[0], entries[1], entries[2], entries[3], entries[4], entries[5], entries[6], entries[7]);

    for (uint32_t i = 0; i < count && i < names.size(); ++i) {
        size_t entry_offset = i * 88;
        if (entry_offset + 88 > size - 32) break;

        const uint8_t* entry = entries + entry_offset;

        // Only process the first entry of each BRTI section (i=0)
        // Other entries appear to be padding
        if (i > 0) break;

        // BRTI texture entry (88 bytes):
        // uint32_t flags;
        // uint32_t width;
        // uint32_t height;
        // uint32_t depth;
        // uint32_t mip_levels;
        // uint32_t array_layers;
        // uint32_t format;
        // uint32_t dimension;      // 1=1D, 2=2D, 3=3D, 4=Cube, 5=CubeArray, 6=2DArray, 7=2DMS, 8=2DMSArray
        // uint32_t swizzle;
        // uint32_t tile_mode;
        // uint32_t alignment;
        // uint32_t pitch;
        // uint32_t unk1;
        // uint32_t unk2;
        // uint32_t unk3;
        // uint32_t unk4;
        // uint32_t data_offset;    // Offset to texture data (from BNTX data section)
        // uint32_t data_size;      // Size of texture data

        uint32_t flags = *reinterpret_cast<const uint32_t*>(entry);
        uint32_t width = *reinterpret_cast<const uint32_t*>(entry + 4);
        uint32_t height = *reinterpret_cast<const uint32_t*>(entry + 8);
        uint32_t depth = *reinterpret_cast<const uint32_t*>(entry + 12);
        uint32_t mip_levels = *reinterpret_cast<const uint32_t*>(entry + 16);
        uint32_t array_layers = *reinterpret_cast<const uint32_t*>(entry + 20);
        uint32_t format = *reinterpret_cast<const uint32_t*>(entry + 24);
        uint32_t dimension = *reinterpret_cast<const uint32_t*>(entry + 28);
        uint32_t swizzle = *reinterpret_cast<const uint32_t*>(entry + 32);
        uint32_t tile_mode = *reinterpret_cast<const uint32_t*>(entry + 36);
        uint32_t alignment = *reinterpret_cast<const uint32_t*>(entry + 40);
        uint32_t pitch = *reinterpret_cast<const uint32_t*>(entry + 44);
        uint32_t tex_data_offset = *reinterpret_cast<const uint32_t*>(entry + 64);  // data_offset at 64
        uint32_t tex_data_size = *reinterpret_cast<const uint32_t*>(entry + 76);   // data_size at 76

        // Map BNTX format to internal TextureFormat for size calculation
        TextureFormat internal_format = bntx_format_to_texture_format(format);

        // Calculate texture data size if not provided (size == 0)
        if (tex_data_size == 0 && width > 0 && height > 0) {
            // Calculate size based on format and dimensions
            size_t block_width = 4, block_height = 4, bytes_per_block = 16;

            // Determine block size from internal format
            switch (internal_format) {
                case TextureFormat::ASTC_4x4_UNORM:
                case TextureFormat::ASTC_4x4_SRGB:
                case TextureFormat::ASTC_5x4_UNORM:
                case TextureFormat::ASTC_5x4_SRGB:
                case TextureFormat::ASTC_5x5_UNORM:
                case TextureFormat::ASTC_5x5_SRGB:
                case TextureFormat::ASTC_6x5_UNORM:
                case TextureFormat::ASTC_6x5_SRGB:
                case TextureFormat::ASTC_6x6_UNORM:
                case TextureFormat::ASTC_6x6_SRGB:
                case TextureFormat::ASTC_8x5_UNORM:
                case TextureFormat::ASTC_8x5_SRGB:
                case TextureFormat::ASTC_8x6_UNORM:
                case TextureFormat::ASTC_8x6_SRGB:
                case TextureFormat::ASTC_8x8_UNORM:
                case TextureFormat::ASTC_8x8_SRGB:
                case TextureFormat::ASTC_10x5_UNORM:
                case TextureFormat::ASTC_10x5_SRGB:
                case TextureFormat::ASTC_10x6_UNORM:
                case TextureFormat::ASTC_10x6_SRGB:
                case TextureFormat::ASTC_10x8_UNORM:
                case TextureFormat::ASTC_10x8_SRGB:
                case TextureFormat::ASTC_10x10_UNORM:
                case TextureFormat::ASTC_10x10_SRGB:
                case TextureFormat::ASTC_12x10_UNORM:
                case TextureFormat::ASTC_12x10_SRGB:
                case TextureFormat::ASTC_12x12_UNORM:
                case TextureFormat::ASTC_12x12_SRGB:
                    // ASTC formats - extract block dimensions from format
                    switch (internal_format) {
                        case TextureFormat::ASTC_4x4_UNORM:
                        case TextureFormat::ASTC_4x4_SRGB:
                            block_width = 4; block_height = 4; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_5x4_UNORM:
                        case TextureFormat::ASTC_5x4_SRGB:
                            block_width = 5; block_height = 4; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_5x5_UNORM:
                        case TextureFormat::ASTC_5x5_SRGB:
                            block_width = 5; block_height = 5; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_6x5_UNORM:
                        case TextureFormat::ASTC_6x5_SRGB:
                            block_width = 6; block_height = 5; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_6x6_UNORM:
                        case TextureFormat::ASTC_6x6_SRGB:
                            block_width = 6; block_height = 6; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_8x5_UNORM:
                        case TextureFormat::ASTC_8x5_SRGB:
                            block_width = 8; block_height = 5; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_8x6_UNORM:
                        case TextureFormat::ASTC_8x6_SRGB:
                            block_width = 8; block_height = 6; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_8x8_UNORM:
                        case TextureFormat::ASTC_8x8_SRGB:
                            block_width = 8; block_height = 8; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_10x5_UNORM:
                        case TextureFormat::ASTC_10x5_SRGB:
                            block_width = 10; block_height = 5; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_10x6_UNORM:
                        case TextureFormat::ASTC_10x6_SRGB:
                            block_width = 10; block_height = 6; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_10x8_UNORM:
                        case TextureFormat::ASTC_10x8_SRGB:
                            block_width = 10; block_height = 8; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_10x10_UNORM:
                        case TextureFormat::ASTC_10x10_SRGB:
                            block_width = 10; block_height = 10; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_12x10_UNORM:
                        case TextureFormat::ASTC_12x10_SRGB:
                            block_width = 12; block_height = 10; bytes_per_block = 16; break;
                        case TextureFormat::ASTC_12x12_UNORM:
                        case TextureFormat::ASTC_12x12_SRGB:
                            block_width = 12; block_height = 12; bytes_per_block = 16; break;
                        default:
                            block_width = 4; block_height = 4; bytes_per_block = 16; break;
                    }
                    break;
                case TextureFormat::BC1_UNORM:
                case TextureFormat::BC2_UNORM:
                case TextureFormat::BC3_UNORM:
                    block_width = 4; block_height = 4; bytes_per_block = 16; break;
                case TextureFormat::BC4_UNORM:
                case TextureFormat::BC5_UNORM:
                    block_width = 4; block_height = 4; bytes_per_block = 8; break;
                case TextureFormat::BC6H_UFLOAT:
                case TextureFormat::BC6H_SFLOAT:
                case TextureFormat::BC7_UNORM:
                case TextureFormat::BC7_SRGB:
                    block_width = 4; block_height = 4; bytes_per_block = 16; break;
                default:
                    // Default to RGBA8 (4 bytes per pixel)
                    block_width = 1; block_height = 1; bytes_per_block = 4; break;
            }

            // Calculate size for each mip level
            uint32_t total_size = 0;
            uint32_t w = width, h = height;
            for (uint32_t m = 0; m < mip_levels; ++m) {
                uint32_t blocks_w = (w + block_width - 1) / block_width;
                uint32_t blocks_h = (h + block_height - 1) / block_height;
                total_size += blocks_w * blocks_h * bytes_per_block;
                w = std::max<uint32_t>(1, w / 2);
                h = std::max<uint32_t>(1, h / 2);
            }
            // Multiply by array layers
            total_size *= array_layers > 0 ? array_layers : 1;

            tex_data_size = static_cast<uint32_t>(total_size);
            std::fprintf(stderr, "[DEBUG]   Calculated texture size: %u bytes (w=%u h=%u mips=%u layers=%u fmt=0x%08x)\n",
                         tex_data_size, width, height, mip_levels, array_layers, format);
        }

        std::fprintf(stderr, "[DEBUG]   Texture %u: %s %ux%ux%u mips=%u layers=%u format=0x%08x dim=%u offset=%u size=%u\n",
                     i, names[i].c_str(), width, height, depth, mip_levels, array_layers, format, dimension, tex_data_offset, tex_data_size);

        BntxTextureInfo info;
        info.name = (i < names.size()) ? names[i] : "texture_" + std::to_string(i);
        info.width = width;
        info.height = height;
        info.depth = depth;
        info.mip_levels = mip_levels;
        info.array_layers = array_layers;
        info.format = static_cast<TextureFormat>(format);
        info.dimension = dimension;
        info.swizzle = swizzle;
        info.tile_mode = tile_mode;
        info.alignment = alignment;
        info.pitch = pitch;
        info.data_offset = texture_data_base + tex_data_offset;
        info.data_size = tex_data_size;
        info.mip_offsets.push_back(info.data_offset);
        info.mip_sizes.push_back(info.data_size);

        bntx_textures_.push_back(std::move(info));
    }

    return true;
}

std::optional<std::vector<uint8_t>> BfresFile::get_texture_data(size_t index) const {
    if (index >= textures_.size()) return std::nullopt;

    const TextureInfo& tex = textures_[index];
    if (tex.data_offset + tex.data_size > data_.size()) return std::nullopt;

    std::vector<uint8_t> result(tex.data_size);
    memcpy(result.data(), &data_[tex.data_offset], tex.data_size);
    return result;
}

std::optional<std::vector<uint8_t>> BfresFile::get_texture_data(const std::string& name) const {
    for (size_t i = 0; i < textures_.size(); ++i) {
        if (textures_[i].name == name) {
            return get_texture_data(i);
        }
    }
    return std::nullopt;
}

std::optional<std::vector<uint8_t>> BfresFile::get_bntx_texture_data(size_t index) const {
    if (index >= bntx_textures_.size()) return std::nullopt;

    const BntxTextureInfo& tex = bntx_textures_[index];
    if (tex.data_offset + tex.data_size > data_.size()) return std::nullopt;

    std::vector<uint8_t> result(tex.data_size);
    memcpy(result.data(), &data_[tex.data_offset], tex.data_size);
    return result;
}

std::optional<std::vector<uint8_t>> BfresFile::get_bntx_texture_data(const std::string& name) const {
    for (size_t i = 0; i < bntx_textures_.size(); ++i) {
        if (bntx_textures_[i].name == name) {
            return get_bntx_texture_data(i);
        }
    }
    return std::nullopt;
}

std::optional<std::span<const uint8_t>> BfresFile::get_section(const std::string& name) const {
    auto it = section_map_.find(name);
    if (it == section_map_.end()) return std::nullopt;

    const SectionEntry& section = it->second;
    if (section.offset + section.size > data_.size()) return std::nullopt;

    return std::span<const uint8_t>(&data_[section.offset], section.size);
}

} // namespace kirbyrecomp::bfres