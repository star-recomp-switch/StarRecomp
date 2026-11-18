// KirbyRecomp Generator - Core Generation Logic
// Implements the pipeline: ExeFS/RomFS -> NSO -> IR -> C -> native

#include "generator.h"
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iomanip>

#include "kirbyrecomp/nso/nso.h"
#include "switchrecomp/translator/translator.h"
#include "switchrecomp/codegen/codegen.h"
#include "switchrecomp/ir/ir.h"

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

    // Step 5: Write registration header
    if (!WriteRegistrationHeader()) {
        std::cerr << "[Generator] Failed to write registration header\n";
        return false;
    }

    // Step 6: Write output
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

    // Store function addresses and names for registration
    NsoModule& mutable_module = const_cast<NsoModule&>(module);
    for (const sr::ir::Function* fn : ir_module.value()->functions()) {
        uint64_t guest_addr = fn->guest_address();
        std::string fn_name = "fn_";
        if (!module.name.empty()) {
            fn_name += module.name + "_";
        }
        // Convert guest address to hex string
        char addr_hex[32];
        snprintf(addr_hex, sizeof(addr_hex), "%llx", static_cast<unsigned long long>(guest_addr));
        fn_name += addr_hex;
        mutable_module.functions.emplace_back(guest_addr, fn_name);
    }

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

bool Generator::WriteRegistrationHeader() {
    std::cout << "[Generator] Writing registration header\n";

    std::string header_path = options_.output_dir + "/registration.h";
    std::ofstream out(header_path.c_str());
    if (!out) {
        std::cerr << "[Generator] Failed to open registration header: " << header_path << "\n";
        return false;
    }

    out << "// Auto-generated function registration header\n";
    out << "// Generated by KirbyRecomp Generator\n\n";
    out << "#pragma once\n\n";
    out << "#ifdef __cplusplus\n";
    out << "extern \"C\" {\n";
    out << "#endif\n\n";
    out << "void* sr_rt_guest_to_host(void* ctx, uint64_t addr, uint64_t size, int write);\n";
    out << "void  sr_rt_call(void* ctx, uint64_t target);\n";
    out << "void  sr_rt_handle_svc(void* ctx, uint32_t svc_id);\n";
    out << "void  sr_rt_register_function(uint64_t guest_addr, void(*fn)(void*));\n";
    out << "uint64_t sr_rt_load64(void* ctx, uint64_t addr);\n\n";
    out << "#ifdef __cplusplus\n";
    out << "}\n";
    out << "#endif\n\n";

    // Forward declare all functions
    for (const auto& module : modules_) {
        for (const auto& [addr, name] : module.functions) {
            out << "void " << name << "(void* ctx);\n";
        }
    }
    out << "\n";

    // Registration function
    out << "static inline void RegisterAllFunctions() {\n";
    for (const auto& module : modules_) {
        for (const auto& [addr, name] : module.functions) {
            out << "    sr_rt_register_function(0x" << std::hex << addr << std::dec << "ULL, " << name << ");\n";
        }
    }
    out << "}\n";

    out.close();
    std::cout << "[Generator] Registration header: " << header_path << "\n";
    return true;
}

bool Generator::WriteOutput() {
    std::cout << "[Generator] Writing output to " << options_.output_dir << "\n";

    // 1. Create output directory structure
    std::string mkdir_cmd = "mkdir \"" + options_.output_dir + "\" 2>nul";
    system(mkdir_cmd.c_str());

    std::string modules_dir = options_.output_dir + "/modules";
    std::string shaders_dir = options_.output_dir + "/shaders";
    system(("mkdir \"" + modules_dir + "\" 2>nul").c_str());
    system(("mkdir \"" + shaders_dir + "\" 2>nul").c_str());

    // 2. Write per-function C files (already done in ProcessModule)
    // The C files are already written in ProcessModule()

    // 3. Write module header/source (already done in ProcessModule)

    // 4. Write shader SPIR-V and pipeline cache
    if (!WriteShaders()) {
        std::cerr << "[Generator] Failed to write shaders\n";
        return false;
    }

    // 5. Write provenance.toml
    if (!WriteProvenance()) {
        std::cerr << "[Generator] Failed to write provenance.toml\n";
        return false;
    }

    std::cout << "[Generator] Output written to " << options_.output_dir << "\n";
    return true;
}

