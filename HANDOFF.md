# KirbyRecomp - Session Handoff Document

## Session Summary
**Date**: 2026-11-12
**Status**: Feasibility research complete. Ready to begin implementation (Milestone 1).
**Recommendation**: PROCEED WITH KIRBY

---

## What Was Accomplished

### Research Completed (All Verified/Inferred)
1. **Game Architecture**: Kirby and the Forgotten Land runs on Switch (Tegra X1, Maxwell GPU), uses NVN graphics API, Horizon OS IPC services. Custom HAL Laboratory engine.
2. **Executable Formats**: NSO (relocatable ARM64 modules, LZ4, RSA), NCA containers, NSP/XCI encrypted (require prod.keys). Base address `0x7100000000`.
3. **CPU Translation**: AArch64 → x86-64 via static recompilation (AArch64 → IR → C → native). SwitchRecomp provides numeric.h for bit-exact semantics.
4. **Graphics**: NVN → Vulkan 1.3 translation. Maxwell bytecode → SPIR-V ahead-of-time. LASRecomp has working reference.
5. **Audio**: audren/audout/hwopus HLE via SDL3 + libopus/FFmpeg.
6. **Runtime**: HLE service registry for sm:, fs:, am:, nvn:, hid:, audren:, time:, set:, threading SVCs.
7. **Existing Projects**:
   - **SwitchRecomp (kaiyao7)**: GPL-3.0, most advanced framework (IR, codegen, numeric, tests) — **primary dependency**
   - **LASRecomp**: MIT, runs Link's Awakening — shader translator + NVN runtime reference
   - **Super Kirby Clash Recomp**: MIT, alpha, same engine family — validation target
   - **N64Recomp**: Mature N64 recompiler — distribution model reference

### Documents Created
| File | Purpose |
|------|---------|
| `RESEARCH.md` | 14-section feasibility report (all sources cited) |
| `ROADMAP.md` | 7 milestones over 14-24+ months |
| `TODO.md` | Granular task tracker by timeframe |
| `README.md` | Project overview with architecture diagram |
| `LEGAL.md` | Zero-game-code policy, provenance.toml requirement |
| `ARCHITECTURE.md` | Technical architecture: data flows, components, interfaces |
| `HANDOFF.md` | This file |

---

## Where to Pick Up (Next Steps)

### Immediate: Milestone 1 - Foundation (Months 1-3)

**Repository Setup** (Week 1):
```bash
cd D:\Claude Stuff\KirbyRecomp
git init
git submodule add https://github.com/kaiyao7/SwitchRecomp vendor/switchrecomp
# Create CMakeLists.txt with C++20, MSVC/Clang support
# Copy vendor/switchrecomp/include/switchrecomp/common/numeric.h → include/kirbyrecomp/numeric.h
# Set up GitHub Actions CI (Windows MSVC + Clang)
```

**Core Infrastructure** (Week 1-2):
- Create `src/translator/` directory structure
- Implement Capstone-based AArch64 disassembler wrapper
- Define IR instruction set (subset of SwitchRecomp IR)
- Implement IR → JSON serialization
- Basic codegen: IR → C (integer ALU ops only)

**Translator Core** (Month 1-2):
- Implement AArch64 instruction decoder (integer core)
- Add golden fixture tests (instruction → IR → C → verify numeric equivalence)
- Integrate SwitchRecomp's end-to-end test framework
- Achieve 100% integer instruction coverage (per SwitchRecomp)

---

## Key Technical Decisions Made

| Decision | Rationale |
|----------|-----------|
| **Primary language**: C++20 | Project requirement; MSVC/Clang compatible |
| **Build system**: CMake + Zig cc (or MSVC/Clang) | Cross-platform, used by LASRecomp |
| **IR**: SwitchRecomp's JSON-serializable SSA-like IR | Proven, tested, GPL-3.0 compatible |
| **Numeric layer**: SwitchRecomp's `numeric.h` | Bit-exact AArch64 semantics, header-only |
| **Shader translation**: Adapt Ryujinx nxnative (MIT) | Working Maxwell→SPIR-V translator |
| **NVN runtime**: LASRecomp reference (MIT) | Working Vulkan 1.3 backend |
| **Distribution**: Zero game code in repo; generator runs on user machine | Legal defensibility, proven by LASRecomp |
| **Validation target**: Super Kirby Clash first | Simpler, same engine family, already works |

---

## Critical Context for Next Session

### Legal Constraints (NON-NEGOTIABLE)
- **NO game files in repo ever** — no ROMs, NSPs, NSOs, keys, assets, shaders
- User provides ExeFS/RomFS/prod.keys locally
- Generator runs on user's machine
- `provenance.toml` required for every build
- See `LEGAL.md` for full policy

### Dependencies to Acquire
- `vendor/switchrecomp/` — git submodule (GPL-3.0)
- Capstone (BSD-3) — disassembly
- Unicorn (GPL-2) — CFG recovery (optional, can use SwitchRecomp's approach)
- SDL3 (zlib) — windowing/input/audio
- Vulkan SDK — graphics backend
- VMA (MIT) — GPU memory
- libopus/FFmpeg — audio decoding

### Validation Strategy
1. **Unit**: numeric.h exhaustive tests, instruction round-trip
2. **Integration**: NSO → C → compile → run (hello world)
3. **E2E**: Super Kirby Clash boot → title screen → first battle
4. **Target**: Kirby Forgotten Land progressive area validation

---

## Known Risks & Mitigations

| Risk | Mitigation |
|------|------------|
| Nintendo DMCA | Zero game code in repo; HLE not emulation; keys user-provided |
| Graphics complexity (3D vs LA 2.5D) | Validate against SKC at each graphics milestone; incremental |
| SIMD/NEON coverage | SwitchRecomp growing; prioritize Forgotten Land needs |
| Legal uncertainty | Conservative posture; consult attorney if needed |

---

## File Structure (Current)
```
KirbyRecomp/
├── HANDOFF.md          ← THIS FILE
├── README.md
├── RESEARCH.md
├── ROADMAP.md
├── TODO.md
├── LEGAL.md
├── ARCHITECTURE.md
├── CLAUDE.md           (project instructions)
├── RTK.md              (token optimization tool reference)
└── (no code yet)
```

---

## Commands to Resume Work

```bash
# On new machine:
cd /path/to/KirbyRecomp
# Verify documents exist
ls -la *.md

# Start implementation:
git init
git submodule add https://github.com/kaiyao7/SwitchRecomp vendor/switchrecomp
# ... begin M1 tasks from TODO.md
```

---

## Contact / Questions
- Refer to `RESEARCH.md` for all source URLs and verification status
- Refer to `ARCHITECTURE.md` for component interfaces and data flows
- Refer to `TODO.md` for granular next tasks
- Legal questions → `LEGAL.md`

**The project is ready for Milestone 1 implementation. No blockers.**