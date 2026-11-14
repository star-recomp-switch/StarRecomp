#include "kirbyrecomp/runtime.h"
#include "svc_thread.cpp"  // Include the thread SVC implementation

#include <cstring>

namespace kr {

using sr::common::u8;

u64 sr_ldr64(SrCpuState* cpu, SrMemory* mem, u64 addr) {
    u64 value = 0;
    if (mem->read) {
        mem->read(mem, addr, &value, 8);
    } else if (mem->base) {
        // Flat memory model: guest address = host address offset
        u8* ptr = static_cast<u8*>(mem->base) + addr;
        if (addr + 8 <= mem->size) {
            std::memcpy(&value, ptr, 8);
        }
    }
    return value;
}

void sr_str64(SrCpuState* cpu, SrMemory* mem, u64 addr, u64 value) {
    if (mem->write) {
        mem->write(mem, addr, &value, 8);
    } else if (mem->base) {
        // Flat memory model: guest address = host address offset
        u8* ptr = static_cast<u8*>(mem->base) + addr;
        if (addr + 8 <= mem->size) {
            std::memcpy(ptr, &value, 8);
        }
    }
}

void sr_rt_call(SrCpuState* cpu, u64 target) {
    // Save return address (LR = PC + 4)
    cpu->x[30] = cpu->pc + 4;

    // Jump to target
    cpu->pc = target;

    // Note: In a real implementation, this would invoke the generated function
    // For now, this is a stub
    (void)cpu;
    (void)target;
}

void sr_rt_handle_svc(SrCpuState* cpu, u32 svc_id) {
    // Handle supervisor calls
    // First try thread SVCs
    if (sr_rt_dispatch_thread_svc(cpu, svc_id)) {
        return;
    }

    // SVC #0: exit/return from guest code
    // Other SVCs: system calls
    (void)cpu;
    (void)svc_id;
}

void* sr_rt_guest_to_host(SrCpuState* cpu, u64 addr, u64 size, int write) {
    (void)cpu;
    (void)size;
    (void)write;

    // Simple flat memory model
    // In a real implementation, this would do page table walks
    return nullptr;
}

void sr_cpu_init(SrCpuState* cpu) {
    std::memset(cpu, 0, sizeof(SrCpuState));
    cpu->pstate = 0; // NZCV = 0
}

bool sr_memory_init(SrMemory* mem, void* base, u64 size) {
    if (!mem || !base || size == 0) {
        return false;
    }
    mem->base = base;
    mem->size = size;
    mem->page_table = nullptr;
    mem->read = nullptr;
    mem->write = nullptr;
    return true;
}

void sr_cpu_reset(SrCpuState* cpu) {
    sr_cpu_init(cpu);
}

u64 sr_cpu_execute(SrCpuState* cpu, SrMemory* mem, u64 entry_point) {
    cpu->pc = entry_point;
    // In a real implementation, this would jump to the generated code
    // For now, return 0
    (void)mem;
    return 0;
}

// Dispatch thread-related SVCs
bool sr_rt_dispatch_thread_svc(SrCpuState* cpu, u32 svc_id) {
    return kr::dispatch_thread_svc(cpu, svc_id);
}

} // namespace kr