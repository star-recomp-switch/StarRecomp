// KirbyRecomp Generator - Header

#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

namespace kirbyrecomp::gen {

struct GeneratorOptions {
    fs::path exefs_path;
    fs::path romfs_path;
    fs::path keys_path;
    fs::path output_dir;
    bool verbose = false;
};

struct NsoModule {
    std::string name;
    fs::path path;
    bool is_main = false;
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

    GeneratorOptions options_;
    std::vector<NsoModule> modules_;
    bool keys_parsed_ = false;
};

} // namespace kirbyrecomp::gen