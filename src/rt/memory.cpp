#include "kirbyrecomp/runtime.h"

#include <cstdlib>
#include <cstring>

namespace kr {

using sr::common::u8;

// Default memory read implementation for flat memory model
bool sr_memory_default_read(SrMemory* mem, u64 guest_addr, void* out, u64 size) {
    if (!mem || !mem->base || guest_addr + size > mem->size) {
        return false;
    }
    u8* ptr = static_cast<u8*>(mem->base) + guest_addr;
    std::memcpy(out, ptr, static_cast<size_t>(size));
    return true;
}

// Default memory write implementation for flat memory model
bool sr_memory_default_write(SrMemory* mem, u64 guest_addr, const void* src, u64 size) {
    if (!mem || !mem->base || guest_addr + size > mem->size) {
        return false;
    }
    u8* ptr = static_cast<u8*>(mem->base) + guest_addr;
    std::memcpy(ptr, src, static_cast<size_t>(size));
    return true;
}

// Allocate and initialize a flat memory region
bool sr_memory_create_flat(SrMemory* mem, u64 size) {
    if (!mem || size == 0) {
        return false;
    }

    // Use _aligned_malloc on Windows, aligned_alloc on POSIX
#if defined(_WIN32)
    void* base = _aligned_malloc(static_cast<size_t>((size + 4095) & ~4095ULL), 4096);
#else
    void* base = std::aligned_alloc(4096, static_cast<size_t>((size + 4095) & ~4095ULL));
#endif
    if (!base) {
        return false;
    }

    std::memset(base, 0, static_cast<size_t>(size));

    mem->base = base;
    mem->size = size;
    mem->page_table = nullptr;
    mem->read = sr_memory_default_read;
    mem->write = sr_memory_default_write;

    return true;
}

// Destroy a flat memory region
void sr_memory_destroy_flat(SrMemory* mem) {
    if (mem && mem->base) {
        std::free(mem->base);
        mem->base = nullptr;
        mem->size = 0;
        mem->read = nullptr;
        mem->write = nullptr;
    }
}

} // namespace kr