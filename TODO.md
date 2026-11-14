# KirbyRecomp - Task Tracker

## Immediate (Week 1-2)

### Repository Setup
- [ ] Initialize git repository
- [ ] Add SwitchRecomp as git submodule (`git submodule add https://github.com/kaiyao7/SwitchRecomp vendor/switchrecomp`)
- [ ] Create CMakeLists.txt with C++20, MSVC/Clang support
- [ ] Copy `vendor/switchrecomp/include/switchrecomp/common/numeric.h` to `include/kirbyrecomp/numeric.h`
- [ ] Set up GitHub Actions CI (Windows MSVC + Clang)

### Core Infrastructure
- [ ] Create `src/translator/` directory structure
- [ ] Implement Capstone-based AArch64 disassembler wrapper
- [ ] Define IR instruction set (subset of SwitchRecomp IR)
- [ ] Implement IR → JSON serialization
- [ ] Basic codegen: IR → C (integer ALU ops only)

## Short Term (Month 1-2)

### Translator Core
- [ ] Implement AArch64 instruction decoder (integer core)
- [ ] Add golden fixture tests (instruction → IR → C → verify)
- [ ] Integrate SwitchRecomp's end-to-end test framework
- [ ] Achieve 100% integer instruction coverage (per SwitchRecomp)

### NSO/NCA Handling
- [ ] Implement NSO header parsing
- [ ] Implement section loading (text, rodata, data, bss)
- [ ] Implement LZ4 decompression
- [ ] Implement relocation processing
- [ ] Implement NCA extraction (using hactool/nscb as reference)

## Medium Term (Month 2-4)

### Control Flow & Memory
- [ ] Guest memory manager (4GB, page tables, W^X)
- [ ] Function identification & boundary detection
- [ ] Jump table recovery (Unicorn-assisted)
- [ ] Vtable recovery for C++ classes
- [ ] Function pointer analysis

### HLE Services - Phase 1
- [ ] Service registry (`sr_hle_register`, `sr_hle_invoke`)
- [ ] `sm:` Service Manager (register/get service)
- [ ] `fs:` RomFS read-only access
- [ ] `am:` Basic app lifecycle (GetAppletType, etc.)

## Medium-Long Term (Month 4-8)

### HLE Services - Phase 2
- [ ] `hid:` Input (SDL3 keyboard/mouse/gamepad)
- [ ] `time:` System time
- [ ] `set:` System settings
- [ ] Threading SVCs (CreateThread, WaitSynchronization, etc.)
- [ ] `fs:` Save data (DISF/DPFS, AES-CMAC)

### Graphics - Shader Translation
- [ ] Extract Maxwell shaders from NSO/RomFS
- [ ] Parse Maxwell bytecode format
- [ ] Port Ryujinx nxnative translator (MIT) to C++/standalone
- [ ] Generate SPIR-V modules
- [ ] Shader reflection (bindings, push constants, descriptors)
- [ ] Pipeline cache generation at install time

## Long Term (Month 8-16)

### Graphics - NVN Runtime
- [ ] Maxwell FIFO command parser
- [ ] Command processor dispatch
- [ ] Resource management (textures, buffers, samplers, images)
- [ ] PSO creation & caching
- [ ] Draw/dispatch command translation to Vulkan
- [ ] Synchronization (fences, semaphores, events)
- [ ] Compute pipeline support
- [ ] VMA integration for memory allocation
- [ ] FSR 1 + SMAA integration

### Audio
- [ ] `audren:u` Audio Renderer (software mixing)
- [ ] `audout:u` Audio Output (SDL3 audio callback)
- [ ] `hwopus:` Opus decode (libopus or FFmpeg)
- [ ] Buffer event system

## Game-Specific (Month 14+)

### Kirby and the Forgotten Land
- [ ] Parse KFL ExeFS (identify all NSO modules)
- [ ] Handle larger code size (more modules than SKC)
- [ ] Game-specific NVN commands
- [ ] Color LUT handling fix
- [ ] Depth clamp implementation
- [ ] Save data support
- [ ] Performance profiling & optimization
- [ ] 120 FPS mode implementation
- [ ] Full playthrough validation

## Ongoing
- [ ] Update ARCHITECTURE.md after each milestone
- [ ] Update RESEARCH.md with new findings
- [ ] Legal review of distribution model
- [ ] Community documentation (README.md, BUILDING.md)