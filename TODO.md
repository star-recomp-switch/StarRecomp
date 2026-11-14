# KirbyRecomp - Task Tracker

## Current Status: Milestone 5 (NVN Shader Translation) - IN PROGRESS

---

## ✅ COMPLETED: Milestone 1-4 (Foundation through Code Gen Pipeline)

### Repository & Build
- [x] Git repository initialized
- [x] SwitchRecomp as git submodule
- [x] CMakeLists.txt with C++20, MSVC/Clang support
- [x] numeric.h integrated
- [x] GitHub Actions CI (Windows MSVC + Clang)

### Translator Core
- [x] Capstone-based AArch64 disassembler wrapper
- [x] IR definition with JSON serialization
- [x] IR → C codegen (integer, FP, SIMD subsets)
- [x] Generator tool (kirbyrecomp_gen.exe)
- [x] All 4 NSO modules processed (main, rtld, sdk, subsdk0)
- [x] Full executable linking and running

### NSO Loading
- [x] NSO header parsing (magic, version, flags, BSS size, module name)
- [x] Section loading (text, rodata, data, bss) with offsets/sizes
- [x] LZ4 decompression via system library
- [x] Relocation processing (framework ready)

### HLE Service Framework
- [x] Service registry (`kr::hle::ServiceManager`, `IService`)
- [x] `sm:` Service Manager
- [x] `fs:` File System (ExeFS/RomFS with full CRUD operations)
- [x] `am:` Application Manager
- [x] `hid:` Human Interface Device
- [x] `time:` Time Service
- [x] `set:` Settings Service
- [x] `audren:` Audio Renderer (SDL3)
- [x] `audout:` Audio Output (SDL3)
- [x] `hwopus:` Hardware Opus Decoder (libopus)
- [x] `nvn:` NVN Graphics (Vulkan backend basics)
- [x] Threading SVCs (CreateThread, WaitSynchronization, etc.)
- [x] All HLE tests passing (12 tests)

---

## 🔄 IN PROGRESS: Milestone 5 - NVN Shader Translation

### Shader Module Infrastructure
- [x] Created `src/shader/` module with CMake integration
- [x] NVN shader container format parser (`shader.h/cpp`)
  - NVN8 magic, header structure, section offsets
  - Support for all shader stages (vertex, fragment, compute, geometry, tessellation)
- [x] Maxwell ISA instruction decoder (`maxwell_decoder.h/cpp`)
  - 40+ Maxwell opcodes with format-specific decoding
  - Register file support (GPR, Predicate, CCR, SR, UR, CR, TR, Sampler, ADDR)
  - Operand types (register, immediate, constant, address)
  - Format decoders: R, I, C, LDST, TEX, CTRL, SYNC, MISC
- [x] SPIR-V generator (`spirv_generator.h/cpp`)
  - Complete SPIR-V builder with all major instructions
  - Type system: void, bool, int/float (8/16/32/64), vectors, matrices, images, samplers, arrays, structs, pointers, functions
  - Constants, variables, functions, control flow, memory access
  - Arithmetic (int/float), bitwise, comparison, vector ops, conversions
  - Image/sampler operations, barriers, subgroup ops
- [x] NVN service integration (`src/hle/service.cpp`)
  - `shader_create`: Parse Maxwell bytecode → translate to SPIR-V → create VkShaderModule
  - `program_create`: Link shader stages → create pipeline layout → create graphics/compute pipelines
  - Shader reflection extraction for resource bindings
  - SPIR-V binary storage for pipeline creation

### Remaining Milestone 5 Tasks
- [x] **Shader reflection implementation** - Extract inputs/outputs/descriptors from Maxwell bytecode
  - [x] Parse LDG/STG instructions for buffer bindings
  - [x] Parse TEX/TXF instructions for texture/sampler bindings
  - [x] Parse uniform register access for push constants / UBOs
  - [x] Extract vertex attributes from vertex shader
  - [x] Extract fragment outputs from fragment shader
  - [x] Extract compute local workgroup size
