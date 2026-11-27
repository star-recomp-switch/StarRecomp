// KirbyRecomp Generator - Core Generation Logic
// Implements the pipeline: ExeFS/RomFS -> NSO -> IR -> C -> native

#include "generator.h"
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <dirent.h>
#include <sys/stat.h>

#include "kirbyrecomp/nso/nso.h"
#include "kirbyrecomp/shader/shader.h"
#include "kirbyrecomp/nvn/command_translator.h"
#include "kirbyrecomp/nvn/resource_manager.h"
#include "kirbyrecomp/kfl/kfl_nso.h"
#include "switchrecomp/translator/translator.h"
#include "switchrecomp/codegen/codegen.h"
#include "switchrecomp/ir/ir.h"
#include <vulkan/vulkan.h>

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

    // Step 2: Extract and parse NSO modules using KFL module loader
    if (!ExtractNsoModules()) {
        std::cerr << "[Generator] Failed to extract NSO modules\n";
        return false;
    }

    // Step 3 (Milestone 8): Load KFL-specific modules from ExeFS
    if (!LoadKflModules()) {
        std::cerr << "[Generator] Failed to load KFL modules\n";
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

    // Step 5: Translate NVN command buffers (Milestone 6)
    if (!TranslateNvnCommands()) {
        std::cerr << "[Generator] Failed to translate NVN commands\n";
        return false;
    }

    // Step 6: Write registration header
    if (!WriteRegistrationHeader()) {
        std::cerr << "[Generator] Failed to write registration header\n";
        return false;
    }

    // Step 7: Write output
    if (!WriteOutput()) {
        std::cerr << "[Generator] Failed to write output\n";
        return false;
    }

    std::cout << "[Generator] Generation complete!\n";
    return true;
}

