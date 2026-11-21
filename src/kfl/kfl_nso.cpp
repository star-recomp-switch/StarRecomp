// KFL NSO Module Loader Implementation
// Handles KFL-specific NSO modules and ExeFS parsing

#include "kirbyrecomp/kfl/kfl_nso.h"

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <openssl/sha.h>

namespace kirbyrecomp::kfl {

// -----------------------------------------------------------------------------
// KflModuleLoader Implementation
// -----------------------------------------------------------------------------

bool KflModuleLoader::load_exefs(const std::string& exefs_path) {
    last_error_.clear();

    // Check if it's a directory (directory-based ExeFS for testing) or a file
    std::filesystem::path path(exefs_path);
    if (std::filesystem::is_directory(path)) {
        // Directory-based ExeFS (test mode) - scan directory for .nso files
        return load_exefs_directory(exefs_path);
    } else {
        // File-based ExeFS (production) - parse ExeFS container file
        return load_exefs_file(exefs_path);
    }
}

bool KflModuleLoader::load_exefs_directory(const std::string& exefs_dir) {
    last_error_.clear();

    entries_.clear();

    // Scan directory for .nso files
    for (const auto& dir_entry : std::filesystem::directory_iterator(exefs_dir)) {
        if (!dir_entry.is_regular_file()) continue;

        std::string filename = dir_entry.path().filename().string();
        if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".nso") {
            continue; // Skip non-NSO files
        }

        std::string full_path = dir_entry.path().string();

        ExeFsModuleEntry entry;
        entry.name = dir_entry.path().filename().string();
        entry.type = identify_module_type(dir_entry.path().filename().string());
        entry.offset = 0; // Not applicable for directory mode
        entry.size = std::filesystem::file_size(dir_entry.path());
        entry.decompressed_size = entry.size;

        // Compute SHA-256 hash
        FILE* f = fopen(full_path.c_str(), "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fseek(f, 0, SEEK_SET);

            std::vector<uint8_t> buffer(size);
            fread(buffer.data(), 1, size, f);
            fclose(f);

            // Compute SHA-256
            unsigned char hash[32];
            SHA256(buffer.data(), buffer.size(), hash);
            std::copy(hash, hash + 32, entry.hash.begin());

            // Load the module
            auto module_opt = load_nso_from_entry(ExeFsModuleEntry{
                entry.name, entry.type, 0, entry.size, entry.decompressed_size, entry.hash, {}
            }, exefs_dir);

            if (!module_opt) {
                last_error_ = "Failed to load module: " + entry.name + " - " + nso_parser_.get_last_error();
                return false;
            }

            auto& module = *module_opt;
            modules_[static_cast<size_t>(entry.type)] = std::make_unique<kirbyrecomp::nso::NsoModule>(std::move(module));

            // Clear dependencies for directory mode
            entry.dependencies.clear();

            entries_.push_back(entry);
        }
    }

    if (entries_.empty()) {
        last_error_ = "No .nso files found in directory: " + exefs_dir;
        return false;
    }

    return true;
}

bool KflModuleLoader::load_exefs_file(const std::string& exefs_path) {
    last_error_.clear();

    // Read ExeFS file
    FILE* f = fopen(exefs_path.c_str(), "rb");
    if (!f) {
        last_error_ = "Failed to open ExeFS: " + exefs_path;
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        last_error_ = "ExeFS file empty or invalid";
        return false;
    }

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    size_t read = fread(buffer.data(), 1, buffer.size(), f);
    fclose(f);

    if (read != buffer.size()) {
        last_error_ = "Failed to read ExeFS file completely";
        return false;
    }

    // Parse ExeFS
    if (!parse_exefs_header(buffer)) {
        return false;
    }

    if (!parse_exefs_entries(buffer)) {
        return false;
    }

    // Load each module
    std::string exefs_dir = std::filesystem::path(exefs_path).parent_path().string();
    for (const auto& entry : entries_) {
        auto module_opt = load_nso_from_entry(entry, exefs_dir);
        if (!module_opt) {
            last_error_ = "Failed to load module: " + entry.name + " - " + nso_parser_.get_last_error();
            return false;
        }

        auto& module = *module_opt;
        modules_[static_cast<size_t>(identify_module_type(entry.name))] = std::make_unique<kirbyrecomp::nso::NsoModule>(std::move(module));
    }

    // Apply relocations for all loaded modules
    if (!apply_all_relocations()) {
        return false;
    }

    return true;
}

