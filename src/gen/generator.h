// KirbyRecomp Generator - Header
// Uses std::string instead of std::filesystem to avoid mingw64 linking issues

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <chrono>
#include <sstream>
#include <fstream>

namespace kirbyrecomp::gen {

struct GeneratorOptions {
    std::string exefs_path;
    std::string romfs_path;
    std::string keys_path;
    std::string output_dir;
    bool verbose = false;
};

struct ShaderInfo {
    std::vector<uint32_t> spirv;
    std::vector<uint32_t> reflection_data; // placeholder for reflection data
};

struct NsoModule {
    std::string name;
    std::string path;
    bool is_main = false;
    // Track all translated functions for registration
    std::vector<std::pair<uint64_t, std::string>> functions; // {guest_address, function_name}
    // Shader data
    std::unordered_map<uint64_t, std::vector<uint32_t>> shaders; // shader_id -> SPIR-V
    // Metadata for provenance
    size_t text_size = 0;
    size_t rodata_size = 0;
    size_t data_size = 0;
};

class Generator {
public:
    explicit Generator(const GeneratorOptions& options);
    ~Generator() = default;

    bool Run();

private:
    bool ParseKeys();
    bool ExtractNsoModules();
    bool LoadKflModules();
    bool ProcessModule(const NsoModule& module);
    bool TranslateShaders();
    bool TranslateNvnCommands();

    // Helper to recursively find .bfsha files
    static void FindBfshaFilesRecursive(const std::string& dir, std::vector<std::string>& out_files);
    bool WriteOutput();
    bool WriteShaders();
    bool WriteProvenance();
    bool WriteRegistrationHeader();

    std::string GetCurrentTimestamp();
    size_t GetTotalShaderCount() const;

    GeneratorOptions options_;
    std::vector<NsoModule> modules_;
    bool keys_parsed_ = false;

    // Key storage
    std::string keys_content_;
    std::unordered_map<std::string, std::string> keys_;

    // Shader and pipeline cache data
    std::vector<uint32_t> pipeline_cache_data_;
    std::unordered_map<uint64_t, std::vector<uint32_t>> shader_spirv_cache_;
};

} // namespace kirbyrecomp::gen