# Contributing to StarRecomp

Thank you for your interest in contributing to StarRecomp! This document will help you get set up and start contributing.

## Table of Contents
- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Environment Setup](#development-environment-setup)
- [Building the Project](#building-the-project)
- [Running Tests](#running-tests)
- [Code Style & Guidelines](#code-style--guidelines)
- [Submitting Changes](#submitting-changes)
- [Reporting Issues](#reporting-issues)
- [Architecture Overview](#architecture-overview)

## Code of Conduct

This project follows a standard code of conduct. Please be respectful, inclusive, and constructive in all interactions.

## Getting Started

### Prerequisites

Before you begin, ensure you have the following installed:

#### Required Software (Windows)

| Tool | Version | Purpose |
|------|---------|---------|
| **MSYS2** | Latest | Build environment (MSYS2/MinGW64) |
| **Clang** | 16+ | C/C++ Compiler (via MSYS2) |
| **Ninja** | 1.11+ | Build system |
| **CMake** | 3.25+ | Build configuration |
| **Git** | 2.30+ | Version control |
| **Vulkan SDK** | 1.4.363.0+ | Vulkan API headers/libs |

#### Installing Dependencies (Windows/MSYS2)

```bash
# 1. Install MSYS2 from https://www.msys2.org/
# 2. Open "MSYS2 MinGW 64-bit" terminal
# 3. Update packages
pacman -Syu

# Install required packages
pacman -S mingw-w64-x86_64-clang \
          mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-ninja \
          mingw-w64-x86_64-vulkan-headers \
          mingw-w64-x86_64-vulkan-loader \
          mingw-w64-x86_64-lz4 \
          mingw-w64-x86_64-opus \
          git

# 4. Install Vulkan SDK from https://vulkan.lunarg.com/sdk/home
# Download and run the installer (1.4.363.0 or newer)

# 5. Install Git if not present
pacman -S git
```

### Setting Up Your Fork

1. **Fork the repository** on GitHub
2. **Clone your fork** with submodules:
   ```bash
   git clone --recurse-submodules https://github.com/YOUR_USERNAME/StarRecomp.git
   cd StarRecomp
   ```
3. **Add upstream remote**:
   ```bash
   git remote add upstream https://github.com/star-recomp-switch/StarRecomp.git
   ```

---

## Development Environment Setup

### Required Environment Variables

Add these to your shell profile (`.bashrc`, `.zshrc`, etc.):

```bash
# Vulkan SDK path (adjust version as needed)
export VULKAN_SDK="C:/VulkanSDK/1.4.363.0"

# Add to PATH if not already present
export PATH="/c/msys64/mingw64/bin:$PATH"
```

Or create a `.env` file in the project root:
```bash
VULKAN_SDK=C:/VulkanSDK/1.4.363.0
```

### IDE/Editor Setup

**Recommended: Visual Studio Code**
```bash
# Install recommended extensions
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension twxs.cmake
```

**Recommended VS Code settings** (`.vscode/settings.json`):
```json
{
  "cmake.configureOnOpen": false,
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.generator": "Ninja",
  "cmake.cmakePath": "cmake",
  "cmake.buildTool": "ninja",
  "cmake.generator": "Ninja",
  "cmake.configureArgs": [
    "-G", "Ninja",
    "-DCMAKE_PREFIX_PATH=C:/VulkanSDK/1.4.363.0",
    "-DCMAKE_C_COMPILER=clang",
    "-DCMAKE_CXX_COMPILER=clang++"
  ]
}
```

---

## Building the Project

### Quick Build

```bash
# Configure
cmake -B build -S . -G Ninja

# Build
cmake --build build --parallel

# Run tests
ctest --test-dir build --output-on-failure
```

### Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `-DCMAKE_BUILD_TYPE=Release` | Release build | `Release` |
| `-DBUILD_TESTS=ON` | Enable tests | `ON` |
| `-DBUILD_GAME=OFF` | Build game executable | `OFF` |

### Build Commands

```bash
# Configure (first time or after CMakeLists.txt changes)
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build all targets
cmake --build build --parallel

# Build specific target
cmake --build build --target kirbyrecomp_gen

# Clean build
rm -rf build
cmake -B build -S . -G Ninja
```

### Build Targets

| Target | Description |
|--------|-------------|
| `kirbyrecomp_gen` | NSO → C translator tool |
| `kirbyrecomp_rt` | Runtime library |
| `kirbyrecomp_hle_tests` | HLE service tests |
| `sr_tests` | SwitchRecomp unit tests |

---

## Running Tests

### Run All Tests
```bash
ctest --test-dir build --output-on-failure --parallel
```

### Run Specific Test Suite
```bash
# HLE Service tests
ctest --test-dir build -R kirbyrecomp_hle

# NSO parser tests
ctest --test-dir build -R kirbyrecomp_nso

# SwitchRecomp unit tests
cmake --build build --target sr_tests
ctest --test-dir build -R sr_tests
```

### Run Single Test
```bash
./build/bin/kirbyrecomp_hle_tests.exe --gtest_filter=HleServiceManager.CreateAndGetServices
```

### Test Categories
| Test Suite | Description |
|------------|-------------|
| `kirbyrecomp_hle` | HLE service tests |
| `kirbyrecomp_nso` | NSO parser tests |
| `kirbyrecomp_nso_integration` | NSO integration tests |
| `sr_tests` | SwitchRecomp unit tests |

---

## Code Style & Guidelines

### C/C++ Style

- **Standard**: C++20 (compiler), C11 (C code)
- **Formatting**: `clang-format` (config in `.clang-format`)
- **Line length**: 100 characters max
- **Indentation**: 4 spaces (no tabs)
- **Braces**: Allman style (braces on new lines)

### Naming Conventions

| Entity | Convention | Example |
|--------|------------|---------|
| Functions | `snake_case` | `create_texture()` |
| Variables | `snake_case` | `texture_handle` |
| Types/Structs | `PascalCase` | `TextureResource` |
| Constants/Enums | `UPPER_SNAKE_CASE` | `TEXTURE_FORMAT_RGBA8` |
| Macros | `UPPER_SNAKE_CASE` | `MAX_TEXTURE_SIZE` |

### Commit Message Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types**: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`, `build`, `ci`, `perf`

**Examples**:
```
feat(nvn): add ASTC to BC7 texture transcoding

Implements ASTC-to-BC7 transcoding in the texture upload pipeline.
Supports all ASTC block sizes (4x4 through 12x12) and SRGB variants.

Fixes #123
```

---

## Submitting Changes

### Pull Request Process

1. **Create a feature branch**:
   ```bash
   git checkout -b feat/your-feature-name
   ```

2. **Make your changes**:
   - Write code with tests
   - Follow code style
   - Update documentation if needed

2. **Run tests locally**:
   ```bash
   ctest --test-dir build --output-on-failure
   ```

3. **Commit with conventional commits**:
   ```bash
   git commit -m "feat(nvn): add ASTC texture support
   
   Implements ASTC to BC7 transcoding for texture uploads.
   Supports all block sizes (4x4 through 12x12).
   
   Fixes #123"
   ```

3. **Push and create PR**:
   ```bash
   git push origin feat/your-feature
   # Open PR on GitHub
   ```

### Pull Request Checklist

- [ ] Code compiles without warnings (`-Wall -Wextra`)
- [ ] All tests pass (`ctest --output-on-failure`)
- [ ] Code follows style guide (`clang-format`)
- [ ] Documentation updated (README, comments, etc.)
- [ ] Tests added for new functionality
- [ ] Commit messages follow conventional commits
- [ ] No large binary files committed

### PR Review Process

1. **Automated checks**: CI runs build + tests
2. **Code review**: At least 1 approval required
3. **Merge**: Squash and merge after approval

---

## Reporting Issues

### Bug Reports

Use the [GitHub issue tracker](https://github.com/star-recomp-switch/StarRecomp/issues) with:

- **Title**: Clear, descriptive title
- **Environment**: OS, compiler, Vulkan SDK version
- **Steps to reproduce**: Minimal reproduction steps
- **Expected vs actual behavior**
- **Logs/output**: Relevant logs, error messages
- **Screenshots**: If applicable

### Feature Requests

Use the issue template with:
- Clear description of the feature
- Use case / motivation
- Possible implementation approach
- Alternatives considered

### Labels

| Label | Purpose |
|-------|---------|
| `bug` | Something isn't working |
| `feature` | New feature request |
| `docs` | Documentation improvement |
| `good first issue` | Good for newcomers |
| `help wanted` | Community help needed |
| `blocked` | Blocked by external dependency |

---

## Architecture Overview

### High-Level Structure

```
StarRecomp/
├── src/
│   ├── codegen/          # C code generation from IR
│   ├── gen/              # Generator tool (NSO → C)
│   ├── gpu/              # GPU runtime (sr_gpu)
│   ├── hle/              # HLE services (audio, fs, etc.)
│   ├── kfl/              # KFL module loader
│   ├── nvn/              # NVN → Vulkan translation
│   ├── nso/              # NSO parser/loader
│   ├── nvn/              # NVN → Vulkan translation
│   ├── rt/               # Runtime (memory, threading)
│   ├── shader/           # Shader translation (Maxwell → SPIR-V)
│   ├── translator/       # AArch64 → IR translator
│   └── gen/              # NSO → C generator tool
├── tests/
│   ├── integration/      # Integration tests
│   └── unit/             # Unit tests
├── vendor/               # Git submodules (dependencies)
└── cmake/                # Custom CMake modules
```

### Key Modules

| Module | Purpose |
|--------|---------|
| `nvn/` | NVN (Switch GPU API) → Vulkan translation |
| `shader/` | Maxwell bytecode → SPIR-V translation |
| `kfl/` | KFL module loader (ExeFS/NSO) |
| `hle/` | High-Level Emulation (audio, fs, input, etc.) |
| `nvn/` | NVN command translation to Vulkan |
| `shader/` | Maxwell bytecode → SPIR-V |
| `kfl/` | KFL module loader (ExeFS/NSO) |
| `gen/` | NSO → C translator tool |

### Key Dependencies

| Dependency | Version | Purpose |
|------------|---------|---------|
| Vulkan SDK | 1.4.363.0+ | Graphics API |
| VMA | 3.5.0 | Vulkan Memory Allocator |
| LZ4 | 1.9.4+ | Compression |
| Opus | 1.5+ | Audio decoding |
| FFmpeg | 7.0+ | Media decoding |
| SDL3 | 3.2+ | Window/input |
| Capstone | 5.0+ | Disassembly |
| SDL3 | 3.2+ | Window/input/audio |

---

## Useful Commands Quick Reference

```bash
# Configure
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Run specific test
./build/bin/kirbyrecomp_hle_tests.exe --gtest_filter=HleServiceManager.CreateAndGetServices

# Format code
clang-format -i src/**/*.cpp src/**/*.h

# Check formatting
clang-format --dry-run --Werror src/**/*.cpp src/**/*.h

# Update submodules
git submodule update --init --recursive --remote
```

---

## Questions?

- **Discord**: [StarRecomp Discord](https://discord.gg/starrecomp) (if available)
- **Issues**: [GitHub Issues](https://github.com/star-recomp-switch/StarRecomp/issues)
- **Discussions**: [GitHub Discussions](https://github.com/star-recomp-switch/StarRecomp/discussions)

---

*Last updated: 2026-11-22*  
*For the latest version, see [CONTRIBUTING.md](https://github.com/star-recomp-switch/StarRecomp/blob/main/CONTRIBUTING.md)*