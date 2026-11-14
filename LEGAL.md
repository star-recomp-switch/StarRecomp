# KirbyRecomp - Legal Policy

## Core Principle
**This project enables users to build a native Windows port of Kirby and the Forgotten Land from their own legally obtained game files. The repository itself contains ZERO Nintendo copyrighted material.**

## What This Repository Contains (Permitted)
- Original source code for the recompilation toolchain
- Original source code for the runtime (HLE services, NVN→Vulkan layer, audio, etc.)
- Build scripts, configuration, documentation
- Synthetic test binaries and original test assets created by the project
- License files for all third-party dependencies

## What This Repository Does NOT Contain (Prohibited)
- ❌ Game ROMs, NSP files, XCI files, or any game container formats
- ❌ Extracted NCA packages or NSO modules
- ❌ Game executable code (AArch64 machine code from the game)
- ❌ Game assets: models, textures, animations, audio, video
- ❌ Game shader bytecode (Maxwell bytecode)
- ❌ Nintendo encryption keys (`prod.keys`, `title.keys`, etc.)
- ❌ Nintendo firmware or system modules
- ❌ Any copyrighted Nintendo material in any form

## User Responsibilities
Users who build the project MUST:
1. **Own a legitimate copy** of Kirby and the Forgotten Land (physical cartridge or digital purchase)
2. **Dump their own game files** from their own Nintendo Switch console
3. **Extract their own `prod.keys`** from their own console (via Lockpick_RCM or equivalent)
4. **Provide these files as input** to the build process locally on their machine
5. **Not distribute** the resulting native executable or any intermediate game-derived files

## Build Process Design (Legal Safeguards)
- **Release builds ship ZERO game code** - only the toolchain and runtime
- **Generator runs on user's machine** - user's game files never leave their computer
- **No encryption circumvention in repo** - key extraction happens on user's console, not in our code
- **No DRM bypass** - we reimplement Horizon OS services (HLE), not circumvent protection
- **Provenance tracking** - builds require `provenance.toml` describing lawful input sources

## Third-Party Licenses
| Component | License | Notes |
|-----------|---------|-------|
| SwitchRecomp (core) | GPL-3.0 | Keeps derivatives open; we may fork or link |
| LASRecomp shader translator | MIT | Via Ryujinx nxnative (MIT) |
| Capstone | BSD-3-Clause | Disassembly framework |
| Unicorn | GPL-2.0 | Instruction emulation for CFG recovery |
| SDL3 | zlib | Windowing, input, audio |
| Vulkan Memory Allocator | MIT | GPU memory management |
| libopus / FFmpeg | BSD-3-Clause / LGPL | Audio decoding |
| Zig | MIT | Compiler toolchain (optional) |

## DMCA Considerations
- Static recompilation ≠ emulation (no runtime interpretation, no keys in repo)
- Nintendo's 2024-2026 DMCA sweeps targeted **emulators** with built-in decryption
- Our approach: user provides keys + game files; we provide toolchain only
- Precedent: LASRecomp, Super Kirby Clash Recomp, N64Recomp operate similarly

## Prohibited Activities
- Do not implement encryption circumvention or key extraction
- Do not provide instructions for obtaining `prod.keys` from unauthorized sources
- Do not distribute pre-built game executables or game-derived assets
- Do not upload user-provided game material to any external service
- Do not facilitate piracy in any form

## Legal Review
This policy is not legal advice. Consult an attorney for definitive guidance. The project maintains a conservative posture favoring user ownership and zero-repo-copyrighted-material.

## Reporting
If you believe any file in this repository violates this policy, open an issue or contact the maintainers immediately.