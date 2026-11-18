#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
#include "switchrecomp/runtime/runtime.h"
using sr::runtime::GuestContext;
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
uint64_t sr_rt_load64(GuestContext* ctx, uint64_t addr);
#ifdef __cplusplus
}
#endif

void fn_guest_100(GuestContext* ctx) {
bb_0:
    glabel0:
    sr_rt_call(ctx, 512);
bb_1:
    glabel1:
    return;
}

void fn_guest_200(GuestContext* ctx) {
bb_0:
    glabel0:
    ctx->x[0] = 42;
    glabel1:
    return;
}


void register_functions() {
    sr_rt_register_function(0x100, fn_100);
    sr_rt_register_function(0x200, fn_200);
}

#include <stdio.h>
int main() {
    GuestContext ctx;
    for (int i = 0; i < 31; i++) ctx.x[i] = 0;
    ctx.sp = 0; ctx.pc = 0; ctx.nzcv = 0;
    register_functions();
    fn_100(&ctx);
    printf("x0 = %llu\n", (unsigned long long)ctx.x[0]);
    return 0;
}
