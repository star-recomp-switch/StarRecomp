# KirbyRecomp

Static recompilation pipeline for **Kirby and the Forgotten Land** (Nintendo Switch) → native Windows x86-64.

## Status
**Milestone 6 Complete** — ARM64→IR→C pipeline, shader translation (NVN→SPIR-V), NVN command translation, KFL module loader stub.

## Objective
Translate the game's AArch64 executable code to native x86-64 ahead of time, reimplement required Horizon OS services (NVN graphics, audio, filesystem, input) atop Windows/Vulkan/SDL3, and allow users to build the game locally from their **own legally obtained** game files.

## ⚖️ Legal — **Read First**
**This repository contains ZERO Nintendo copyrighted material.**

- ❌ No game ROMs, NSPs, XCIs, firmware, or encryption keys
- ❌ No game assets (models, textures, audio, shaders)
- ❌ No prod.keys, title keys, or master keys

**All game files must be user-supplied from their own legally owned copy.**

📖 **Full policy: [LEGAL.md](LEGAL.md)** — Read before building.

**TL;DR:** We ship code/tools only. You bring your own legally owned game. No Nintendo IP leaves your machine.

## Architecture Overview
```
User Game Files (ExeFS/RomFS + prod.keys)
         │
         ▼
┌─────────────────────────────────────┐
│  Intake & Validation                │
└─────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────┐
│  AArch64 → IR Translator            │
│  (Capstone + Unicorn + SwitchRecomp)│
└─────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────┐
│  C Code Generation                  │
│  (SwitchRecomp IR → C + numeric.h)  │
└─────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────┐
│  Shader Translation                 │
│  (Maxwell bytecode → SPIR-V)        │
└─────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────┐
│  Host Compilation                   │
│  (Zig cc / Clang / MSVC)            │
└─────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────┐
│  Runtime (KirbyRecomp)              │
│  - HLE Services (sm, fs, am, nvn,   │
│    hid, audren, time, set, etc.)    │
│  - NVN → Vulkan 1.3 Layer           │
│  - Audio (SDL3 + Opus)              │
│  - Memory Manager (W^X, 4GB)        │
└─────────────────────────────────────┘
```

## Key Technologies
- **Translator**: SwitchRecomp (GPL-3.0) - IR, codegen, numeric layer
- **Shader Translation**: Ryujinx nxnative (MIT) + envytools
- **NVN Runtime**: LASRecomp reference (MIT)
- **Build**: CMake + Zig cc (Clang/LLD) or MSVC/Clang
- **Runtime**: C++20, SDL3, Vulkan 1.3, VMA, libopus/FFmpeg

## Documentation
- [RESEARCH.md](RESEARCH.md) - Comprehensive feasibility report
- [ROADMAP.md](ROADMAP.md) - Milestone-based development plan
- [TODO.md](TODO.md) - Granular task tracker
- [ARCHITECTURE.md](ARCHITECTURE.md) - Technical architecture (WIP)
- [LEGAL.md](LEGAL.md) - Legal policy and provenance requirements

## Building (Future)
```bash
# Prerequisites: CMake, Zig cc or MSVC/Clang, Vulkan SDK, SDL3 dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Usage (Future)
```bash
# User provides their own game files
kirbyrecomp-gen --exefs path/to/ExeFS --romfs path/to/RomFS --keys path/to/prod.keys
# Outputs native Windows executable
```

## References
- [SwitchRecomp](https://github.com/kaiyao7/SwitchRecomp) - Core recompilation framework
- [LASRecomp](https://github.com/peiikai/LASRecomp) - Link's Awakening recompilation (working commercial game)
- [Super Kirby Clash Recomp](https://github.com/xtamqispeak/KirbyRecomp) - Same engine family, alpha
- [Nintendo Switch Decompilation Wiki](https://initialrise.github.io/REWiki/04-recomp/switch-decomp/)

## License
- Project code: MIT (unless incorporating GPL-3.0 SwitchRecomp components)
- SwitchRecomp components: GPL-3.0 (keeps derivatives open)
- No game files covered by source license

## Big Thanks
Many thanks to Claude for the assistance. Without Fable 5.2, this wouldn't have been possible!!
