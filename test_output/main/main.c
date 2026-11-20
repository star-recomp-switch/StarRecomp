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
#ifdef __cplusplus
}
#endif

void fn_806ef15c1404(GuestContext* ctx) {
bb_0:
    glabel0:
    ctx->x[0] = ctx->x[0] + 1;
    glabel1:
    return;
bb_1:
    glabel2:
    /* nop */
    glabel3:
    /* nop */
    glabel4:
    /* nop */
    glabel5:
    /* nop */
    glabel6:
    /* nop */
    glabel7:
    /* nop */
}

