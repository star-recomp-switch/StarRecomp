#pragma once

#include "kirbyrecomp/types.h"

namespace kr {

using sr::common::u8;
using sr::common::u16;
using sr::common::u32;
using sr::common::u64;
using sr::common::s32;
using sr::common::s64;

// Forward declarations
struct SrMemory;

// AArch64 CPU state as seen by generated code
// Matches the layout expected by the codegen backend
struct SrCpuState {
    // General purpose registers X0-X30 (X31 is SP, handled separately)
    u64 x[31];

    // Stack pointer (X31)
    u64 sp;

    // Program counter
    u64 pc;

    // PSTATE flags (NZCV): bit31=N, bit30=Z, bit29=C, bit28=V
    u32 pstate;

    // System registers (optional, for future expansion)
    u64 tpidr_el0;   // Thread pointer
    u64 sp_el0;      // EL0 stack pointer
    u64 elr_el1;     // Exception link register
    u64 spsr_el1;    // Saved program status register
};

// Memory abstraction for generated code
struct SrMemory {
    // Base address of memory region
    void* base;

    // Size of memory region in bytes
    u64 size;

    // Optional: page table or memory map for virtual-to-physical translation
    void* page_table;

    // Read memory at guest address
    // Returns true on success, false on fault
    bool (*read)(SrMemory* mem, u64 guest_addr, void* out, u64 size);

    // Write memory at guest address
    // Returns true on success, false on fault
    bool (*write)(SrMemory* mem, u64 guest_addr, const void* src, u64 size);
};

// Runtime helper functions (implemented in runtime.cpp)

// Load 64-bit value from guest memory
u64 sr_ldr64(SrCpuState* cpu, SrMemory* mem, u64 addr);

// Store 64-bit value to guest memory
void sr_str64(SrCpuState* cpu, SrMemory* mem, u64 addr, u64 value);

// Call a guest function at the given address
void sr_rt_call(SrCpuState* cpu, u64 target);

// Handle SVC (supervisor call)
void sr_rt_handle_svc(SrCpuState* cpu, u32 svc_id);

// Convert guest address to host pointer
// Returns nullptr on fault
void* sr_rt_guest_to_host(SrCpuState* cpu, u64 addr, u64 size, int write);

// Initialize CPU state
void sr_cpu_init(SrCpuState* cpu);

// Initialize memory
bool sr_memory_init(SrMemory* mem, void* base, u64 size);

// Reset CPU state to initial values
void sr_cpu_reset(SrCpuState* cpu);

// Execute a function by address
// Returns the return value in X0
u64 sr_cpu_execute(SrCpuState* cpu, SrMemory* mem, u64 entry_point);

} // namespace kr