- [ ] **Pipeline cache generation** - Serialize VkPipelineCache at install time
  - [ ] Collect all shader variants during translation
  - [ ] Generate pipeline cache binary
  - [ ] Implement cache loading at runtime
- [ ] **Variant preparation** - Prepare common state combinations ahead of time
  - [ ] Blend state variants
  - [ ] Depth/stencil state variants
  - [ ] Rasterization state variants
  - [ ] Multisample state variants
- [ ] **Tests for Maxwell → SPIR-V translation**
  - [ ] Unit tests for decoder (golden fixtures)
  - [ ] Integration tests with known shader bytecode
  - [ ] SPIR-V validation with spirv-val
- [ ] **Improve Maxwell decoder accuracy**
  - [ ] Handle dual-issue instruction pairs correctly
  - [ ] Implement full predicate register logic
  - [ ] Support extended opcode space
  - [ ] Proper immediate encoding (32-bit, 64-bit)
  - [ ] Address register arithmetic

---

## 📋 PLANNED: Milestone 6 - NVN → Vulkan Runtime

### Command Buffer & Resource Management
- [ ] Maxwell FIFO command parser (using SwitchRecomp's command_processor)
- [ ] Command processor dispatch to Vulkan
- [ ] Resource management (textures, buffers, samplers, images)
- [ ] PSO creation & caching with proper descriptor set layouts
- [ ] Draw/dispatch command translation to Vulkan
- [ ] Synchronization (fences, semaphores, events, timeline semaphores)
- [ ] Compute pipeline support
- [ ] VMA integration for memory allocation
- [ ] FSR 1 + SMAA integration

### Advanced Features
- [ ] Texture format conversion (ASTC, BC, etc.)
- [ ] Buffer mapping strategies (persistent, coherent)
- [ ] Descriptor set pooling and reuse
- [ ] Pipeline layout compatibility checking
- [ ] Shader specialization constants for variants

---

## 📋 PLANNED: Milestone 7 - Audio HLE (Complete)

- [ ] `audren:u` Audio Renderer (software mixing - basic impl done)
- [ ] `audout:u` Audio Output (SDL3 audio callback - basic impl done)
- [ ] `hwopus:` Opus decode (libopus - basic impl done)
- [ ] Buffer event system
- [ ] Full audio pipeline integration

---

## 📋 PLANNED: Milestone 8 - Kirby Forgotten Land Specific

- [ ] Parse KFL ExeFS (identify all NSO modules)
- [ ] Handle larger code size (more modules than SKC)
- [ ] Game-specific NVN commands
- [ ] Color LUT handling fix
- [ ] Depth clamp implementation
- [ ] Save data support (DISF/DPFS, AES-CMAC)
- [ ] Performance profiling & optimization
- [ ] 120 FPS mode implementation
- [ ] Full playthrough validation

---

## 🔧 Ongoing Maintenance

- [x] Update ARCHITECTURE.md after each milestone
- [x] Update RESEARCH.md with new findings
- [x] Update PROGRESS.md after each session
- [ ] Legal review of distribution model
- [ ] Community documentation (README.md, BUILDING.md)
- [ ] Fix test process exit code 3 (static destruction order)
- [ ] Enable hash verification (investigate game-specific quirk)
- [ ] Fix ExeFS directory path for tests
- [ ] Improve module name parsing from MOD0 section

---

## Notes

**Milestone 5 Focus**: The shader translation pipeline is now architecturally complete:
1. **Parse** → NVN container / raw Maxwell bytecode
2. **Decode** → Maxwell instructions with operands
3. **Translate** → SPIR-V via builder
4. **Integrate** → VkShaderModule + VkPipeline

The remaining work is primarily in **shader reflection** (extracting resource bindings from decoded instructions) and **pipeline cache generation** (serializing the results for fast runtime loading).

**Reference implementations to study**:
- Ryujinx nxnative (Maxwell → SPIR-V, MIT)
- LASRecomp shader translator
- envytools/envyas (Maxwell ISA reference)
- SwitchRecomp's vulkan_backend.cpp (existing GPU infrastructure)