bool KflModuleLoader::parse_exefs_header(std::span<const uint8_t> buffer) {
    if (buffer.size() < sizeof(KflExeFsHeader)) {
        last_error_ = "ExeFS buffer too small for header";
        return false;
    }

    const KflExeFsHeader* header = reinterpret_cast<const KflExeFsHeader*>(buffer.data());

    if (std::memcmp(header->magic, "ExeFS", 4) != 0) {
        last_error_ = "Invalid ExeFS magic";
        return false;
    }

    if (header->version != 1) {
        last_error_ = "Unsupported ExeFS version: " + std::to_string(header->version);
        return false;
    }

    return true;
}

bool KflModuleLoader::parse_exefs_entries(std::span<const uint8_t> buffer) {
    if (buffer.size() < sizeof(KflExeFsHeader)) {
        last_error_ = "ExeFS buffer too small";
        return false;
    }

    const KflExeFsHeader* header = reinterpret_cast<const KflExeFsHeader*>(buffer.data());

    if (buffer.size() < header->header_size) {
        last_error_ = "ExeFS buffer too small for header";
        return false;
    }

    // Entries start after header
    const uint8_t* entries_data = buffer.data() + header->header_size;
    size_t entries_size = header->entry_count * sizeof(ExeFsEntry);

    if (buffer.size() < header->header_size + entries_size) {
        last_error_ = "ExeFS buffer too small for entries";
        return false;
    }

    entries_.clear();
    entries_.reserve(header->entry_count);

    for (uint32_t i = 0; i < header->entry_count; ++i) {
        const ExeFsEntry* entry = reinterpret_cast<const ExeFsEntry*>(entries_data + i * sizeof(ExeFsEntry));

        ExeFsModuleEntry module_entry;
        module_entry.name = std::string(entry->name, 8);
        // Trim null terminators
        size_t name_end = module_entry.name.find('\0');
        if (name_end != std::string::npos) {
            module_entry.name = module_entry.name.substr(0, name_end);
        }

        module_entry.type = identify_module_type(module_entry.name);
        module_entry.offset = entry->offset;
        module_entry.size = entry->size;
        module_entry.decompressed_size = entry->size; // Will be updated after decompression
        std::memcpy(module_entry.hash.data(), entry->hash, 32);

        // Determine dependencies based on module type
        switch (module_entry.type) {
            case KflModuleType::Main:
                module_entry.dependencies.push_back({KflModuleType::Rtld, "1.0"});
                module_entry.dependencies.push_back({KflModuleType::Sdk, "1.0"});
                break;
            case KflModuleType::SubSdk0:
            case KflModuleType::SubSdk1:
            case KflModuleType::SubSdk2:
                module_entry.dependencies.push_back({KflModuleType::Sdk, "1.0"});
                break;
            case KflModuleType::DynAudio:
                module_entry.dependencies.push_back({KflModuleType::Sdk, "1.0"});
                module_entry.dependencies.push_back({KflModuleType::SubSdk0, "1.0"});
                break;
            case KflModuleType::DynGfx:
            case KflModuleType::DynNvn:
                module_entry.dependencies.push_back({KflModuleType::Sdk, "1.0"});
                break;
            default:
                break;
        }

        entries_.push_back(std::move(module_entry));
    }

    return true;
}

std::optional<kirbyrecomp::nso::NsoModule> KflModuleLoader::load_module(KflModuleType type) {
    // Find entry for this module type
    auto it = std::find_if(entries_.begin(), entries_.end(),
                           [type](const ExeFsModuleEntry& e) { return e.type == type; });

    if (it == entries_.end()) {
        last_error_ = "Module type not found in ExeFS";
        return std::nullopt;
    }

    // Module should already be loaded if load_exefs was called
    size_t idx = static_cast<size_t>(type);
    if (modules_[idx]) {
        return *modules_[idx];
    }

    return std::nullopt;
}

