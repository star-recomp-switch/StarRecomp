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
    ctx->x[0] = 0;
    glabel1:
    ctx->x[1] = 0;
    glabel2:
    sr_v_set_d(&ctx->v[0], (double)(int64_t)ctx->x[0]);
    glabel3:
    sr_v_set_d(&ctx->v[1], (double)(int64_t)ctx->x[1]);
    glabel4:
    sr_v_set_d(&ctx->v[0], sr_v_get_d(&ctx->v[0]) / sr_v_get_d(&ctx->v[1]));
    glabel5:
    ctx->x[0] = (uint64_t)(isnan(sr_v_get_d(&ctx->v[0])) ? (int64_t)0 : (sr_v_get_d(&ctx->v[0]) >= 0x1.0p63 ? (int64_t)9223372036854775807LL : (sr_v_get_d(&ctx->v[0]) <= -0x1.0p63 ? (-9223372036854775807LL - 1) : (int64_t)sr_v_get_d(&ctx->v[0]))));
    glabel6:
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
