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
- [x] **Pipeline cache generation** - Serialize VkPipelineCache at install time
  - [x] Create pipeline cache on device initialization
  - [x] Load existing cache from disk (%APPDATA%/KirbyRecomp/)
  - [x] Save cache to disk on device finalize
  - [x] Use cache for graphics/compute pipeline creation
- [x] **Variant preparation** - Prepare common state combinations ahead of time
  - [x] Blend state variants (opaque, alpha, additive, premultiplied, multiply)
  - [x] Depth/stencil state variants (none, read-only, read-write, stencil)
  - [x] Rasterization state variants (fill cull back/front/none, wireframe)
  - [x] Multisample state variants (1x, 2x, 4x, 8x)
  - [x] Pipeline variant pre-generation with cache integration
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

## 🔄 IN PROGRESS: Milestone 6 - NVN → Vulkan Runtime

### Command Buffer & Resource Management
- [x] Maxwell FIFO command parser (command_buffer.h/cpp)
  - Command header parsing (opcode, size)
  - Draw commands (DRAW_ARRAYS, DRAW_ELEMENTS, etc.)
  - Compute dispatch (DISPATCH, DISPATCH_INDIRECT)
  - Resource binding (BIND_VERTEX_BUFFER, BIND_INDEX_BUFFER, BIND_SHADER_RESOURCE)
  - Pipeline state (SET_VIEWPORT, SET_SCISSOR, SET_BLEND_STATE, etc.)
  - Buffer/texture copy operations
  - Synchronization (BARRIER, EVENT_SIGNAL, EVENT_WAIT)
  - Memory management commands
- [x] Resource management (resource_manager.h/cpp)
  - Buffer management with GPU virtual addressing
  - Texture management with NVN→Vulkan format conversion
  - Sampler management with full descriptor support
  - Descriptor pool/set management
  - Pipeline management (stubs)
  - Command pool/buffer allocation
  - Synchronization primitives (fence, semaphore, event)
  - Memory type selection and format mapping
- [x] Command buffer translator (command_translator.h/cpp)
  - PipelineState tracking (vertex buffers, index buffer, descriptor sets, dynamic state)
  - Draw commands: DRAW_ARRAYS, DRAW_ELEMENTS, DRAW_ARRAYS_INSTANCED, DRAW_ELEMENTS_INSTANCED
  - Compute dispatch: DISPATCH
  - Resource binding: vertex/index buffers, shader resources, samplers, shaders
  - Pipeline state: viewport, scissor, sample mask
  - Buffer copies: COPY_BUFFER, COPY_BUFFER_REGION, UPDATE_BUFFER
  - Synchronization: BARRIER (memory/execution)
  - Dynamic state application (viewport, scissor)
  - Descriptor write flushing
  - Pipeline state management
- [x] PSO creation & caching with proper descriptor set layouts
  - PipelineDesc/ComputePipelineDesc with shader reflection
  - Descriptor set layout creation from shader reflection
  - Pipeline layout with push constant ranges
  - Graphics pipeline with vertex input, rasterization, blend, depth/stencil
  - Compute pipeline creation
  - Pipeline cache integration for install-time compilation
- [x] Synchronization (fences, semaphores, events, timeline semaphores)
  - Fence management: create, destroy, wait, reset, wait_fences
  - Semaphore management: binary and timeline semaphores
  - Timeline semaphore pool: create pool, acquire/release semaphores
  - Event management: create, destroy, set, reset, wait_event
  - Submit command buffer with full synchronization (wait/signal semaphores, fence)
  - Timeline semaphore submit info support
- [x] VMA integration for memory allocation
  - VmaAllocator for automatic memory management
  - Buffer/Image creation with vmaCreateBuffer/vmaCreateImage
  - Automatic memory type selection and dedicated allocations
  - Proper cleanup with vmaDestroyBuffer/vmaDestroyImage
  - Buffer device address support via VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT
- [x] FSR 1 + SMAA integration
  - FSR 1.0 EASU (Edge Adaptive Spatial Upsampling) pipeline
  - FSR 1.0 RCAS (Robust Contrast Adaptive Sharpening) pipeline
  - SMAA edge detection, blend weight, and neighborhood blending pipelines
  - Quality mode selection and parameter configuration

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