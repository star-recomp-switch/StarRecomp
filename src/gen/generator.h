// KirbyRecomp Generator - Header
// Uses std::string instead of std::filesystem to avoid mingw64 linking issues

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace kirbyrecomp::gen {

struct GeneratorOptions {
    std::string exefs_path;
    std::string romfs_path;
    std::string keys_path;
    std::string output_dir;
    bool verbose = false;
};

struct NsoModule {
    std::string name;
    std::string path;
    bool is_main = false;
    // Track all translated functions for registration
    std::vector<std::pair<uint64_t, std::string>> functions; // {guest_address, function_name}
    // TODO: Add parsed NSO data
    // std::vector<uint8_t> text_section;
    // std::vector<uint8_t> rodata_section;
    // std::vector<uint8_t> data_section;
    // std::vector<Relocation> relocations;
};

class Generator {
public:
    explicit Generator(const GeneratorOptions& options);
    ~Generator() = default;

    bool Run();

private:
    bool ParseKeys();
    bool ExtractNsoModules();
    bool ProcessModule(const NsoModule& module);
    bool TranslateShaders();
    bool WriteOutput();
    bool WriteRegistrationHeader();

    GeneratorOptions options_;
    std::vector<NsoModule> modules_;
    bool keys_parsed_ = false;
};

} // namespace kirbyrecomp::gen