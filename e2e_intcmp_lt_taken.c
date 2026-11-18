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

void fn_guest_0(GuestContext* ctx) {
bb_0:
    glabel0:
    ctx->x[0] = 1;
    glabel1:
    ctx->x[1] = 2;
    glabel2:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[0]) < (int64_t)(ctx->x[1])) << 31) | ((uint32_t)((uint64_t)(ctx->x[0]) == (uint64_t)(ctx->x[1])) << 30) | ((uint32_t)((uint64_t)(ctx->x[0]) >= (uint64_t)(ctx->x[1])) << 29) | ((uint32_t)((((uint64_t)(ctx->x[0]) ^ (uint64_t)(ctx->x[1])) & ((uint64_t)(ctx->x[0]) ^ ((uint64_t)(ctx->x[0]) - (uint64_t)(ctx->x[1])))) >> 63) << 28));
    glabel3:
    if ((((ctx->nzcv >> 31) & 1u) != ((ctx->nzcv >> 28) & 1u))) goto glabel0;
bb_1:
    glabel4:
    ctx->x[0] = 8;
    glabel5:
    goto glabel1;
bb_2:
    glabel6:
    ctx->x[0] = 7;
    glabel7:
    return;
}


#include <stdio.h>
#include <string.h>
int main() {
    GuestContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    fn_0(&ctx);
    printf("result = %lld\n", (long long)(int64_t)ctx.x[0]);
    return 0;
}
