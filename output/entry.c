#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
#include "switchrecomp/runtime/runtime.h"
#include "switchrecomp/runtime/vreg.h"
using sr::runtime::GuestContext;
using sr::runtime::MemProtect;
#else
#include "switchrecomp/runtime/vreg.h"
typedef struct GuestContext {
    uint64_t x[31];
    uint64_t sp;
    uint64_t pc;
    uint32_t nzcv;
    sr_vreg v[32];
    uint32_t fpcr;
    uint32_t fpsr;
    uint64_t shared_counter;
    uint64_t thread_handle;
} GuestContext;
#endif

#ifdef __cplusplus
extern "C" {
#endif
void* sr_rt_guest_to_host(GuestContext* ctx, uint64_t addr, uint64_t size, int write);
void  sr_rt_call(GuestContext* ctx, uint64_t target);
void  sr_rt_handle_svc(GuestContext* ctx, uint32_t svc_id);
void  sr_rt_register_function(uint64_t guest_addr, void(*fn)(GuestContext*));
bool  sr_rt_map_memory(uint64_t guest_addr, uint64_t size, uint32_t prot);
#ifdef __cplusplus
}
#endif

// Forward declare the generated function
void fn_7100574af9(GuestContext* ctx);

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
#include "switchrecomp/runtime/runtime.h"
#include "switchrecomp/runtime/vreg.h"
using sr::runtime::GuestContext;
using sr::runtime::MemProtect;
#else
#include "switchrecomp/runtime/vreg.h"
typedef struct GuestContext {
    uint64_t x[31];
    uint64_t sp;
    uint64_t pc;
    uint32_t nzcv;
    sr_vreg v[32];
    uint32_t fpcr;
    uint32_t fpsr;
    uint64_t shared_counter;
    uint64_t thread_handle;
} GuestContext;
#endif

#ifdef __cplusplus
extern "C" {
#endif
void* sr_rt_guest_to_host(GuestContext* ctx, uint64_t addr, uint64_t size, int write);
void  sr_rt_call(GuestContext* ctx, uint64_t target);
void  sr_rt_handle_svc(GuestContext* ctx, uint32_t svc_id);
void  sr_rt_register_function(uint64_t guest_addr, void(*fn)(GuestContext*));
bool  sr_rt_map_memory(uint64_t guest_addr, uint64_t size, uint32_t prot);
#ifdef __cplusplus
}
#endif

// Forward declare the generated function
void fn_7100574af9(GuestContext* ctx);

int main(int argc, char** argv) {
    printf("KirbyRecomp - Starting recompiled game...\n");
    fflush(stdout);

    // Map guest memory regions before running
    // NSO base: 0x7100000000
    // Main NSO text: 0x7100000000 + 0x100 = 0x7100000100 (8.4MB)
    // Map a large region covering text + rodata + data + bss
    printf("Mapping guest memory regions...\n");
    fflush(stdout);
    sr_rt_map_memory(0x7100000000ULL, 0x10000000ULL, 7);  // 256MB, RWX

    // Map zero page (needed for code that dereferences null pointers)
    printf("Mapping zero page...\n");
    fflush(stdout);
    sr_rt_map_memory(0x0ULL, 0x1000ULL, 7);  // 4KB, RWX

    // Write a non-zero value at address 0x0 to break spin loops
    // The code seems to be polling address 0, so give it a non-zero initial value
    // Use the guest_to_host function to get the host pointer
    // We can't call sr_rt_guest_to_host directly from C without context,
    // so let's just map and rely on the runtime's zero-initialization
    // Actually, let's try writing directly via the runtime's internal mechanism
    // For now, the runtime maps pages with zero-initialization

    // Initialize guest context
    GuestContext ctx = {0};
    ctx.pc = 0x7100574AF9;  // Entry point address
    ctx.sp = 0x7FFFFFFF00;  // Stack pointer near top of 2GB

    // Set up thread pointer (TPIDR_EL0) - typically points to thread-local storage
    // On Switch, this is often at a fixed offset or in the 0x0 region
    // We'll leave it as 0 for now

    // Register the entry point function
    sr_rt_register_function(0x7100574AF9, fn_7100574af9);

    // Call the entry point
    printf("Calling entry point at 0x7100574AF9...\n");
    fflush(stdout);
    fn_7100574af9(&ctx);

    printf("Entry point returned\n");
    return 0;
}