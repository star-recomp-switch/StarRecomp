# KirbyRecomp - Technical Architecture

## Overview
This document describes the technical architecture of the KirbyRecomp static recompilation pipeline and runtime.

## System Components

### 1. Intake & Validation (`src/intake/`)
**Purpose**: Validate and extract user-provided game files

**Inputs**:
- `ExeFS/` directory (extracted from NCA)
- `RomFS/` directory (extracted from NCA)
- `prod.keys` (from user's console)

**Outputs**:
- Validated NSO module list with metadata
- RomFS file index
- `provenance.toml` recording source hashes

**Key Functions**:
- `validate_prod_keys()` - Verify key format and derive title keys
- `extract_nso_modules()` - Parse NCA, extract NSO, handle LZ4
- `parse_nso_header()` - Read module metadata (build ID, version, etc.)
- `index_romfs()` - Build file map for runtime access

### 2. AArch64 → IR Translator (`src/translator/`)
**Purpose**: Convert AArch64 machine code to platform-independent IR

**Architecture** (based on SwitchRecomp):
```
AArch64 bytes → Capstone disasm → Instruction IR → Function IR → Module IR
                                    ↓
                            Unicorn emulation (for CFG recovery)
```

**IR Design** (subset of SwitchRecomp):
- SSA-like, typed, JSON-serializable
- Instructions: `ir::Inst { opcode, operands[], result?, flags? }`
- Types: `i8/i16/i32/i64`, `f32/f64`, `v128` (SIMD)
- Control flow: `br`, `br_cond`, `br_indirect`, `call`, `ret`
- Memory: `load`, `store`, `load_global`, `store_global`
- Intrinsics: `svc`, `hvc`, `smc`, `sys_reg_read/write`

**Key Components**:
- `decoder/` - Capstone wrapper, instruction → IR mapping
- `cfg/` - Control flow graph construction, dominance, loops
- `recovery/` - Jump table recovery (Unicorn), vtable recovery, function pointers
- `serialize/` - IR → JSON (for caching, debugging)

**Testing**: Golden fixtures (instruction → IR → C → verify numeric equivalence)

### 3. C Code Generation (`src/codegen/`)
**Purpose**: Emit compilable C11 from IR

**Design** (SwitchRecomp approach):
- One `.c` file per function (enables parallel compilation)
- `numeric.h` for bit-exact AArch64 semantics
- Minimal runtime intrinsics (`sr_*` prefix)

**Output Structure**:
```
generated/
  module_<name>/
    func_<addr>_<name>.c
    func_<addr>_<name>.h
    module.h          # Function declarations
    module.c          # Module init/fini, global data
```

**Numeric Layer** (`include/kirbyrecomp/numeric.h`):
- Header-only, `static inline` functions
- Bit-exact: shifts ≥ width, signed overflow, FP rounding, conversions
- Folds to 1-2 host instructions at `-O2`
- Covers: integer, FP32/FP64, AdvSimd (growing subset)

### 4. Shader Translation (`src/shader/`)
**Purpose**: Convert Maxwell bytecode → SPIR-V at build time

**Pipeline**:
```
Game shader bytecode (from RomFS/NSO)
         │
         ▼
┌─────────────────────────────────────┐
│  Maxwell Parser                     │
│  - Instruction decode (envytools)   │
│  - Control flow reconstruction      │
│  - Resource binding analysis        │
└─────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────┐
│  SPIR-V Generator                   │
│  - Ryujinx nxnative adaptation      │
│  - SSA construction                 │
│  - SPIR-V module emission           │
└─────────────────────────────────────┘
         │
         ▼
┌─────────────────────────────────────┐
│  Pipeline Cache Generator           │
│  - Reflection (bindings, push const)│
│  - Variant enumeration              │
│  - vulkan_pipeline_cache.bin        │
└─────────────────────────────────────┘
```

**Key Challenges**:
- Maxwell ISA → SPIR-V semantic gaps
- Texture/sampler binding model differences
- Uniform buffer vs push constant mapping
- Specialization constants for variants

### 5. Host Compilation (`build/`, `CMakeLists.txt`)
**Purpose**: Compile generated C + runtime into native executable

**Toolchain Options**:
- **Primary**: Zig cc (Clang/LLD) - cross-platform, consistent
- **Windows**: MSVC + Clang-cl
- **Linux/macOS**: Clang/GCC

**CMake Targets**:
- `kirbyrecomp_gen` - Generator tool (translator + shader)
- `kirbyrecomp_rt` - Runtime library (static)
- `kirbyrecomp_game` - Final game executable (links generated + rt)

**Build Flow**:
```bash
# 1. Generate C from user's game files
kirbyrecomp_gen --exefs ... --romfs ... --keys ... --out generated/

# 2. Compile generated + runtime
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target kirbyrecomp_game

# 3. Output: build/kirbyrecomp_game.exe
```

### 6. Runtime (`src/runtime/`)
**Purpose**: HLE of Horizon OS services + NVN→Vulkan + audio + memory

#### 6.1 Service Registry (`service/`)
```cpp
// HLE service interface
using SrHleInvokeFn = Result (*)(SrHleService*, u32 cmd_id, 
                                  const SrHleInParams*, SrHleOutParams*);

struct SrHleService {
    const char* name;           // "nvn", "fs", "am", etc.
    SrHleInvokeFn invoke;
    void* userdata;
};

// Registry
Result sr_hle_register(SrHleService* service);
Result sr_hle_invoke(u32 service_id, u32 cmd_id, ...);
```

#### 6.2 Core Services
| Service | Interface | Key Commands |
|---------|-----------|--------------|
| `sm:` | `IServiceManager` | RegisterService, GetService, UnregisterService |
| `fs:` | `IFileSystem` | OpenFile, ReadFile, GetFileSize, RomFS mount |
| `am:` | `IApplicationManager` | GetAppletType, GetPerformanceMode |
| `nvn:` | `INvn*` | DeviceCreate, QueueSubmit, MemoryMap, etc. |
| `hid:` | `IHid*` | GetNpadState, GetMouseState |
| `audren:` | `IAudioRenderer` | RequestUpdate, AppendBuffer, Start/Stop |
| `audout:` | `IAudioOut` | AppendAudioOutBuffer, RegisterBufferEvent |
| `hwopus:` | `IHardwareOpusDecoder` | DecodeInterleaved |
| `time:` | `ITime` | GetCurrentTime, GetSystemTick |
| `set:` | `ISettings` | GetSettingValue |

#### 6.3 NVN → Vulkan Layer (`gpu/nvn/`)
**Command Buffer Parsing**:
- Maxwell FIFO format (pushbuf, method calls)
- Method opcodes → Vulkan command buffer recording
- Resource handles → Vulkan objects (VkImage, VkBuffer, etc.)

**Resource Management**:
- Texture/buffer/sampler pools
- Descriptor set layout caching
- Pipeline state object (PSO) cache keyed by shader + state

**Synchronization**:
- Fence/semaphore mapping (NVN syncpts → Vulkan timeline semaphores)
- Event handling
- Host/device memory coherence

**Compute Support**:
- Compute pipeline creation
- Dispatch translation
- Shared memory / workgroup management

#### 6.4 Memory Manager (`mem/`)
- 4GB guest address space (matches Switch UMA)
- Page tables with W^X (write XOR execute)
- NSO module loading with relocations
- Heap allocation for game malloc/free
- Stack guard pages

#### 6.5 Audio (`audio/`)
- Ring buffer matching `AudioOutBuffer` layout
- Software mixing (audren HLE)
- Opus decode via libopus/FFmpeg (hwopus HLE)
- SDL3 audio callback for output

#### 6.6 Threading (`thread/`)
- SVC emulation: CreateThread, ExitThread, SleepThread
- Synchronization: Arbitration (mutex), Event, CondVar
- TLS support for Thread Local Region (HIPC buffers)

### 7. Generated Code ↔ Runtime Interface

**Entry Points** (from generated module.c):
```c
// Called by runtime at startup
void sr_module_init(SrModule* mod, SrRuntime* rt);
void sr_module_fini(SrModule* mod);

// Exported functions (called via function pointers)
typedef void (*SrFuncPtr)(SrCpuState* state, ...);
```

**CPU State** (passed to every translated function):
```c
struct SrCpuState {
    u64 x[31];        // X0-X30
    u64 sp, pc;       // SP, PC
    u64 pstate;       // NZCV flags
    f32 v[32][4];     // V0-V31 (SIMD/NEON)
    u32 fpsr, fpcr;   // FP status/control
};
```

**Syscall/Service Interface**:
```c
// Generated code calls this for SVC
u64 sr_svc_call(SrCpuState* state, u32 svc_num, u64 args[8]);

// Generated code calls this for HLE service invoke
Result sr_hle_invoke(u32 service_id, u32 cmd_id, ...);
```

## Data Flow Summary

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│ User Files  │────▶│  Generator  │────▶│   Generated │
│ (ExeFS,     │     │ (Translator │     │     C Code  │
│  RomFS,     │     │  + Shader)  │     │  (per func) │
│  prod.keys) │     └─────────────┘     └──────┬──────┘
└─────────────┘                                  │
                                                 ▼
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Native     │◀────│  Host       │◀────│  Generated  │
│  Executable │     │  Compiler   │     │  + Runtime  │
└─────────────┘     └─────────────┘     └─────────────┘
                           │
                           ▼
                    ┌─────────────┐
                    │  Runtime    │
                    │ (HLE, NVN,  │
                    │  Audio,     │
                    │  Memory)    │
                    └─────────────┘
```

## Configuration Files

### `provenance.toml` (required for builds)
```toml
[source]
game_title = "Kirby and the Forgotten Land"
title_id = "01003FB00C5A8000"
version = "1.0.0"
build_id = "c9649e053bd7be2751ef0b00e7c03f5d59695b94"

[inputs]
exefs_hash = "sha256:..."
romfs_hash = "sha256:..."
prod_keys_hash = "sha256:..."
dumped_by = "user"
dump_tool = "Lockpick_RCM"
dump_date = "2026-11-12"

[toolchain]
kirbyrecomp_version = "0.1.0"
switchrecomp_version = "commit-hash"
lasrecomp_shader_version = "commit-hash"
```

### `build_config.toml` (generator options)
```toml
[translator]
optimization_level = 2
verify_numeric = true
emit_debug_info = false

[shader]
prepare_variants = true
pipeline_cache_path = "shader_cache.bin"

[runtime]
enable_120fps = false
log_level = "info"
validation_layers = false
```

## Testing Strategy

### Unit Tests
- `numeric.h` exhaustive tests (all edge cases)
- Instruction decoder round-trip (asm → IR → C → verify)
- NSO parser (valid/invalid headers, relocations, LZ4)
- Service registry (register, invoke, error handling)

### Integration Tests
- Generator: NSO → C → compile → run (hello world)
- Shader: Maxwell bytecode → SPIR-V → validate
- Runtime: Service mocks + generated code execution

### End-to-End Tests
- Super Kirby Clash: boot → title screen → first battle
- Kirby Forgotten Land: progressive area validation

### Regression Prevention
- Golden fixture hashes for generated C
- SPIR-V binary hashes for shaders
- Frame capture comparison (future)

## Performance Considerations

### Translation Time
- Parallel per-function compilation
- Incremental builds (cache IR → C)
- Shader translation parallelized

### Runtime Performance
- NVN→Vulkan: minimize driver overhead
- Pipeline cache: reuse PSOs
- Memory: avoid guest↔host copies (UMA simulation)
- Audio: lock-free ring buffers

### Hardware Requirements
- CPU: AVX2 + FMA (for numeric layer efficiency)
- GPU: Vulkan 1.3, 4GB+ VRAM
- RAM: 8GB+ system (4GB guest + runtime + OS)

## Future Extensibility

### Multi-Game Support
- Game-specific config in `games/<title_id>/config.toml`
- Shared runtime, game-specific HLE quirks
- Plugin architecture for game-specific fixes

### Platform Ports
- Linux: native Vulkan, SDL3
- macOS: MoltenVK, SDL3
- Android: Vulkan, SDL3 (long-term)

### Graphics Enhancements
- DLSS/FSR 2/3 integration
- Ray tracing (if game shaders support)
- Higher resolution textures (RomFS replacement)