const kirbyrecomp::nso::NsoModule* KflModuleLoader::get_module(KflModuleType type) const {
    size_t idx = static_cast<size_t>(type);
    if (idx < modules_.size() && modules_[idx]) {
        return modules_[idx].get();
    }
    return nullptr;
}

std::vector<kirbyrecomp::nso::NsoModule*> KflModuleLoader::get_modules() const {
    std::vector<kirbyrecomp::nso::NsoModule*> result;
    for (const auto& module : modules_) {
        if (module) {
            result.push_back(module.get());
        }
    }
    return result;
}


std::optional<kirbyrecomp::nso::NsoModule> KflModuleLoader::load_nso_from_entry(const ExeFsModuleEntry& entry, const std::string& exefs_dir) {
    // Read the NSO file from ExeFS directory
    std::string nso_path = exefs_dir + "/" + entry.name;

    // Try without directory prefix first
    FILE* f = fopen(entry.name.c_str(), "rb");
    if (!f) {
        f = fopen(nso_path.c_str(), "rb");
    }

    if (!f) {
        last_error_ = "Failed to open NSO file: " + entry.name;
        return std::nullopt;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        last_error_ = "NSO file empty: " + entry.name;
        return std::nullopt;
    }

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    size_t read = fread(buffer.data(), 1, buffer.size(), f);
    fclose(f);

    if (read != buffer.size()) {
        last_error_ = "Failed to read NSO file: " + entry.name;
        return std::nullopt;
    }

    auto module_opt = nso_parser_.parse_buffer(buffer);
    if (!module_opt) {
        last_error_ = "Failed to parse NSO: " + entry.name + " - " + nso_parser_.get_last_error();
        return std::nullopt;
    }

    return module_opt;
}

// Overload for directory mode (load from full path)
std::optional<kirbyrecomp::nso::NsoModule> KflModuleLoader::load_nso_from_entry(const std::string& nso_path) {
    FILE* f = fopen(nso_path.c_str(), "rb");
    if (!f) {
        last_error_ = "Failed to open NSO file: " + nso_path;
        return std::nullopt;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        last_error_ = "NSO file empty";
        return std::nullopt;
    }

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    size_t read = fread(buffer.data(), 1, buffer.size(), f);
    fclose(f);

    if (read != buffer.size()) {
        last_error_ = "Failed to read NSO file completely";
        return std::nullopt;
    }

    auto module_opt = nso_parser_.parse_buffer(buffer);
    if (!module_opt) {
        last_error_ = "Failed to parse NSO: " + nso_parser_.get_last_error();
        return std::nullopt;
    }

    return module_opt;
}

KflModuleType KflModuleLoader::identify_module_type(const std::string& filename) {
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower.find("main") == 0) return KflModuleType::Main;
    if (lower.find("rtld") == 0) return KflModuleType::Rtld;
    if (lower.find("sdk") == 0 && lower.find("subsdk") != 0) return KflModuleType::Sdk;
    if (lower.find("subsdk0") == 0) return KflModuleType::SubSdk0;
    if (lower.find("subsdk1") == 0) return KflModuleType::SubSdk1;
    if (lower.find("subsdk2") == 0) return KflModuleType::SubSdk2;
    if (lower.find("dynsys") == 0) return KflModuleType::DynSys;
    if (lower.find("dynaudio") == 0) return KflModuleType::DynAudio;
    if (lower.find("dyngfx") == 0) return KflModuleType::DynGfx;
    if (lower.find("dynnvn") == 0) return KflModuleType::DynNvn;

    return KflModuleType::Count;
}

// -----------------------------------------------------------------------------
// KflNvnCommandHandler Implementation
// -----------------------------------------------------------------------------

