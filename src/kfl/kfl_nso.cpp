// KFL NSO Module Loader Implementation
// Handles KFL-specific NSO modules and ExeFS parsing

#include "kirbyrecomp/kfl/kfl_nso.h"

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <errno.h>

namespace kirbyrecomp::kfl {

// -----------------------------------------------------------------------------
// Path utilities (replacement for std::filesystem to avoid mingw64 linking issues)
// -----------------------------------------------------------------------------

static bool is_directory(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

static bool is_regular_file(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

static size_t file_size(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return static_cast<size_t>(st.st_size);
    }
    return 0;
}

static std::string path_filename(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

static std::string path_parent_path(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    return (pos == std::string::npos) ? "" : path.substr(0, pos);
}

static bool path_exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

static bool create_directories(const std::string& path) {
    // Simple implementation - create parent directories recursively
    std::string current;
    for (char c : path) {
        if (c == '/' || c == '\\') {
            if (!current.empty() && !path_exists(current)) {
                if (mkdir(current.c_str()) != 0 && errno != EEXIST) return false;
            }
        }
        current += c;
    }
    // Create the final path
    if (!path_exists(path)) {
        if (mkdir(path.c_str()) != 0 && errno != EEXIST) return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// KflModuleLoader Implementation
// -----------------------------------------------------------------------------

bool KflModuleLoader::load_exefs(const std::string& exefs_path) {
    last_error_.clear();

    // Check if it's a directory (directory-based ExeFS for testing) or a file
    if (is_directory(exefs_path)) {
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
    DIR* dir = opendir(exefs_dir.c_str());
    if (!dir) {
        last_error_ = "Failed to open directory: " + exefs_dir;
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // On Windows, d_type is not available, so use stat
        std::string full_path = exefs_dir + "/" + entry->d_name;
        if (!is_regular_file(full_path)) continue;

        std::string filename = entry->d_name;
        if (filename.size() < 4 || filename.substr(filename.size() - 4) != ".nso") {
            continue; // Skip non-NSO files
        }

        ExeFsModuleEntry mod_entry;
        mod_entry.name = filename;
        mod_entry.type = identify_module_type(filename);
        mod_entry.offset = 0; // Not applicable for directory mode
        mod_entry.size = file_size(full_path);
        mod_entry.decompressed_size = mod_entry.size;

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
            std::copy(hash, hash + 32, mod_entry.hash.begin());

            // Load the module
            auto module_opt = load_nso_from_entry(full_path);
            if (!module_opt) {
                last_error_ = "Failed to load module: " + mod_entry.name + " - " + nso_parser_.get_last_error();
                closedir(dir);
                return false;
            }

            auto& module = *module_opt;
            modules_[static_cast<size_t>(mod_entry.type)] = std::make_unique<kirbyrecomp::nso::NsoModule>(std::move(module));

            // Clear dependencies for directory mode
            mod_entry.dependencies.clear();

            entries_.push_back(mod_entry);
        }
    }

    closedir(dir);

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
    std::string exefs_dir = path_parent_path(exefs_path);
    for (const auto& entry : entries_) {
        auto module_opt = load_nso_from_entry(entry, exefs_dir);
        if (!module_opt) {
            last_error_ = "Failed to load module: " + entry.name + " - " + nso_parser_.get_last_error();
            return false;
        }

        auto& module = *module_opt;
        modules_[static_cast<size_t>(entry.type)] = std::make_unique<kirbyrecomp::nso::NsoModule>(std::move(module));
    }

    return true;
}

bool KflModuleLoader::parse_exefs_header(std::span<const uint8_t> data) {
    if (data.size() < sizeof(KflExeFsHeader)) {
        last_error_ = "ExeFS header too small";
        return false;
    }

    const KflExeFsHeader* header = reinterpret_cast<const KflExeFsHeader*>(data.data());

    // Check magic
    if (memcmp(header->magic, "ExeFS", 5) != 0) {
        last_error_ = "Invalid ExeFS magic";
        return false;
    }

    // For now, just validate basic structure
    entries_.clear();
    return true;
}

bool KflModuleLoader::parse_exefs_entries(std::span<const uint8_t> data) {
    // Simplified - in real implementation would parse entry table and hash table
    // For now, assume directory mode is used
    return true;
}

std::optional<kirbyrecomp::nso::NsoModule> KflModuleLoader::load_nso_from_entry(
    const ExeFsModuleEntry& entry,
    const std::string& base_path) {

    std::string nso_path;
    if (is_directory(base_path)) {
        nso_path = base_path + "/" + entry.name;
    } else {
        // For file-based ExeFS, would need to extract from container
        // For now, fallback to directory mode
        nso_path = entry.name;
    }

    auto module = nso_parser_.parse_file(nso_path.c_str());
    if (!module) {
        last_error_ = "Failed to parse NSO: " + nso_parser_.get_last_error();
        return std::nullopt;
    }

    return module;
}

// Overload for directory mode (load from full path)
std::optional<kirbyrecomp::nso::NsoModule> KflModuleLoader::load_nso_from_entry(const std::string& nso_path) {
    auto module = nso_parser_.parse_file(nso_path.c_str());
    if (!module) {
        last_error_ = "Failed to parse NSO: " + nso_parser_.get_last_error();
        return std::nullopt;
    }
    return module;
}

KflModuleType KflModuleLoader::identify_module_type(const std::string& filename) {
    // Simple heuristic based on filename
    if (filename.find("main") != std::string::npos) return KflModuleType::Main;
    if (filename.find("rtld") != std::string::npos) return KflModuleType::Rtld;
    if (filename.find("sdk") != std::string::npos) return KflModuleType::Sdk;
    if (filename.find("subsdk0") != std::string::npos) return KflModuleType::SubSdk0;
    if (filename.find("subsdk1") != std::string::npos) return KflModuleType::SubSdk1;
    if (filename.find("subsdk2") != std::string::npos) return KflModuleType::SubSdk2;
    if (filename.find("dynsys") != std::string::npos) return KflModuleType::DynSys;
    if (filename.find("dynaudio") != std::string::npos) return KflModuleType::DynAudio;
    if (filename.find("dyngfx") != std::string::npos) return KflModuleType::DynGfx;
    if (filename.find("dynnvn") != std::string::npos) return KflModuleType::DynNvn;
    return KflModuleType::Main;
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

} // namespace kirbyrecomp::kfl