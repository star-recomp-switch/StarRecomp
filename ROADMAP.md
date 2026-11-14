# KirbyRecomp - Development Roadmap

## Vision
Create a static recompilation pipeline for *Kirby and the Forgotten Land* (Nintendo Switch) to native Windows x86-64, using legally obtained game files.

## Milestones

### M1: Foundation (Months 1-3) ✅ PLANNED
- [ ] CMake project setup (C++20, MSVC/Clang)
- [ ] SwitchRecomp `numeric.h` integration
- [ ] Capstone AArch64 disassembler
- [ ] Basic IR definition & JSON serialization
- [ ] IR → C codegen (integer instructions)
- [ ] Unit tests + CI (Windows MSVC + Clang)

### M2: NSO Loading & Control Flow (Months 3-5) ✅ PLANNED
- [ ] NSO parser (headers, sections, relocations, LZ4)
- [ ] NCA extractor (with prod.keys)
- [ ] Guest memory manager (4GB, W^X)
- [ ] Control flow recovery (jump tables, vtables)
- [ ] Unicorn-assisted indirect branch resolution
- [ ] E2E test: SKC NSO → C → native

### M3: HLE Service Framework (Months 5-8) ✅ PLANNED
- [ ] Service registry
- [ ] `sm:` Service Manager
- [ ] `fs:` Filesystem (RomFS/ExeFS)
- [ ] `am:` Application Manager
- [ ] `time:` / `set:` System services
- [ ] `hid:` Input (SDL3)
- [ ] Threading SVCs

### M4: NVN Shader Translation (Months 8-12) ✅ PLANNED
- [ ] Maxwell bytecode parser
- [ ] SPIR-V generator (Ryujinx nxnative)
- [ ] Shader reflection & resource bindings
- [ ] Pipeline cache generation (install-time)
- [ ] Variant preparation

### M5: NVN → Vulkan Runtime (Months 10-16) ✅ PLANNED
- [ ] Command buffer parsing (Maxwell FIFO)
- [ ] Resource management (textures, buffers, samplers)
- [ ] PSO mapping
- [ ] Draw/dispatch translation
- [ ] Synchronization (fences, semaphores)
- [ ] Compute pipeline support
- [ ] VMA integration

### M6: Audio HLE (Months 12-15) ✅ PLANNED
- [ ] `audren:u` Audio Renderer
- [ ] `audout:u` Audio Output (SDL3)
- [ ] `hwopus:` Opus Decoder
- [ ] Buffer event handling

### M7: Kirby Forgotten Land (Months 14-24+) ✅ PLANNED
- [ ] KFL NSO modules support
- [ ] Game-specific NVN commands
- [ ] Color LUT, depth clamp fixes
- [ ] Save data (DISF/DPFS)
- [ ] Performance optimization
- [ ] 120 FPS mode

## Success Criteria per Milestone
Each milestone must have:
- [ ] Automated tests passing
- [ ] Documentation updated (ARCHITECTURE.md, RESEARCH.md)
- [ ] Code review completed
- [ ] No known critical bugs blocking next milestone

## Risk Mitigation
- **Legal**: Zero game code in repo; provenance.toml for user inputs
- **Technical**: Validate against Super Kirby Clash at each graphics milestone
- **Schedule**: Parallelize M4/M5 where possible; M3 services can be stubbed initially