bool Generator::WriteShaders() {
    std::cout << "[Generator] Writing shader SPIR-V and pipeline cache\n";

    std::string shaders_dir = options_.output_dir + "/shaders";
    std::string mkdir_cmd = "mkdir \"" + std::string(options_.output_dir) + "/shaders\" 2>nul";
    system(mkdir_cmd.c_str());

    // Write shader SPIR-V binaries
    for (const auto& module : modules_) {
        // Check if module has shaders
        if (!module.shaders.empty()) {
            for (const auto& [shader_id, spirv] : module.shaders) {
                std::string shader_path = options_.output_dir + "/shaders/" + module.name + "_shader_" + std::to_string(shader_id) + ".spv";
                std::ofstream out(shader_path.c_str(), std::ios::binary);
                if (!out) {
                    std::cerr << "[Generator] Failed to open shader output: " << shader_path << "\n";
                    return false;
                }
                out.write(reinterpret_cast<const char*>(spirv.data()), spirv.size() * sizeof(uint32_t));
                out.close();
                std::cout << "[Generator] Wrote shader: " << shader_path << " (" << spirv.size() * 4 << " bytes)\n";
            }
        }
    }

    // Write pipeline cache
    if (!pipeline_cache_data_.empty()) {
        std::string cache_path = options_.output_dir + "/pipeline_cache.bin";
        std::ofstream out(cache_path.c_str(), std::ios::binary);
        if (!out) {
            std::cerr << "[Generator] Failed to open pipeline cache output: " << cache_path << "\n";
            return false;
        }
        out.write(reinterpret_cast<const char*>(pipeline_cache_data_.data()), pipeline_cache_data_.size());
        out.close();
        std::cout << "[Generator] Wrote pipeline cache: " << cache_path << " (" << pipeline_cache_data_.size() << " bytes)\n";
    }

    std::cout << "[Generator] Shaders written to " << shaders_dir << "\n";
    return true;
}

bool Generator::WriteProvenance() {
    std::cout << "[Generator] Writing provenance.toml\n";

    std::string provenance_path = options_.output_dir + "/provenance.toml";
    std::ofstream out(provenance_path.c_str());
    if (!out) {
        std::cerr << "[Generator] Failed to open provenance.toml\n";
        return false;
    }

    out << "# KirbyRecomp Provenance Information\n";
    out << "# Generated on: " << GetCurrentTimestamp() << "\n\n";

    out << "[build]\n";
    out << "generator_version = \"0.1.0\"\n";
    out << "switchrecomp_version = \"0.1.0\"\n";
    out << "build_timestamp = \"" << GetCurrentTimestamp() << "\"\n\n";

    out << "[game]\n";
    out << "title_id = \"0100B6C0115A4000\"\n";
    out << "title_name = \"Kirby and the Forgotten Land\"\n";
    out << "region = \"ALL\"\n\n";

    out << "[source]\n";
    out << "exefs_path = \"" << options_.exefs_path << "\"\n";
    out << "romfs_path = \"" << options_.romfs_path << "\"\n";
    out << "keys_path = \"" << options_.keys_path << "\"\n\n";

    out << "[modules]\n";
    for (const auto& module : modules_) {
        out << "[[modules]]\n";
        out << "name = \"" << module.name << "\"\n";
        out << "path = \"" << module.path << "\"\n";
        out << "is_main = " << (module.is_main ? "true" : "false") << "\n";
        out << "text_size = " << module.text_size << "\n";
        out << "rodata_size = " << module.rodata_size << "\n";
        out << "data_size = " << module.data_size << "\n";
        out << "function_count = " << module.functions.size() << "\n";
        out << "\n";
    }

    out << "[shaders]\n";
    out << "count = " << GetTotalShaderCount() << "\n";
    out << "pipeline_cache_size = " << pipeline_cache_data_.size() << "\n\n";

    out << "[legal]\n";
    out << "disclaimer = \"This file is generated for educational/research purposes. No Nintendo copyrighted code is included.\"\n";
    out << "user_must_own_game = true\n";

    out.close();
    std::cout << "[Generator] Provenance written to " << provenance_path << "\n";
    return true;
}

std::string Generator::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    std::tm tm_buf;
    std::tm* tm = std::localtime(&in_time_t);
    if (tm) {
        tm_buf = *tm;
        tm = &tm_buf;
    }
    char buffer[64];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
    return std::string(buffer);
}

size_t Generator::GetTotalShaderCount() const {
    size_t count = 0;
    for (const auto& module : modules_) {
        count += module.shaders.size();
    }
    return count;
}

} // namespace kirbyrecomp::gen