// KirbyRecomp Generator Tool - Main Entry Point
// Parses user game files (ExeFS/RomFS + prod.keys) and generates native C code

#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <memory>

namespace fs = std::filesystem;

struct GeneratorOptions {
    fs::path exefs_path;
    fs::path romfs_path;
    fs::path keys_path;
    fs::path output_dir;
    bool verbose = false;
};

int PrintUsage(const char* argv0) {
    std::cerr << "Usage: " << argv0 << " [options]\n"
              << "Options:\n"
              << "  --exefs <path>    Path to ExeFS directory\n"
              << "  --romfs <path>    Path to RomFS directory\n"
              << "  --keys <path>     Path to prod.keys file\n"
              << "  --out <path>      Output directory for generated C code\n"
              << "  -v, --verbose     Verbose output\n"
              << "  -h, --help        Show this help\n";
    return 1;
}

GeneratorOptions ParseArgs(int argc, char* argv[]) {
    GeneratorOptions opts;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--exefs" && i + 1 < argc) {
            opts.exefs_path = argv[++i];
        } else if (arg == "--romfs" && i + 1 < argc) {
            opts.romfs_path = argv[++i];
        } else if (arg == "--keys" && i + 1 < argc) {
            opts.keys_path = argv[++i];
        } else if (arg == "--out" && i + 1 < argc) {
            opts.output_dir = argv[++i];
        } else if (arg == "-v" || arg == "--verbose") {
            opts.verbose = true;
        } else if (arg == "-h" || arg == "--help") {
            return PrintUsage(argv[0]);
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return PrintUsage(argv[0]);
        }
    }
    return opts;
}

bool ValidateInputs(const GeneratorOptions& opts) {
    if (opts.exefs_path.empty()) {
        std::cerr << "Error: --exefs is required\n";
        return false;
    }
    if (opts.romfs_path.empty()) {
        std::cerr << "Error: --romfs is required\n";
        return false;
    }
    if (opts.keys_path.empty()) {
        std::cerr << "Error: --keys is required\n";
        return false;
    }
    if (opts.output_dir.empty()) {
        std::cerr << "Error: --out is required\n";
        return false;
    }

    if (!fs::exists(opts.exefs_path)) {
        std::cerr << "Error: ExeFS path does not exist: " << opts.exefs_path << "\n";
        return false;
    }
    if (!fs::exists(opts.romfs_path)) {
        std::cerr << "Error: RomFS path does not exist: " << opts.romfs_path << "\n";
        return false;
    }
    if (!fs::exists(opts.keys_path)) {
        std::cerr << "Error: prod.keys path does not exist: " << opts.keys_path << "\n";
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    std::cout << "KirbyRecomp Generator v0.1.0\n";
    std::cout << "Static recompilation pipeline for Kirby and the Forgotten Land\n\n";

    auto opts = ParseArgs(argc, argv);

    if (!ValidateInputs(opts)) {
        return 1;
    }

    if (opts.verbose) {
        std::cout << "ExeFS: " << opts.exefs_path << "\n";
        std::cout << "RomFS: " << opts.romfs_path << "\n";
        std::cout << "Keys:  " << opts.keys_path << "\n";
        std::cout << "Out:   " << opts.output_dir << "\n\n";
    }

    // TODO: Implement actual generation pipeline
    // 1. Validate prod.keys
    // 2. Extract NSO modules from ExeFS
    // 3. Parse NSO headers and sections
    // 4. Disassemble AArch64 code to IR
    // 5. Generate C code from IR
    // 6. Translate shaders (Maxwell bytecode -> SPIR-V)
    // 7. Write output files

    std::cout << "[INFO] Generator pipeline not yet implemented\n";
    std::cout << "[INFO] This is Milestone 1 - foundation work in progress\n";

    return 0;
}