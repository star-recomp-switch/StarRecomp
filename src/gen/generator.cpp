// KirbyRecomp Generator - Core Generation Logic
// Implements the pipeline: ExeFS/RomFS -> NSO -> IR -> C -> native

#include "generator.h"
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstring>

#include "kirbyrecomp/nso/nso.h"
#include "switchrecomp/translator/translator.h"
#include "switchrecomp/codegen/codegen.h"

namespace kirbyrecomp::gen {

Generator::Generator(const GeneratorOptions& options)
    : options_(options) {}

bool Generator::Run() {
    std::cout << "[Generator] Starting generation pipeline...\n";

    // Step 1: Validate and parse prod.keys
    if (!ParseKeys()) {
        std::cerr << "[Generator] Failed to parse prod.keys\n";
        return false;
    }

    // Step 2: Extract and parse NSO modules
    if (!ExtractNsoModules()) {
        std::cerr << "[Generator] Failed to extract NSO modules\n";
        return false;
    }

    // Step 3: Process each NSO module
    for (const auto& module : modules_) {
        if (!ProcessModule(module)) {
            std::cerr << "[Generator] Failed to process module: " << module.name << "\n";
            return false;
        }
    }

    // Step 4: Translate shaders
    if (!TranslateShaders()) {
        std::cerr << "[Generator] Failed to translate shaders\n";
        return false;
    }

    // Step 5: Write output
    if (!WriteOutput()) {
        std::cerr << "[Generator] Failed to write output\n";
        return false;
    }

    std::cout << "[Generator] Generation complete!\n";
    return true;
}

bool Generator::ParseKeys() {
    std::cout << "[Generator] Parsing prod.keys from " << options_.keys_path << "\n";
    // TODO: Implement prod.keys parsing
    // This would parse the key file and derive title keys for the game
    keys_parsed_ = true;
    return true;
}

bool Generator::ExtractNsoModules() {
    std::cout << "[Generator] Extracting NSO modules from ExeFS\n";

    // Look for main NSO and other modules using C-style directory iteration
    // Check for known module names
    const char* known_modules[] = {"main", "rtld", "sdk", "subsdk0", nullptr};
    for (int i = 0; known_modules[i]; ++i) {
        std::string full_path = options_.exefs_path + "/" + known_modules[i];
        FILE* f = fopen(full_path.c_str(), "rb");
        if (f) {
            fclose(f);
            NsoModule module;
            module.name = known_modules[i];
            module.path = full_path;
            module.is_main = (std::string(known_modules[i]) == "main");
            modules_.push_back(std::move(module));
            std::cout << "[Generator] Found NSO module: " << known_modules[i] << "\n";
        }
    }

    if (modules_.empty()) {
        std::cerr << "[Generator] No NSO modules found in ExeFS\n";
        return false;
    }

    return true;
}

bool Generator::ProcessModule(const NsoModule& module) {
    std::cout << "[Generator] Processing module: " << module.name << "\n";

    // Parse NSO file
    kirbyrecomp::nso::NsoParser parser;
    auto nso_result = parser.parse_file(module.path.c_str());
    if (!nso_result.has_value()) {
        std::cerr << "[Generator] Failed to parse NSO: " << parser.get_last_error() << "\n";
        return false;
    }

    const auto& nso = nso_result.value();
    const auto* text_section = nso.get_section(kirbyrecomp::nso::SectionType::Text);
    if (!text_section || text_section->data.empty()) {
        std::cerr << "[Generator] No .text section found in NSO module\n";
        return false;
    }

    std::cout << "[Generator] Found .text section: " << text_section->data.size() << " bytes\n";

    // Translate AArch64 code to IR using SwitchRecomp translator
    sr::translator::TranslateOptions translate_opts;
    translate_opts.base_address = kirbyrecomp::nso::DEFAULT_NSO_BASE_ADDRESS + text_section->virtual_offset;
    translate_opts.strict_decode = false;  // Emit Undefined for unimplemented opcodes

    // Use the SwitchRecomp translator (the one in vendor/switchrecomp)
    auto ir_module = sr::translator::Translate(
        sr::common::span<const uint8_t>(text_section->data.data(), text_section->data.size()),
        translate_opts
    );

    if (!ir_module) {
        std::cerr << "[Generator] IR translation failed\n";
        return false;
    }

    std::cout << "[Generator] Translated to IR: " << ir_module.value()->functions().size() << " functions\n";

    // Generate C code from IR using SwitchRecomp codegen
    sr::codegen::CodegenOptions codegen_opts;
    codegen_opts.module_name = module.name;
    codegen_opts.emit_comments = true;

    auto c_code = sr::codegen::GenerateC(*ir_module.value(), codegen_opts);
    if (!c_code) {
        std::cerr << "[Generator] C code generation failed\n";
        return false;
    }

    // Write C code to output directory using C-style operations
    std::string output_subdir = options_.output_dir + "/" + module.name;

    // Create directory using system command (Windows mkdir)
    std::string mkdir_cmd = "mkdir \"" + output_subdir + "\" 2>nul";
    system(mkdir_cmd.c_str());

    std::string c_file_path = output_subdir + "/" + module.name + ".c";

    std::ofstream out(c_file_path.c_str());
    if (!out) {
        std::cerr << "[Generator] Failed to open output file: " << c_file_path << "\n";
        return false;
    }
    out << c_code.value();
    out.close();

    std::cout << "[Generator] Generated C code: " << c_file_path << "\n";

    return true;
}

bool Generator::TranslateShaders() {
    std::cout << "[Generator] Translating shaders from RomFS\n";
    // TODO: Implement shader translation
    // 1. Find shader bytecode in RomFS/shader/
    // 2. Parse Maxwell bytecode
    // 3. Translate to SPIR-V using Ryujinx nxnative adaptation
    // 4. Generate pipeline cache
    return true;
}

bool Generator::WriteOutput() {
    std::cout << "[Generator] Writing output to " << options_.output_dir << "\n";
    // TODO: Implement output writing
    // 1. Create output directory structure
    // 2. Write per-function C files
    // 3. Write module header/source
    // 4. Write shader SPIR-V and pipeline cache
    // 5. Write provenance.toml
    return true;
}

} // namespace kirbyrecomp::gen