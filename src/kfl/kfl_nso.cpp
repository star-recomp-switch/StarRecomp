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

} // namespace kirbyrecomp::kfl