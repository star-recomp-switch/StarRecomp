// KirbyRecomp Generator Tool - Main Entry Point
// Parses user game files (ExeFS/RomFS + prod.keys) and generates native C code
// Avoids std::filesystem due to mingw64 linking issues

#include "generator.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace gen = kirbyrecomp::gen;

// Simple file existence check using C stdio
static bool file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (f) {
        fclose(f);
        return true;
    }
    // Also check if it's a directory
    #ifdef _WIN32
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(path, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        FindClose(h);
        return true;
    }
    #else
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    #endif
    return false;
}

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

bool ParseArgs(int argc, char* argv[], gen::GeneratorOptions& opts) {
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
            PrintUsage(argv[0]);
            return false;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            PrintUsage(argv[0]);
            return false;
        }
    }
    return true;
}

bool ValidateInputs(const gen::GeneratorOptions& opts) {
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

    if (!file_exists(opts.exefs_path.c_str())) {
        std::cerr << "Error: ExeFS path does not exist: " << opts.exefs_path << "\n";
        return false;
    }
    if (!file_exists(opts.romfs_path.c_str())) {
        std::cerr << "Error: RomFS path does not exist: " << opts.romfs_path << "\n";
        return false;
    }
    if (!file_exists(opts.keys_path.c_str())) {
        std::cerr << "Error: prod.keys path does not exist: " << opts.keys_path << "\n";
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    std::cout << "KirbyRecomp Generator v0.1.0\n";
    std::cout << "Static recompilation pipeline for Kirby and the Forgotten Land\n\n";

    gen::GeneratorOptions opts;
    if (!ParseArgs(argc, argv, opts)) {
        return 1;
    }

    if (!ValidateInputs(opts)) {
        return 1;
    }

    if (opts.verbose) {
        std::cout << "ExeFS: " << opts.exefs_path << "\n";
        std::cout << "RomFS: " << opts.romfs_path << "\n";
        std::cout << "Keys:  " << opts.keys_path << "\n";
        std::cout << "Out:   " << opts.output_dir << "\n\n";
    }

    gen::Generator generator(opts);
    bool success = generator.Run();

    return success ? 0 : 1;
}