bool KflNvnCommandHandler::handle_command(uint32_t command_id, const void* in_data, size_t in_size,
                                          void* out_data, size_t out_size) {
    switch (static_cast<KflNvnCmdExt>(command_id)) {
        case KflNvnCmdExt::SetColorLut:
            return set_color_lut(in_data, in_size);

        case KflNvnCmdExt::SetColorLutSize:
            // Handle color LUT size
            return true;

        case KflNvnCmdExt::SetColorLutData:
            return set_color_lut(in_data, in_size);

        case KflNvnCmdExt::EnableColorLut:
            if (in_size >= 1) {
                bool enable = *static_cast<const uint8_t*>(in_data);
                return enable_color_lut(enable);
            }
            return false;

        case KflNvnCmdExt::SetDepthClampEnable:
            if (in_size >= 1) {
                bool enable = *static_cast<const uint8_t*>(in_data);
                float min = 0.0f, max = 1.0f;
                if (in_size >= 12) {
                    const float* params = static_cast<const float*>(in_data);
                    min = params[0];
                    max = params[1];
                }
                return set_depth_clamp(enable, min, max);
            }
            return false;

        case KflNvnCmdExt::SetDepthClampRange:
            if (in_size >= 8) {
                const float* params = static_cast<const float*>(in_data);
                return set_depth_clamp(depth_clamp_enabled_, params[0], params[1]);
            }
            return false;

        case KflNvnCmdExt::SetFramebufferSrgb:
            if (in_size >= 1) {
                bool enable = *static_cast<const uint8_t*>(in_data);
                return set_framebuffer_srgb(enable);
            }
            return false;

        default:
            return false;
    }
}

bool KflNvnCommandHandler::set_color_lut(const void* data, size_t size) {
    if (!data || size == 0) return false;

    color_lut_data_.assign(static_cast<const uint8_t*>(data),
                           static_cast<const uint8_t*>(data) + size);
    return true;
}

bool KflNvnCommandHandler::enable_color_lut(bool enable) {
    color_lut_enabled_ = enable;
    return true;
}

bool KflNvnCommandHandler::set_depth_clamp(bool enable, float min, float max) {
    depth_clamp_enabled_ = enable;
    depth_clamp_min_ = std::clamp(min, 0.0f, 1.0f);
    depth_clamp_max_ = std::clamp(max, 0.0f, 1.0f);
    return true;
}

bool KflNvnCommandHandler::set_framebuffer_srgb(bool enable) {
    framebuffer_srgb_ = enable;
    return true;
}

// -----------------------------------------------------------------------------
// Relocation Handling
// -----------------------------------------------------------------------------

bool KflModuleLoader::apply_all_relocations(uint64_t base_address) {
    // Standard Switch module base addresses
    static constexpr uint64_t MODULE_BASE_ADDRESSES[10] = {
        0x7100000000ULL,  // Main
        0x7110000000ULL,  // Rtld
        0x7120000000ULL,  // Sdk
        0x7130000000ULL,  // SubSdk0
        0x7140000000ULL,  // SubSdk1
        0x7150000000ULL,  // SubSdk2
        0x7160000000ULL,  // DynSys
        0x7170000000ULL,  // DynAudio
        0x7180000000ULL,  // DynGfx
        0x7190000000ULL,  // DynNvn
    };

    if (base_address == 0) {
        base_address = MODULE_BASE_ADDRESSES[0];
    }

    // Sort modules by their base address to ensure correct order
    struct ModuleInfo {
        KflModuleType type;
        uint64_t base_addr;
        const kirbyrecomp::nso::NsoModule* module;
    };

    std::vector<ModuleInfo> module_infos;
    for (size_t i = 0; i < modules_.size(); ++i) {
        if (modules_[i]) {
            ModuleInfo info;
            info.type = static_cast<KflModuleType>(i);
            info.base_addr = MODULE_BASE_ADDRESSES[i];
            info.module = modules_[i].get();
            module_infos.push_back(info);
        }
    }

    // Sort by base address
    std::sort(module_infos.begin(), module_infos.end(),
              [](const ModuleInfo& a, const ModuleInfo& b) {
                  return a.base_addr < b.base_addr;
              });

    // Apply relocations for each module
    for (const auto& info : module_infos) {
        if (!apply_relocations_for_module(info.module, info.base_addr)) {
            last_error_ = "Failed to apply relocations for module: " + std::to_string(static_cast<int>(info.type));
            return false;
        }
    }

    return true;
}