bool Generator::ParseKeys() {
    std::cout << "[Generator] Parsing prod.keys from " << options_.keys_path << "\n";

    FILE* f = fopen(options_.keys_path.c_str(), "rb");
    if (!f) {
        std::cerr << "[Generator] Failed to open prod.keys\n";
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size < 0) {
        fclose(f);
        return false;
    }

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    size_t read = fread(buffer.data(), 1, buffer.size(), f);
    fclose(f);

    if (read != buffer.size()) {
        std::cerr << "[Generator] Failed to read prod.keys\n";
        return false;
    }

    // prod.keys is a text file with lines like:
    // key_name = hex_value
    // We'll parse it as a simple key-value store
    std::string content(buffer.begin(), buffer.end());

    // Store the raw content for provenance
    keys_content_ = content;

    // Parse key-value pairs
    size_t pos = 0;
    while (pos < content.size()) {
        // Skip whitespace
        while (pos < content.size() && isspace(content[pos])) pos++;

        if (pos >= content.size()) break;

        // Check for comment
        if (content[pos] == '#') {
            // Skip to end of line
            while (pos < content.size() && content[pos] != '\n') pos++;
            continue;
        }

        // Find key
        size_t key_start = pos;
        while (pos < content.size() && !isspace(content[pos]) && content[pos] != '=') pos++;
        if (pos >= content.size()) break;

        std::string key = content.substr(key_start, pos - key_start);

        // Skip whitespace
        while (pos < content.size() && isspace(content[pos])) pos++;
        if (pos >= content.size() || content[pos] != '=') continue;

        pos++; // skip '='

        // Skip whitespace
        while (pos < content.size() && isspace(content[pos])) pos++;
        if (pos >= content.size()) break;

        // Find value (until newline)
        size_t val_start = pos;
        while (pos < content.size() && content[pos] != '\n' && content[pos] != '\r') pos++;
        std::string value = content.substr(val_start, pos - val_start);

        // Trim trailing whitespace from value
        while (!value.empty() && isspace(value.back())) value.pop_back();

        // Store in map
        if (!key.empty()) {
            keys_[key] = value;
        }
    }

    std::cout << "[Generator] Parsed " << keys_.size() << " keys from prod.keys\n";

    // Verify required keys exist
    const std::vector<std::string> required_keys = {
        "master_key_00",
        "master_key_01",
        "master_key_02",
        "master_key_03",
        "master_key_04",
        "master_key_05",
        "master_key_06",
        "master_key_07",
        "master_key_08",
        "master_key_09",
        "master_key_0A",
        "master_key_0B",
        "master_key_0C",
        "master_key_0D",
        "master_key_0E",
        "master_key_0F",
        "titlekek_source",
        "header_key_source"
    };

    for (const auto& req : required_keys) {
        if (keys_.find(req) == keys_.end()) {
            std::cerr << "[Generator] Warning: Required key not found: " << req << "\n";
        }
    }

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

bool Generator::LoadKflModules() {
    std::cout << "[Generator] Loading KFL modules from ExeFS (Milestone 8)\n";

    // Check if ExeFS is a directory (our test setup) or a file
    // For now, skip KFL loading if the path is a directory (our test setup)
    // The KFL loader expects a single ExeFS file, not a directory
    std::string exefs_path = options_.exefs_path;
    FILE* f = fopen(exefs_path.c_str(), "rb");
    if (f) {
        fclose(f);
        // It's a file - try KFL loading
        kirbyrecomp::kfl::KflModuleLoader kfl_loader;
        if (!kfl_loader.load_exefs(options_.exefs_path)) {
            std::cerr << "[Generator] Failed to load KFL ExeFS: " << kfl_loader.get_last_error() << "\n";
            return false;
        }

        // Get all loaded modules
        auto kfl_modules = kfl_loader.get_modules();
        if (!kfl_modules.empty()) {
            std::cout << "[Generator] Loaded " << kfl_modules.size() << " KFL modules\n";
            // TODO: Add modules to modules_ list
        }
    } else {
        // It's a directory (our test setup) - KFL modules would be individual NSO files
        std::cout << "[Generator] ExeFS is a directory, skipping KFL ExeFS parser (modules already loaded via ExtractNsoModules)\n";
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
    std::cerr << "[Generator] Calling translator with " << text_section->data.size() << " bytes\n";
    std::cerr.flush();
    auto ir_module = sr::translator::Translate(
        sr::common::span<const uint8_t>(text_section->data.data(), text_section->data.size()),
        translate_opts
    );
    std::cerr << "[Generator] Translator returned\n";
    std::cerr.flush();

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

    // Look for shader files in RomFS/shader/ recursively
    std::string shader_dir = options_.romfs_path + "/shader";

    // Recursively find .bfsha files
    std::vector<std::string> shader_files;
    FindBfshaFilesRecursive(shader_dir, shader_files);

    if (shader_files.empty()) {
        std::cout << "[Generator] No shader files found in " << shader_dir << "\n";
        return true;  // Not an error, just no shaders
    }

    std::cout << "[Generator] Found " << shader_files.size() << " shader file(s)\n";

    // Initialize shader translator
    kirbyrecomp::shader::ShaderTranslator translator;

    for (size_t i = 0; i < shader_files.size(); ++i) {
        const std::string& shader_path = shader_files[i];
        std::cout << "[Generator] Processing shader: " << shader_path << "\n";

        // Read shader file
        FILE* f = fopen(shader_path.c_str(), "rb");
        if (!f) {
            std::cerr << "[Generator] Failed to open shader: " << shader_path << "\n";
            continue;
        }

        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (size < 0) {
            fclose(f);
            continue;
        }

        std::vector<uint8_t> shader_data(static_cast<size_t>(size));
        size_t read = fread(shader_data.data(), 1, shader_data.size(), f);
        fclose(f);

        if (read != shader_data.size()) {
            std::cerr << "[Generator] Failed to read shader: " << shader_path << "\n";
            continue;
        }

        // Try to parse as NVN shader container
        auto parsed = translator.parse_shader_container(shader_data);
        if (!parsed) {
            // Try parsing as raw Maxwell bytecode (assume fragment shader as default)
            std::cout << "[Generator] NVN container parse failed, trying raw Maxwell bytecode\n";
            parsed = translator.parse_maxwell_bytecode(shader_data, kirbyrecomp::shader::NvnShaderStage::Fragment);
        }

        if (!parsed) {
            std::cerr << "[Generator] Failed to parse shader: " << shader_path << " - " << translator.get_last_error() << "\n";
            continue;
        }

        // Translate to SPIR-V
        kirbyrecomp::shader::ShaderTranslationOptions trans_opts;
        trans_opts.optimize = true;
        trans_opts.validate_spirv = true;
        trans_opts.target_stage = static_cast<kirbyrecomp::shader::NvnShaderStage>(parsed->header.shader_stage);

        auto spirv = translator.translate_to_spirv(*parsed, trans_opts);
        if (!spirv) {
            std::cerr << "[Generator] SPIR-V translation failed: " << translator.get_last_error() << "\n";
            continue;
        }

        // Store SPIR-V in cache
        uint64_t shader_id = static_cast<uint64_t>(i);
        shader_spirv_cache_[shader_id] = spirv->words;

        // Also store in first module (for now, all shaders go to first module)
        if (!modules_.empty()) {
            modules_[0].shaders[shader_id] = spirv->words;
        }

        std::cout << "[Generator] Translated shader " << i << " to SPIR-V (" << spirv->words.size() << " words)\n";
    }

    // Generate simple pipeline cache (placeholder)
    if (!shader_spirv_cache_.empty()) {
        pipeline_cache_data_ = {0x01, 0x00, 0x00, 0x00};  // Version 1
        pipeline_cache_data_.insert(pipeline_cache_data_.end(),
                                   static_cast<uint32_t>(shader_spirv_cache_.size()));
    }

    return true;
}

bool Generator::TranslateNvnCommands() {
    std::cout << "[Generator] Translating NVN command buffers from RomFS\n";

    // Look for NVN command buffer files in RomFS/nvn/
    std::string nvn_dir = options_.romfs_path + "/nvn";
    std::string find_cmd = "dir /b \"" + nvn_dir + "\" 2>nul";
    FILE* pipe = _popen(find_cmd.c_str(), "r");
    if (!pipe) {
        std::cout << "[Generator] No NVN directory found at " << nvn_dir << "\n";
        return true;  // Not an error, just no NVN commands
    }

    char buffer[1024];
    std::vector<std::string> nvn_files;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        line.erase(line.find_last_not_of("\r\n") + 1);
        if (!line.empty()) {
            nvn_files.push_back(nvn_dir + "/" + line);
        }
    }
    _pclose(pipe);

    if (nvn_files.empty()) {
        std::cout << "[Generator] No NVN command files found in " << nvn_dir << "\n";
        return true;
    }

    std::cout << "[Generator] Found " << nvn_files.size() << " NVN command file(s)\n";

    for (size_t i = 0; i < nvn_files.size(); ++i) {
        const std::string& nvn_path = nvn_files[i];
        std::cout << "[Generator] Processing NVN commands: " << nvn_path << "\n";

        // Read NVN command file
        FILE* f = fopen(nvn_path.c_str(), "rb");
        if (!f) {
            std::cerr << "[Generator] Failed to open NVN file: " << nvn_path << "\n";
            continue;
        }

        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (size < 0) {
            fclose(f);
            continue;
        }

        std::vector<uint8_t> nvn_data(static_cast<size_t>(size));
        size_t read = fread(nvn_data.data(), 1, nvn_data.size(), f);
        fclose(f);

        if (read != nvn_data.size()) {
            std::cerr << "[Generator] Failed to read NVN file: " << nvn_path << "\n";
            continue;
        }

        // Parse NVN command buffer using the command buffer parser
        kirbyrecomp::nvn::CommandBufferParser parser;
        auto parsed = parser.parse(nvn_data);
        if (!parsed) {
            std::cerr << "[Generator] Failed to parse NVN commands: " << parser.get_last_error() << "\n";
            continue;
        }

        // Generate NVN command replay C++ file using the translator
        std::string replay_path = options_.output_dir + "/nvn_replay_" + std::to_string(i) + ".cpp";
        std::ofstream out(replay_path.c_str());
        if (!out) {
            std::cerr << "[Generator] Failed to open NVN replay output: " << replay_path << "\n";
            continue;
        }

        out << "// Auto-generated NVN command replay\n";
        out << "// Generated by KirbyRecomp Generator\n\n";
        out << "#include <vulkan/vulkan.h>\n";
        out << "#include <vector>\n";
        out << "#include \"kirbyrecomp/nvn/command_translator.h\"\n";
        out << "#include \"kirbyrecomp/nvn/resource_manager.h\"\n\n";
        out << "void replay_nvn_commands_" << i << "(VkCommandBuffer cmd_buffer, kirbyrecomp::nvn::ResourceManager& resource_manager) {\n";
        out << "    // NVN command buffer replay for " << nvn_path << "\n";
        out << "    // Size: " << nvn_data.size() << " bytes\n";
        out << "    // Parsed " << parsed->size() << " commands\n\n";

        // Use the command buffer translator to generate Vulkan commands
        out << "    kirbyrecomp::nvn::CommandBufferTranslator translator(resource_manager.get_device(), cmd_buffer, resource_manager);\n";
        out << "    if (!translator.begin_command_buffer()) {\n";
        out << "        return;\n";
        out << "    }\n\n";

        // Generate translation for each parsed command
        for (const auto& cmd : *parsed) {
            // Map opcode to translator method
            switch (cmd.opcode) {
                case kirbyrecomp::nvn::MaxwellCmdOpcode::SET_VIEWPORT: {
                    out << "    {\n";
                    out << "        const auto* vp = reinterpret_cast<const kirbyrecomp::nvn::CmdSetViewport*>(cmd.data.data());\n";
                    out << "        VkViewport viewport{};\n";
                    out << "        viewport.x = vp->x;\n";
                    out << "        viewport.y = vp->y;\n";
                    out << "        viewport.width = vp->width;\n";
                    out << "        viewport.height = vp->height;\n";
                    out << "        viewport.minDepth = vp->min_depth;\n";
                    out << "        viewport.maxDepth = vp->max_depth;\n";
                    out << "        vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);\n";
                    out << "    }\n";
                    break;
                }
                case kirbyrecomp::nvn::MaxwellCmdOpcode::SET_SCISSOR: {
                    out << "    {\n";
                    out << "        const auto* sc = reinterpret_cast<const kirbyrecomp::nvn::CmdSetScissor*>(cmd.data.data());\n";
                    out << "        VkRect2D rect{};\n";
                    out << "        rect.offset.x = sc->x;\n";
                    out << "        rect.offset.y = sc->y;\n";
                    out << "        rect.extent.width = sc->width;\n";
                    out << "        rect.extent.height = sc->height;\n";
                    out << "        vkCmdSetScissor(cmd_buffer, 0, 1, &rect);\n";
                    out << "    }\n";
                    break;
                }
                case kirbyrecomp::nvn::MaxwellCmdOpcode::DRAW_ARRAYS: {
                    out << "    {\n";
                    out << "        const auto* draw = reinterpret_cast<const kirbyrecomp::nvn::CmdDrawArrays*>(cmd.data.data());\n";
                    out << "        vkCmdDraw(cmd_buffer, draw->vertex_count, draw->instance_count, draw->first_vertex, draw->first_instance);\n";
                    out << "    }\n";
                    break;
                }
                default:
                    out << "    // Unhandled opcode: " << static_cast<uint32_t>(cmd.opcode) << "\n";
                    break;
            }
        }

        out << "\n    translator.end_command_buffer();\n";
        out << "}\n\n";
        out.close();

        std::cout << "[Generator] Generated NVN replay: " << replay_path << " (" << parsed->size() << " commands parsed)\n";
    }

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
    out << "#include \"switchrecomp/runtime/runtime.h\"\n";
    out << "using sr::runtime::GuestContext;\n";
    out << "extern \"C\" {\n";
    out << "#else\n";
    out << "#include \"switchrecomp/runtime/vreg.h\"\n";
    out << "typedef struct GuestContext {\n";
    out << "    uint64_t x[31];\n";
    out << "    uint64_t sp;\n";
    out << "    uint64_t pc;\n";
    out << "    uint32_t nzcv;\n";
    out << "    sr_vreg v[32];\n";
    out << "    uint32_t fpcr;\n";
    out << "    uint32_t fpsr;\n";
    out << "    uint64_t shared_counter;\n";
    out << "    uint64_t thread_handle;\n";
    out << "} GuestContext;\n";
    out << "extern \"C\" {\n";
    out << "#endif\n\n";
    // Match the declarations in entry.c / runtime.h
    out << "void* sr_rt_guest_to_host(GuestContext* ctx, uint64_t addr, uint64_t size, int write);\n";
    out << "void  sr_rt_call(GuestContext* ctx, uint64_t target);\n";
    out << "void  sr_rt_handle_svc(GuestContext* ctx, uint32_t svc_id);\n";
    out << "void  sr_rt_register_function(uint64_t guest_addr, void(*fn)(GuestContext*));\n";
    out << "uint64_t sr_rt_load64(GuestContext* ctx, uint64_t addr);\n\n";
    out << "#ifdef __cplusplus\n";
    out << "}\n";
    out << "#endif\n\n";

    // Forward declare all functions using GuestContext* (matches generated C code)
    for (const auto& module : modules_) {
        for (const auto& [addr, name] : module.functions) {
            out << "void " << name << "(GuestContext* ctx);\n";
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

    // Write main entry point
    if (!WriteMainEntryPoint()) {
        return false;
    }

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

// Generate a main entry point C file that registers all functions and provides main()
bool Generator::WriteMainEntryPoint() {
    std::cout << "[Generator] Writing main entry point\n";

    std::string main_path = options_.output_dir + "/main.c";
    std::ofstream out(main_path.c_str());
    if (!out) {
        std::cerr << "[Generator] Failed to open main.c\n";
        return false;
    }

    out << "// KirbyRecomp Main Entry Point\n";
    out << "// Auto-generated by KirbyRecomp Generator\n\n";
    out << "#include <stdint.h>\n";
    out << "#include <stdio.h>\n\n";

#ifdef __cplusplus
    out << "#include \"switchrecomp/runtime/runtime.h\"\n";
    out << "using sr::runtime::GuestContext;\n";
#else
    out << "#include \"switchrecomp/runtime/vreg.h\"\n";
    out << "typedef struct GuestContext {\n";
    out << "    uint64_t x[31];\n";
    out << "    uint64_t sp;\n";
    out << "    uint64_t pc;\n";
    out << "    uint32_t nzcv;\n";
    out << "    sr_vreg v[32];\n";
    out << "    uint32_t fpcr;\n";
    out << "    uint32_t fpsr;\n";
    out << "    uint64_t shared_counter;\n";
    out << "    uint64_t thread_handle;\n";
    out << "} GuestContext;\n";
#endif

    out << "#ifdef __cplusplus\n";
    out << "extern \"C\" {\n";
    out << "#endif\n";
    out << "void* sr_rt_guest_to_host(GuestContext* ctx, uint64_t addr, uint64_t size, int write);\n";
    out << "void  sr_rt_call(GuestContext* ctx, uint64_t target);\n";
    out << "void  sr_rt_handle_svc(GuestContext* ctx, uint32_t svc_id);\n";
    out << "void  sr_rt_register_function(uint64_t guest_addr, void(*fn)(GuestContext*));\n";
    out << "#ifdef __cplusplus\n";
    out << "}\n";
    out << "#endif\n\n";

    // Forward declare all functions
    for (const auto& module : modules_) {
        for (const auto& [addr, name] : module.functions) {
            out << "void " << name << "(GuestContext* ctx);\n";
        }
    }
    out << "\n";

    // Registration function
    out << "static void RegisterAllFunctions() {\n";
    for (const auto& module : modules_) {
        for (const auto& [addr, name] : module.functions) {
            out << "    sr_rt_register_function(0x" << std::hex << addr << std::dec << "ULL, " << name << ");\n";
        }
    }
    out << "}\n\n";

    // Main entry point
    out << "int main(int argc, char** argv) {\n";
    out << "    (void)argc; (void)argv;\n\n";
    out << "    printf(\"KirbyRecomp - Starting...\\n\");\n\n";
    out << "    // Register all translated functions\n";
    out << "    RegisterAllFunctions();\n\n";
    out << "    printf(\"All functions registered.\\n\");\n\n";
    out << "    // Create a guest context\n";
    out << "    GuestContext ctx = {0};\n";
    out << "    ctx.pc = 0x15729FFED30ULL; // Main module entry point\n\n";
    out << "    // Call the main module entry function\n";
    out << "    // In a real implementation, this would be the actual entry point\n";
    out << "    printf(\"KirbyRecomp execution would start here.\\n\");\n\n";
    out << "    return 0;\n";
    out << "}\n";

    out.close();
    std::cout << "[Generator] Wrote main entry point: " << options_.output_dir + "/main.c" << "\n";
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

void Generator::FindBfshaFilesRecursive(const std::string& dir, std::vector<std::string>& out_files) {
    DIR* d = opendir(dir.c_str());
    if (!d) return;

    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        std::string full_path = dir + "/" + entry->d_name;

        // Check if it's a directory
        struct stat st;
        if (stat(full_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            // Recurse into subdirectory
            FindBfshaFilesRecursive(full_path, out_files);
        } else {
            // Check if it's a .bfsha file
            std::string filename = entry->d_name;
            if (filename.size() >= 6 && filename.substr(filename.size() - 6) == ".bfsha") {
                out_files.push_back(full_path);
            }
        }
    }
    closedir(d);
}

} // namespace kirbyrecomp::gen