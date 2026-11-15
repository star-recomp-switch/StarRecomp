#pragma once

// KFL (Kirby and the Forgotten Land) NSO Module Loader
// Handles KFL-specific NSO modules and ExeFS parsing

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <span>
#include <unordered_map>
#include <memory>

#include "kirbyrecomp/nso/nso.h"

namespace kirbyrecomp::kfl {

// -----------------------------------------------------------------------------
// KFL Module Types
// -----------------------------------------------------------------------------

enum class KflModuleType : uint32_t {
    Main        = 0,  // Main executable (main.nso)
    Rtld        = 1,  // Runtime loader (rtld.nso)
    Sdk         = 2,  // SDK module (sdk.nso)
    SubSdk0     = 3,  // Sub SDK module 0 (subsdk0.nso)
    SubSdk1     = 4,  // Sub SDK module 1 (subsdk1.nso)
    SubSdk2     = 5,  // Sub SDK module 2 (subsdk2.nso)
    DynSys      = 6,  // Dynamic system module
    DynAudio    = 7,  // Dynamic audio module
    DynGfx      = 8,  // Dynamic graphics module
    DynNvn      = 9,  // Dynamic NVN module
    Count       = 10
};

// Module dependencies
struct ModuleDependency {
    KflModuleType module;
    std::string version;  // Minimum version required
};

// Module entry in ExeFS
struct ExeFsModuleEntry {
    std::string name;           // e.g., "main.nso", "sdk.nso"
    KflModuleType type;
    uint64_t offset;            // Offset in ExeFS
    uint64_t size;              // Compressed size
    uint64_t decompressed_size; // Decompressed size
    std::array<uint8_t, 32> hash;  // SHA-256
    std::vector<ModuleDependency> dependencies;
};

// KFL ExeFS header
#pragma pack(push, 1)
struct KflExeFsHeader {
    char magic[4];              // "ExeFS"
    uint32_t version;
    uint32_t header_size;
    uint32_t entry_count;
    uint64_t total_size;
    uint8_t reserved[24];
};
#pragma pack(pop)

// ExeFS entry
#pragma pack(push, 1)
struct ExeFsEntry {
    char name[8];               // 8-byte name (padded with nulls)
    uint64_t offset;
    uint64_t size;
    uint32_t hash[8];           // SHA-256 hash
    uint32_t flags;
    uint32_t alignment;
};
#pragma pack(pop)

// -----------------------------------------------------------------------------
// KFL Module Loader
// -----------------------------------------------------------------------------

class KflModuleLoader {
public:
    KflModuleLoader() = default;
    ~KflModuleLoader() = default;

    // Parse ExeFS and load all modules
    bool load_exefs(const std::string& exefs_path);

    // Load a specific module by type
    std::optional<kirbyrecomp::nso::NsoModule> load_module(KflModuleType type);

    // Get loaded module
    const kirbyrecomp::nso::NsoModule* get_module(KflModuleType type) const;

    // Get all loaded modules
    std::vector<kirbyrecomp::nso::NsoModule*> get_modules() const;

    // Apply relocations for all loaded modules
    bool apply_all_relocations(uint64_t base_address = 0x7100000000ULL);

    // Get ExeFS info
    const std::vector<ExeFsModuleEntry>& get_entries() const { return entries_; }

    // Get last error
    std::string get_last_error() const { return last_error_; }

    // Module base addresses (standard Switch addresses)
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

private:
    std::string last_error_;
    std::vector<ExeFsModuleEntry> entries_;
    std::array<std::unique_ptr<kirbyrecomp::nso::NsoModule>, 10> modules_;
    kirbyrecomp::nso::NsoParser nso_parser_;  // NSO parser instance

    // Parse ExeFS header
    bool parse_exefs_header(std::span<const uint8_t> buffer);

    // Parse ExeFS entries
    bool parse_exefs_entries(std::span<const uint8_t> buffer);

    // Load NSO module from ExeFS entry
    std::optional<kirbyrecomp::nso::NsoModule> load_nso_from_entry(const ExeFsModuleEntry& entry, const std::string& exefs_dir);

    // Identify module type from filename
    KflModuleType identify_module_type(const std::string& filename);
};

// -----------------------------------------------------------------------------
// KFL NVN Command Extensions
// -----------------------------------------------------------------------------

enum class KflNvnCmdExt : uint32_t {
    // KFL-specific NVN commands
    SetTextureFilter          = 0x1000,
    SetSamplerLodBias         = 0x1001,
    SetTextureSwizzle         = 0x1002,
    SetTextureBorderColor     = 0x1003,
    SetTextureMaxAnisotropy   = 0x1004,
    SetTextureCompareMode     = 0x1005,
    SetTextureCompareFunc     = 0x1006,
    SetTextureMaxLevel        = 0x1007,
    SetTextureMinLevel        = 0x1008,
    SetTextureBaseLevel       = 0x1009,

    // Color LUT commands
    SetColorLut               = 0x1100,
    SetColorLutSize           = 0x1101,
    SetColorLutData           = 0x1102,
    EnableColorLut            = 0x1103,

    // Depth clamp
    SetDepthClampEnable       = 0x1200,
    SetDepthClampRange        = 0x1201,

    // Framebuffer commands
    SetFramebufferSrgb        = 0x1300,
    SetFramebufferColorLut    = 0x1301,

    // Compute commands
    DispatchComputeIndirect   = 0x1400,
    SetComputeResource        = 0x1401,
};

// KFL NVN command handler
class KflNvnCommandHandler {
public:
    KflNvnCommandHandler() = default;
    ~KflNvnCommandHandler() = default;

    // Handle KFL-specific NVN command
    // Returns true if command was handled
    bool handle_command(uint32_t command_id, const void* in_data, size_t in_size,
                        void* out_data, size_t out_size);

    // Color LUT management
    bool set_color_lut(const void* data, size_t size);
    bool enable_color_lut(bool enable);

    // Depth clamp
    bool set_depth_clamp(bool enable, float min, float max);

    // Framebuffer SRGB
    bool set_framebuffer_srgb(bool enable);

private:
    bool color_lut_enabled_ = false;
    std::vector<uint8_t> color_lut_data_;
    bool depth_clamp_enabled_ = false;
    float depth_clamp_min_ = 0.0f;
    float depth_clamp_max_ = 1.0f;
    bool framebuffer_srgb_ = false;
};

} // namespace kirbyrecomp::kfl