bool KflModuleLoader::apply_relocations_for_module(const kirbyrecomp::nso::NsoModule* module, uint64_t base_address) {
    if (!module) return true;

    // Apply relocations for each section
    for (const auto& section : module->sections) {
        if (!apply_relocations_for_section(module, section, base_address)) {
            return false;
        }
    }

    return true;
}

bool KflModuleLoader::apply_relocations_for_section(const kirbyrecomp::nso::NsoModule* module, const kirbyrecomp::nso::Section& section, uint64_t base_address) {
    for (const auto& reloc : section.relocations) {
        // Calculate the target address within the section
        // Section virtual_offset is relative to module base
        uint64_t section_base = base_address + section.virtual_offset;
        uint64_t target_addr = section_base + reloc.offset;

        switch (reloc.type) {
            case kirbyrecomp::nso::RelocType::Absolute64: {
                // 64-bit absolute address
                uint64_t* target = reinterpret_cast<uint64_t*>(target_addr);
                *target = base_address + reloc.addend;
                break;
            }
            case kirbyrecomp::nso::RelocType::PreL64: {
                // 64-bit PC-relative (PREL64)
                int64_t* target = reinterpret_cast<int64_t*>(target_addr);
                int64_t value = static_cast<int64_t>(base_address + reloc.addend - target_addr);
                *reinterpret_cast<int64_t*>(target_addr) = value;
                break;
            }
            case kirbyrecomp::nso::RelocType::Absolute32: {
                // 32-bit absolute
                uint32_t* target = reinterpret_cast<uint32_t*>(target_addr);
                *target = static_cast<uint32_t>(base_address + reloc.addend);
                break;
            }
            case kirbyrecomp::nso::RelocType::PreL31:
            case kirbyrecomp::nso::RelocType::AddAbs: {
                // 32-bit PC-relative (PREL31 / ADR_PREL_LO21)
                int32_t* target = reinterpret_cast<int32_t*>(target_addr);
                int64_t value = static_cast<int64_t>(base_address + reloc.addend - target_addr);
                *reinterpret_cast<int32_t*>(target_addr) = static_cast<int32_t>(value);
                break;
            }
            case kirbyrecomp::nso::RelocType::LdPreL19:
            case kirbyrecomp::nso::RelocType::PreL16: {
                // 16/19-bit PC-relative
                int32_t* target = reinterpret_cast<int32_t*>(target_addr);
                int64_t value = static_cast<int64_t>(base_address + reloc.addend - target_addr);
                // These are smaller bit-width relocations
                break;
            }
            case kirbyrecomp::nso::RelocType::MovW_Uabs_G0:
            case kirbyrecomp::nso::RelocType::MovW_Uabs_G0_NC:
            case kirbyrecomp::nso::RelocType::MovW_Uabs_G1:
            case kirbyrecomp::nso::RelocType::MovW_Uabs_G1_NC:
            case kirbyrecomp::nso::RelocType::MovW_Uabs_G2:
            case kirbyrecomp::nso::RelocType::MovW_Uabs_G2_NC:
            case kirbyrecomp::nso::RelocType::MovW_Uabs_G3:
            case kirbyrecomp::nso::RelocType::MovW_Sabs_G0:
            case kirbyrecomp::nso::RelocType::MovW_Sabs_G1:
            case kirbyrecomp::nso::RelocType::MovW_Sabs_G2: {
                // MOVW/MOVK relocations - handled by the compiler/linker typically
                // For static recompilation, these are usually resolved at link time
                break;
            }
            default:
                // Unknown relocation type - skip with warning
                // In practice, we should log a warning but continue
                break;
        }
    }

    return true;
}

} // namespace kirbyrecomp::kfl