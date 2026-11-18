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
    sr_v_set_s(&ctx->v[0], (float)3);
    glabel1:
    for (int sr_i = 0; sr_i < 4; ++sr_i) sr_v_set_lane_s(&ctx->v[1], sr_i, sr_v_get_s(&ctx->v[0]));
    glabel2:
    sr_v_set_s(&ctx->v[2], (float)7);
    glabel3:
    sr_v_set_lane_s(&ctx->v[1], 2, sr_v_get_s(&ctx->v[2]));
    glabel4:
    { union { float f; uint32_t u; } sr_u3; sr_u3.f = sr_v_get_lane_s(&ctx->v[1], 2); ctx->x[3] = sr_u3.u; }
    glabel5:
    return;
}


#include <stdio.h>
#include <string.h>
#include <stdint.h>
int main(void) {
    GuestContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    fn_0(&ctx);
    for (int i = 0; i < 4; ++i)
        printf("lane%d = %.6f\n", i, sr_v_get_lane_s(&ctx.v[1], i));
    union { uint32_t u; float f; } cvt;
    cvt.u = (uint32_t)ctx.x[3];
    printf("w3 = 0x%08X\n", (unsigned)cvt.u);
    printf("w3f = %.6f\n", cvt.f);
    return 0;
}
