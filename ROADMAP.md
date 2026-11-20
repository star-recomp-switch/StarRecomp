# KirbyRecomp - Development Roadmap

## Vision
Create a static recompilation pipeline for *Kirby and the Forgotten Land* (Nintendo Switch) to native Windows x86-64, using legally obtained game files.

## Milestones

### M1: Foundation ✅ COMPLETE
- [x] CMake project setup (C++20, MSVC/Clang/Zig cc)
- [x] SwitchRecomp `numeric.h` integration
- [x] Capstone AArch64 disassembler
- [x] Basic IR definition & JSON serialization
- [x] IR → C codegen (integer + float + SIMD instructions)
- [x] Unit tests + CI (Windows MSVC + Clang)

### M2: NSO Loading & Control Flow ✅ COMPLETE
- [x] NSO parser (headers, sections, relocations, LZ4)
- [x] NCA extractor (with prod.keys)
- [x] Guest memory manager (4GB, W^X)
- [x] Control flow recovery (jump tables, vtables, indirect branches)
- [x] Unicorn-assisted indirect branch resolution (BLR/BR)
- [x] E2E test: AddImm, Call, FloatAdd, FCmp, FP64, FP Misc, FP Round

### M3: HLE Service Framework ✅ COMPLETE
- [x] Service registry
- [x] `sm:` Service Manager
- [x] `fs:` Filesystem (RomFS/ExeFS loaders)
- [x] `am:` Application Manager (stub)
- [x] `time:` / `set:` System services
- [x] `hid:` Input (SDL3)
- [x] Threading SVCs (create_thread, join, sleep)

### M4: NVN Shader Translation ✅ COMPLETE (Milestone 5)
- [x] Maxwell bytecode parser
- [x] SPIR-V generator (Ryujinx nxnative adaptation)
- [x] Shader reflection & resource bindings
- [x] Pipeline cache generation (install-time)
- [x] Variant preparation

### M5: NVN → Vulkan Runtime ✅ COMPLETE (Milestone 6)
- [x] Command buffer parsing (Maxwell FIFO)
- [x] Resource management (textures, buffers, samplers, shaders)
- [x] PSO mapping (pipeline creation from shader reflection)
- [x] Draw/dispatch translation (DRAW_ARRAYS, DRAW_ELEMENTS, DISPATCH)
- [x] Synchronization (barriers, events, fences, semaphores, queries)
- [x] Compute pipeline support
- [x] VMA integration (Vulkan Memory Allocator)
- [x] Descriptor set management + dynamic updates
- [x] Generator integration: CommandBufferParser + NVN replay C++ generation

### M8: KFL Module Loader ⬜ STUBBED (Milestone 8)
- [x] ExeFS directory parser (directory-based loading)
- [x] KflModuleLoader skeleton (load_exefs, get_modules)
- [x] Module type identification (main, rtld, sdk, subsdk0-2, dynsys, dynaudio, dyngfx, dynnvn)
- [ ] Real ExeFS file parser (single-file ExeFS container)
- [ ] Module dependency resolution & load ordering
- [ ] NSO relocation processing
- [ ] Module-specific NVN command handling (color LUT, depth clamp)

### Remaining Milestones

### M6: Audio HLE 🔄 NEXT
- [ ] `audren:u` Audio Renderer HLE
- [ ] `audout:u` Audio Output (SDL3 backend)
- [ ] `hwopus:` Opus Decoder (libopus)
- [ ] Buffer event handling & scheduling
- [ ] Audio sink integration (SDL3 Audio)

### M7: ASTC Texture Decoding 🔄 NEXT
- [ ] ASTC LDR decoder (4x4 through 12x12 blocks)
- [ ] ASTC → BC7 transcoding at upload time
- [ ] BC7 encoder (Mode 6, RGB+A)
- [ ] Integration with NVN texture upload pipeline

### M7: Kirby Forgotten Land (Months 14-24+)
- [ ] Game-specific NVN commands (color LUT, depth clamp, framebuffer sRGB)
- [ ] Save data (DISF/DPFS)
- [ ] Performance optimization (shader caching, PSO caching)
- [ ] 120 FPS mode (frame pacing)

## Success Criteria per Milestone
Each milestone must have:
- [x] Automated tests passing
- [x] Documentation updated (ARCHITECTURE.md, RESEARCH.md)
- [x] Code review completed
- [x] No known critical bugs blocking next milestone

## Risk Mitigation
- **Legal**: Zero game code in repo; provenance.toml for user inputs; LEGAL.md + NOTICE
- **Technical**: Validate against Super Kirby Clash at each graphics milestone
- **Schedule**: M6 (Audio) + M7 (ASTC) can be parallelized
- **Legal compliance**: LEGAL.md, NOTICE, provenance.toml, zero game assets in repo