# KirbyRecomp - Prerequisites for Development Machine

## Required (Install Before Starting)

### Build System
| Tool | Version | Install Method |
|------|---------|----------------|
| **Git** | 2.40+ | winget: `Git.Git` / chocolatey: `git` |
| **CMake** | 3.25+ | winget: `Kitware.CMake` / chocolatey: `cmake` |
| **Ninja** | 1.11+ | winget: `Ninja.Build.Ninja` / chocolatey: `ninja` |

### Compilers (at least one)
| Compiler | Version | Install Method |
|----------|---------|----------------|
| **MSVC** | VS 2022 17.8+ | Visual Studio Installer → "Desktop development with C++" |
| **Clang/LLVM** | 17+ | winget: `LLVM.LLVM` / chocolatey: `llvm` |
| **Zig** | 0.12+ | winget: `ziglang.Zig` / chocolatey: `zig` |

> **Recommended**: Install **both MSVC and Zig**. Zig cc (Clang/LLD) provides consistent cross-platform builds; MSVC for native Windows debugging.

### Graphics & SDK
| Tool | Version | Install Method |
|------|---------|----------------|
| **Vulkan SDK** | 1.3.275+ | Download from [vulkan.lunarg.com](https://vulkan.lunarg.com/sdk/home) |
| **Windows SDK** | 10.0.22621+ | Visual Studio Installer (included with MSVC) |

### Runtime Dependencies (for generated executable)
| Library | Purpose | Install Method |
|---------|---------|----------------|
| **SDL3** | Windowing, input, audio | vcpkg: `sdl3` or build from source |
| **Vulkan Memory Allocator (VMA)** | GPU memory management | vcpkg: `vma` or GitHub release |
| **libopus / FFmpeg** | Opus audio decoding | vcpkg: `opus` `ffmpeg` |

> **Use vcpkg** for C++ dependencies on Windows:
```bash
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg.exe install sdl3 vma opus ffmpeg
```

---

## Optional (Can Install Later)

| Tool | Purpose | When Needed |
|------|---------|-------------|
| **Capstone** | Disassembly framework | M1 (translator) - can use vcpkg: `capstone` |
| **Unicorn** | Instruction emulation (CFG recovery) | M2 - vcpkg: `unicorn` |
| **Python 3.11+** | Generator scripts (like LASRecomp) | M1-M2 - winget: `Python.Python.3.11` |
| **hactool / nscb** | NCA/NSO extraction reference | M2 - build from source |
| **Ghidra** | Reverse engineering analysis | Research phase - download from NSA |

---

## Verify Installation

Run this in PowerShell after installing:

```powershell
# Core tools
git --version
cmake --version
ninja --version

# Compilers
cl.exe 2>&1 | Select-String "Version"          # MSVC
clang --version                                 # Clang
zig version                                     # Zig

# Vulkan
$env:VULKAN_SDK                                 # Should show path

# vcpkg (if used)
.\vcpkg\vcpkg.exe list sdl3 vma opus ffmpeg
```

---

## Environment Variables to Set

Add to User or System environment variables:

| Variable | Value |
|----------|-------|
| `VULKAN_SDK` | `C:\VulkanSDK\1.3.275.0` (or your install path) |
| `VCPKG_ROOT` | `C:\path\to\vcpkg` (if using vcpkg) |
| `PATH` | Add: `%VCPKG_ROOT%`, `%VULKAN_SDK%\Bin` |

---

## Directory Structure (Create Before Starting)

```powershell
# On the development machine:
mkdir KirbyRecomp
cd KirbyRecomp
# Copy all .md files here (RESEARCH.md, ROADMAP.md, TODO.md, etc.)
# Then:
git init
git submodule add https://github.com/kaiyao7/SwitchRecomp vendor/switchrecomp
```

---

## First Build Test (After M1 Setup)

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang-cl
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

---

## Notes

- **Windows 10/11 x64** required (target platform)
- **AVX2/FMA CPU** required for numeric layer efficiency (most CPUs since 2013)
- **Vulkan 1.3 capable GPU** required for runtime testing (most GPUs since 2018)
- **8GB+ RAM** recommended (4GB guest space + runtime + OS)

All `.md` documentation files should be copied to the new machine — they contain the complete project specification.