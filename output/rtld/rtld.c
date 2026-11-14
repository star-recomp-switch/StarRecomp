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

void fn_7100001c13(GuestContext* ctx) {
    uint64_t v241 = 0;
    uint64_t v301 = 0;
    uint64_t v305 = 0;
    uint64_t v312 = 0;
    uint64_t v331 = 0;
    uint64_t v358 = 0;
    uint64_t v362 = 0;
    uint64_t v369 = 0;
    uint64_t v388 = 0;
    uint64_t v419 = 0;
    uint64_t v423 = 0;
    uint64_t v430 = 0;
    uint64_t v441 = 0;
    uint64_t v530 = 0;
    uint64_t v682 = 0;
    uint64_t v696 = 0;
    uint64_t v710 = 0;
    uint64_t v724 = 0;
    uint64_t v742 = 0;
    uint64_t v768 = 0;
    uint64_t v808 = 0;
    uint64_t v814 = 0;
    uint64_t v897 = 0;
    uint64_t v906 = 0;
    uint64_t v916 = 0;
    uint64_t v928 = 0;
    uint64_t v992 = 0;
    uint64_t v1000 = 0;
    uint64_t v1018 = 0;
    uint64_t v1030 = 0;
    uint64_t v1043 = 0;
    uint64_t v1055 = 0;
    uint64_t v1062 = 0;
    uint64_t v1064 = 0;
    uint64_t v1074 = 0;
    uint64_t v1078 = 0;
    uint64_t v1090 = 0;
    uint64_t v1093 = 0;
    uint64_t v1101 = 0;
    uint64_t v1104 = 0;
    uint64_t v1127 = 0;
    uint64_t v1130 = 0;
    uint64_t v1138 = 0;
    uint64_t v1142 = 0;
    uint64_t v1150 = 0;
    uint64_t v1157 = 0;
    uint64_t v1161 = 0;
    uint64_t v1169 = 0;
    uint64_t v1173 = 0;
    uint64_t v1181 = 0;
    uint64_t v1185 = 0;
    uint64_t v1193 = 0;
    uint64_t v1196 = 0;
    uint64_t v1204 = 0;
    uint64_t v1208 = 0;
    uint64_t v1216 = 0;
    uint64_t v1220 = 0;
    uint64_t v1228 = 0;
    uint64_t v1231 = 0;
    uint64_t v1251 = 0;
    uint64_t v1259 = 0;
    uint64_t v1262 = 0;
    uint64_t v1270 = 0;
    uint64_t v1273 = 0;
    uint64_t v1288 = 0;
    uint64_t v1318 = 0;
    uint64_t v1321 = 0;
    uint64_t v1323 = 0;
    uint64_t v1331 = 0;
    uint64_t v1333 = 0;
    uint64_t v1339 = 0;
    uint64_t v1344 = 0;
    uint64_t v1348 = 0;
    uint64_t v1394 = 0;
    uint64_t v1398 = 0;
    uint64_t v1401 = 0;
    uint64_t v1403 = 0;
    uint64_t v1405 = 0;
    uint64_t v1447 = 0;
    uint64_t v1472 = 0;
    uint64_t v1482 = 0;
    uint64_t v1493 = 0;
    uint64_t v1500 = 0;
    uint64_t v1504 = 0;
    uint64_t v1512 = 0;
    uint64_t v1565 = 0;
    uint64_t v1576 = 0;
    uint64_t v1599 = 0;
    uint64_t v1628 = 0;
    uint64_t v1631 = 0;
    uint64_t v1633 = 0;
    uint64_t v1704 = 0;
    uint64_t v1707 = 0;
    uint64_t v1725 = 0;
    uint64_t v1728 = 0;
    uint64_t v1794 = 0;
    uint64_t v1816 = 0;
    uint64_t v1819 = 0;
    uint64_t v1821 = 0;
    uint64_t v1911 = 0;
    uint64_t v1949 = 0;
    uint64_t v1955 = 0;
    uint64_t v1961 = 0;
    uint64_t v1964 = 0;
    uint64_t v1975 = 0;
    uint64_t v1987 = 0;
    uint64_t v1989 = 0;
    uint64_t v2000 = 0;
    uint64_t v2002 = 0;
    uint64_t v2009 = 0;
    uint64_t v2012 = 0;
    uint64_t v2031 = 0;
    uint64_t v2043 = 0;
    uint64_t v2095 = 0;
    uint64_t v2097 = 0;
    uint64_t v2114 = 0;
    uint64_t v2117 = 0;
    uint64_t v2120 = 0;
    uint64_t v2143 = 0;
    uint64_t v2150 = 0;
    uint64_t v2152 = 0;
    uint64_t v2158 = 0;
    uint64_t v2168 = 0;
    uint64_t v2174 = 0;
    uint64_t v2219 = 0;
    uint64_t v2228 = 0;
    uint64_t v2231 = 0;
    uint64_t v2241 = 0;
bb_0:
    glabel0:
    goto glabel1;
bb_1:
    glabel1:
    /* unimplemented opcode */
    glabel2:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[0]) < (int64_t)(0)) << 31) | ((uint32_t)((uint64_t)(ctx->x[0]) == (uint64_t)(0)) << 30) | ((uint32_t)((uint64_t)(ctx->x[0]) >= (uint64_t)(0)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[0]) ^ (uint64_t)(0)) & ((uint64_t)(ctx->x[0]) ^ ((uint64_t)(ctx->x[0]) - (uint64_t)(0)))) >> 63) << 28));
    glabel3:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_2:
    glabel4:
    ctx->x[0] = 31 | ctx->x[1];
    glabel5:
    sr_rt_call(ctx, 485331311663);
bb_3:
    glabel6:
    /* unimplemented opcode */
    glabel7:
    ctx->x[30] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[30], 8, 0);
    glabel8:
    ctx->x[0] = (int64_t)ctx->x[0] >> 0;
    glabel9:
    ctx->x[0] = ctx->x[0] + ctx->x[30];
    glabel10:
    sr_rt_call(ctx, 485331311683);
bb_4:
    glabel11:
    /* unimplemented opcode */
    glabel12:
    ctx->x[30] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[30], 8, 0);
    glabel13:
    ctx->x[2] = (int64_t)ctx->x[2] >> 0;
    glabel14:
    ctx->x[2] = ctx->x[2] + ctx->x[30];
    glabel15:
    ctx->x[2] = ctx->x[2] - ctx->x[0];
    glabel16:
    ctx->x[1] = 0;
    glabel17:
    sr_rt_call(ctx, 485331317475);
bb_5:
    glabel18:
    sr_rt_call(ctx, 485331311715);
bb_6:
    glabel19:
    /* unimplemented opcode */
    glabel20:
    ctx->x[30] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[30], 8, 0);
    glabel21:
    ctx->x[0] = (int64_t)ctx->x[0] >> 0;
    glabel22:
    ctx->x[0] = ctx->x[0] + ctx->x[30];
    glabel23:
    sr_rt_call(ctx, 485331311735);
bb_7:
    glabel24:
    /* unimplemented opcode */
    glabel25:
    ctx->x[30] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[30], 8, 0);
    glabel26:
    ctx->x[1] = (int64_t)ctx->x[1] >> 0;
    glabel27:
    ctx->x[1] = ctx->x[1] + ctx->x[30];
    glabel28:
    ctx->x[0] = 31 | ctx->x[0];
    glabel29:
    ctx->x[0] = 31 | ctx->x[1];
    glabel30:
    sr_rt_call(ctx, 485331316915);
bb_8:
    glabel31:
    ctx->x[0] = 31 | ctx->x[20];
    glabel32:
    ctx->x[0] = 31 | ctx->x[21];
    glabel33:
    sr_rt_call(ctx, 485331312835);
bb_9:
    glabel34:
    ctx->x[0] = 31 | ctx->x[19];
    glabel35:
    ctx->x[0] = 485331308544 + 12288;
    glabel36:
    ctx->x[1] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[1], 8, 0);
    glabel37:
    sr_rt_call(ctx, 485331311791);
bb_10:
    glabel38:
    /* unimplemented opcode */
    glabel39:
    ctx->x[30] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[30], 8, 0);
    glabel40:
    ctx->x[2] = (int64_t)ctx->x[2] >> 0;
    glabel41:
    ctx->x[2] = ctx->x[2] + ctx->x[30];
    glabel42:
    sr_rt_call(ctx, 485331311811);
bb_11:
    glabel43:
    /* unimplemented opcode */
    glabel44:
    ctx->x[30] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[30], 8, 0);
    glabel45:
    ctx->x[3] = (int64_t)ctx->x[3] >> 0;
    glabel46:
    ctx->x[3] = ctx->x[3] + ctx->x[30];
    glabel47:
    sr_rt_call(ctx, 485331311831);
bb_12:
    glabel48:
    /* unimplemented opcode */
    glabel49:
    ctx->x[30] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[30], 8, 0);
    glabel50:
    ctx->x[4] = (int64_t)ctx->x[4] >> 0;
    glabel51:
    ctx->x[4] = ctx->x[4] + ctx->x[30];
    glabel52:
    sr_rt_call(ctx, 485331317843);
bb_13:
    glabel53:
    goto glabel0;
bb_14:
    glabel54:
    sr_rt_call(ctx, 485331311859);
bb_15:
    glabel55:
    /* unimplemented opcode */
    glabel56:
    ctx->x[30] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[30], 8, 0);
    glabel57:
    ctx->x[2] = (int64_t)ctx->x[2] >> 0;
    glabel58:
    ctx->x[2] = ctx->x[2] + ctx->x[30];
    glabel59:
    ctx->x[2] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[2], 8, 0);
    glabel60:
    if (ctx->x[2]) goto bb_1; else goto bb_0;
bb_16:
    glabel61:
    ctx->x[0] = 485331308544 + 12288;
    glabel62:
    ctx->x[2] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[2], 8, 0);
    glabel63:
    if (ctx->x[2]) goto bb_1; else goto bb_0;
bb_17:
    glabel64:
    /* unimplemented opcode */
    glabel65:
    ctx->x[0] = 63489;
    glabel66:
    sr_rt_call(ctx, 485331317199);
bb_18:
    glabel67:
    goto glabel0;
bb_19:
    glabel68:
    ctx->x[0] = 31 - 144;
    glabel69:
    /* unimplemented opcode */
    glabel70:
    /* unimplemented opcode */
    glabel71:
    ctx->x[0] = 31 + 128;
    glabel72:
    ctx->x[0] = 485331308544 + 12288;
    glabel73:
    ctx->x[0] = 485331308544 + 12288;
    glabel74:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel75:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel76:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_20:
    glabel77:
    ctx->x[0] = 485331308544 + 12288;
    glabel78:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel79:
    ctx->x[0] = 485331308544 + 12288;
    glabel80:
    ctx->x[0] = 485331308544 + 12288;
    glabel81:
    ctx->x[9] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 0);
    glabel82:
    ctx->x[0] = 485331308544 + 12288;
    glabel83:
    ctx->x[10] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[10], 8, 0);
    glabel84:
    ctx->x[0] = 485331308544 + 12288;
    glabel85:
    ctx->x[0] = 485331308544 + 12288;
    glabel86:
    ctx->x[11] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[11], 8, 0);
    glabel87:
    ctx->x[0] = 485331308544 + 12288;
    glabel88:
    ctx->x[0] = 485331308544 + 12288;
    glabel89:
    ctx->x[12] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[12], 8, 0);
    glabel90:
    ctx->x[0] = 485331308544 + 12288;
    glabel91:
    ctx->x[0] = 485331308544 + 12288;
    glabel92:
    ctx->x[13] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[13], 8, 0);
    glabel93:
    ctx->x[0] = 485331308544 + 12288;
    glabel94:
    ctx->x[0] = 485331308544 + 12288;
    glabel95:
    ctx->x[14] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[14], 8, 0);
    glabel96:
    ctx->x[0] = 485331308544 + 12288;
    glabel97:
    ctx->x[0] = 485331308544 + 12288;
    glabel98:
    ctx->x[15] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[15], 8, 0);
    glabel99:
    ctx->x[0] = 485331308544 + 12288;
    glabel100:
    ctx->x[0] = 485331308544 + 12288;
    glabel101:
    ctx->x[16] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[16], 8, 0);
    glabel102:
    *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 1) = ctx->x[8];
    glabel103:
    ctx->x[0] = 485331308544 + 12288;
    glabel104:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel105:
    ctx->x[0] = 485331308544 + 12288;
    glabel106:
    ctx->x[0] = 485331308544 + 12288;
    glabel107:
    ctx->x[17] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[17], 8, 0);
    glabel108:
    /* unimplemented opcode */
    glabel109:
    ctx->x[0] = 485331308544 + 12288;
    glabel110:
    ctx->x[0] = 485331308544 + 12288;
    glabel111:
    ctx->x[9] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 0);
    glabel112:
    ctx->x[10] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[10], 8, 0);
    glabel113:
    /* unimplemented opcode */
    glabel114:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel115:
    ctx->x[1] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[1], 8, 0);
    glabel116:
    /* unimplemented opcode */
    glabel117:
    ctx->x[2] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[2], 8, 0);
    glabel118:
    ctx->x[3] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[3], 8, 0);
    glabel119:
    /* unimplemented opcode */
    glabel120:
    ctx->x[4] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[4], 8, 0);
    glabel121:
    ctx->x[5] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[5], 8, 0);
    glabel122:
    /* unimplemented opcode */
    glabel123:
    ctx->x[6] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[6], 8, 0);
    glabel124:
    ctx->x[7] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[7], 8, 0);
    glabel125:
    /* unimplemented opcode */
    glabel126:
    sr_rt_call(ctx, 485331317155);
bb_21:
    glabel127:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 1) = ctx->x[0];
    glabel128:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel129:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(1)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(1)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(1)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(1)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(1)))) >> 31) << 28));
    glabel130:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_22:
    glabel131:
    ctx->x[0] = 485331308544 + 12288;
    glabel132:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel133:
    /* unimplemented opcode */
    glabel134:
    if (((ctx->nzcv >> 29) & 1u)) goto glabel0;
bb_23:
    glabel135:
    ctx->x[0] = 485331308544 + 12288;
    glabel136:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel137:
    ctx->x[20] = ctx->x[20] + ctx->x[0];
    glabel138:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v241, 8, 0);
    glabel139:
    /* unimplemented opcode */
    glabel140:
    /* unimplemented opcode */
    glabel141:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel2;
bb_24:
    glabel142:
    /* unimplemented opcode */
    glabel143:
    /* unimplemented opcode */
    glabel144:
    ctx->x[0] = 31 + 144;
    glabel145:
    return;
bb_25:
    glabel146:
    /* unimplemented opcode */
    glabel147:
    /* unimplemented opcode */
    glabel148:
    /* unimplemented opcode */
    glabel149:
    ctx->x[0] = 485331308544 + 12288;
    glabel150:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel151:
    ctx->x[0] = 485331308544 + 12288;
    glabel152:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel153:
    ctx->x[0] = 31 | 31;
    glabel154:
    goto glabel3;
bb_26:
    glabel155:
    /* unimplemented opcode */
    glabel156:
    /* unimplemented opcode */
    glabel157:
    /* unimplemented opcode */
    glabel158:
    ctx->x[0] = 31 + 0;
    glabel159:
    ctx->x[0] = 485331308544 + 12288;
    glabel160:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel161:
    ctx->x[0] = 31 | 31;
    glabel162:
    sr_rt_call(ctx, 485331312611);
bb_27:
    glabel163:
    ctx->x[0] = 31 | 31;
    glabel164:
    /* unimplemented opcode */
    glabel165:
    return;
bb_28:
    glabel166:
    /* unimplemented opcode */
    glabel167:
    /* unimplemented opcode */
    glabel168:
    /* unimplemented opcode */
    glabel169:
    ctx->x[0] = 31 | ctx->x[0];
    glabel170:
    ctx->x[0] = ~0;
    glabel171:
    if (ctx->x[2]) goto bb_1; else goto bb_0;
bb_29:
    glabel172:
    ctx->x[0] = 485331308544 + 12288;
    glabel173:
    ctx->x[0] = 485331308544 + 12288;
    glabel174:
    ctx->x[9] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 0);
    glabel175:
    ctx->x[10] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[10], 8, 0);
    glabel176:
    ctx->x[0] = 485331308544 + 12288;
    glabel177:
    /* unimplemented opcode */
    glabel178:
    ctx->x[10] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[10], 8, 0);
    glabel179:
    ctx->x[12] = ctx->x[12] & 18446744073709486080ULL;
    glabel180:
    ctx->x[0] = v301 | 43691;
    glabel181:
    ctx->x[12] = ctx->x[12] & 281474976710655;
    glabel182:
    ctx->x[0] = v305 | 3074269695633784832;
    glabel183:
    ctx->x[10] = ctx->x[10] - ctx->x[9];
    glabel184:
    /* unimplemented opcode */
    glabel185:
    ctx->x[10] = (int64_t)ctx->x[10] >> 2;
    glabel186:
    ctx->x[10] = (uint64_t)ctx->x[10] >> 63;
    glabel187:
    ctx->x[12] = ctx->x[12] + v312;
    glabel188:
    /* unimplemented opcode */
    glabel189:
    if (((ctx->nzcv >> 29) & 1u)) goto glabel6;
bb_30:
    glabel190:
    ctx->x[0] = 485331308544 + 12288;
    glabel191:
    ctx->x[12] = ctx->x[12] + 864;
    glabel192:
    /* unimplemented opcode */
    glabel193:
    ctx->x[11] = ctx->x[11] + 1;
    glabel194:
    /* unimplemented opcode */
    glabel195:
    if (ctx->x[14]) goto bb_1; else goto bb_0;
bb_31:
    glabel196:
    /* unimplemented opcode */
    glabel197:
    if (((ctx->nzcv >> 29) & 1u)) goto glabel0;
bb_32:
    glabel198:
    ctx->x[10] = 24;
    glabel199:
    ctx->x[11] = ctx->x[11] * ctx->x[10];
    glabel200:
    ctx->x[0] = v331 + ctx->x[9];
    glabel201:
    ctx->x[0] = 31 | 31;
    glabel202:
    /* unimplemented opcode */
    glabel203:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 1) = ctx->x[2];
    glabel204:
    return;
bb_33:
    glabel205:
    ctx->x[0] = ~0;
    glabel206:
    return;
bb_34:
    glabel207:
    /* unimplemented opcode */
    glabel208:
    /* unimplemented opcode */
    glabel209:
    ctx->x[0] = 31 | ctx->x[0];
    glabel210:
    ctx->x[0] = ~0;
    glabel211:
    if (ctx->x[2]) goto bb_1; else goto bb_0;
bb_35:
    glabel212:
    ctx->x[0] = 485331308544 + 12288;
    glabel213:
    ctx->x[0] = 485331308544 + 12288;
    glabel214:
    ctx->x[9] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 0);
    glabel215:
    ctx->x[10] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[10], 8, 0);
    glabel216:
    ctx->x[0] = 485331308544 + 12288;
    glabel217:
    /* unimplemented opcode */
    glabel218:
    ctx->x[10] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[10], 8, 0);
    glabel219:
    ctx->x[12] = ctx->x[12] & 18446744073709486080ULL;
    glabel220:
    ctx->x[0] = v358 | 43691;
    glabel221:
    ctx->x[12] = ctx->x[12] & 281474976710655;
    glabel222:
    ctx->x[0] = v362 | 3074269695633784832;
    glabel223:
    ctx->x[10] = ctx->x[10] - ctx->x[9];
    glabel224:
    /* unimplemented opcode */
    glabel225:
    ctx->x[10] = (int64_t)ctx->x[10] >> 2;
    glabel226:
    ctx->x[10] = (uint64_t)ctx->x[10] >> 63;
    glabel227:
    ctx->x[12] = ctx->x[12] + v369;
    glabel228:
    /* unimplemented opcode */
    glabel229:
    if (((ctx->nzcv >> 29) & 1u)) goto glabel6;
bb_36:
    glabel230:
    ctx->x[0] = 485331308544 + 12288;
    glabel231:
    ctx->x[12] = ctx->x[12] + 864;
    glabel232:
    /* unimplemented opcode */
    glabel233:
    ctx->x[11] = ctx->x[11] + 1;
    glabel234:
    /* unimplemented opcode */
    glabel235:
    if (ctx->x[14]) goto bb_1; else goto bb_0;
bb_37:
    glabel236:
    /* unimplemented opcode */
    glabel237:
    if (((ctx->nzcv >> 29) & 1u)) goto glabel0;
bb_38:
    glabel238:
    ctx->x[10] = 24;
    glabel239:
    ctx->x[11] = ctx->x[11] * ctx->x[10];
    glabel240:
    ctx->x[0] = v388 + ctx->x[9];
    glabel241:
    ctx->x[0] = 31 | 31;
    glabel242:
    /* unimplemented opcode */
    glabel243:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 1) = ctx->x[2];
    glabel244:
    return;
bb_39:
    glabel245:
    ctx->x[0] = ~0;
    glabel246:
    return;
bb_40:
    glabel247:
    /* unimplemented opcode */
    glabel248:
    /* unimplemented opcode */
    glabel249:
    goto glabel15;
bb_41:
    glabel250:
    /* unimplemented opcode */
    glabel251:
    /* unimplemented opcode */
    glabel252:
    /* unimplemented opcode */
    glabel253:
    /* unimplemented opcode */
    glabel254:
    /* unimplemented opcode */
    glabel255:
    /* unimplemented opcode */
    glabel256:
    /* unimplemented opcode */
    glabel257:
    ctx->x[0] = 31 + 48;
    glabel258:
    ctx->x[0] = 485331308544 + 12288;
    glabel259:
    ctx->x[0] = 485331308544 + 12288;
    glabel260:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel261:
    ctx->x[9] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 0);
    glabel262:
    /* unimplemented opcode */
    glabel263:
    ctx->x[10] = ctx->x[10] & 18446744073709486080ULL;
    glabel264:
    ctx->x[0] = v419 | 43691;
    glabel265:
    ctx->x[10] = ctx->x[10] & 281474976710655;
    glabel266:
    ctx->x[0] = v423 | 3074269695633784832;
    glabel267:
    ctx->x[9] = ctx->x[9] - ctx->x[8];
    glabel268:
    /* unimplemented opcode */
    glabel269:
    ctx->x[9] = (int64_t)ctx->x[9] >> 2;
    glabel270:
    ctx->x[9] = (uint64_t)ctx->x[9] >> 63;
    glabel271:
    ctx->x[10] = ctx->x[10] + v430;
    glabel272:
    /* unimplemented opcode */
    glabel273:
    /* unimplemented opcode */
    glabel274:
    /* unimplemented opcode */
    glabel275:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_42:
    glabel276:
    ctx->x[9] = 24;
    glabel277:
    ctx->x[22] = ctx->x[22] * ctx->x[9];
    glabel278:
    ctx->x[0] = v441 + ctx->x[8];
    glabel279:
    ctx->x[0] = 31 | ctx->x[1];
    glabel280:
    ctx->x[0] = 31 | ctx->x[0];
    glabel281:
    ctx->x[0] = 485331312640 + 12288;
    glabel282:
    ctx->x[8] = ctx->x[8] - 8;
    glabel283:
    goto glabel0;
bb_43:
    glabel284:
    ctx->x[0] = 31 | ctx->x[20];
    glabel285:
    ctx->x[0] = 31 | ctx->x[21];
    glabel286:
    sr_rt_call(ctx, 485331312611);
bb_44:
    glabel287:
    ctx->x[23] = ctx->x[23] - 24;
    glabel288:
    /* unimplemented opcode */
    glabel289:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_45:
    glabel290:
    ctx->x[24] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[24], 8, 0);
    glabel291:
    ctx->x[22] = ctx->x[22] - 1;
    glabel292:
    ctx->x[0] = 31 | ctx->x[21];
    glabel293:
    ctx->x[23] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[23], 8, 0);
    glabel294:
    /* unimplemented opcode */
    glabel295:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_46:
    glabel296:
    /* unimplemented opcode */
    glabel297:
    /* unimplemented opcode */
    glabel298:
    ctx->x[24] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[24], 8, 0);
    glabel299:
    goto glabel0;
bb_47:
    glabel300:
    /* unimplemented opcode */
    glabel301:
    /* unimplemented opcode */
    glabel302:
    /* unimplemented opcode */
    glabel303:
    /* unimplemented opcode */
    glabel304:
    return;
bb_48:
    glabel305:
    ctx->x[0] = 485331312640 + 12288;
    glabel306:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel307:
    return;
bb_49:
    glabel308:
    /* unimplemented opcode */
    glabel309:
    ctx->x[0] = 485331312640 + 12288;
    glabel310:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel311:
    return;
bb_50:
    glabel312:
    /* unimplemented opcode */
    glabel313:
    ctx->x[0] = 31 - 144;
    glabel314:
    /* unimplemented opcode */
    glabel315:
    /* unimplemented opcode */
    glabel316:
    /* unimplemented opcode */
    glabel317:
    /* unimplemented opcode */
    glabel318:
    /* unimplemented opcode */
    glabel319:
    /* unimplemented opcode */
    glabel320:
    ctx->x[0] = 31 + 128;
    glabel321:
    ctx->x[0] = 485331312640 + 12288;
    glabel322:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel323:
    ctx->x[0] = 31 | ctx->x[0];
    glabel324:
    ctx->x[0] = 485331312640 + 12288;
    glabel325:
    ctx->x[25] = ctx->x[25] + 608;
    glabel326:
    ctx->x[0] = 31 | ctx->x[1];
    glabel327:
    ctx->x[25] = ctx->x[25] + 16;
    glabel328:
    /* unimplemented opcode */
    glabel329:
    ctx->x[0] = 31 | ctx->x[19];
    glabel330:
    ctx->x[0] = 31 | 31;
    glabel331:
    ctx->x[0] = 31 | 31;
    glabel332:
    /* unimplemented opcode */
    glabel333:
    ctx->x[0] = 31 | ctx->x[21];
    glabel334:
    /* unimplemented opcode */
    glabel335:
    sr_rt_call(ctx, 485331314279);
bb_51:
    glabel336:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel337:
    ctx->x[0] = 31 | 31;
    glabel338:
    ctx->x[24] = 20301;
    glabel339:
    ctx->x[24] = ctx->x[24] & 18446744069414649855ULL;
    glabel340:
    ctx->x[0] = v530 | 809762816;
    glabel341:
    ctx->x[25] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[25], 8, 0);
    glabel342:
    ctx->x[0] = 485331312640 + 12288;
    glabel343:
    ctx->x[26] = ctx->x[26] + 624;
    glabel344:
    ctx->x[0] = 485331312640 + 12288;
    glabel345:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 1) = ctx->x[9];
    glabel346:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 1) = ctx->x[22];
    glabel347:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 1) = ctx->x[21];
    glabel348:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[25], 8, 1) = ctx->x[8];
    glabel349:
    goto glabel0;
bb_52:
    glabel350:
    ctx->x[23] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[23], 8, 0);
    glabel351:
    ctx->x[21] = ctx->x[21] + ctx->x[9];
    glabel352:
    ctx->x[9] = ctx->x[9] + ctx->x[8];
    glabel353:
    ctx->x[0] = 31 | ctx->x[23];
    glabel354:
    ctx->x[0] = 31 | ctx->x[21];
    glabel355:
    ctx->x[0] = 31 | 31;
    glabel356:
    ctx->x[0] = 31 | ctx->x[22];
    glabel357:
    ctx->x[0] = 31 | 31;
    glabel358:
    /* unimplemented opcode */
    glabel359:
    sr_rt_call(ctx, 485331314279);
bb_53:
    glabel360:
    ctx->x[0] = 31 | ctx->x[23];
    glabel361:
    sr_rt_call(ctx, 485331314123);
bb_54:
    glabel362:
    ctx->x[23] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[23], 8, 0);
    glabel363:
    ctx->x[26] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[26], 8, 0);
    glabel364:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[23], 8, 1) = ctx->x[9];
    glabel365:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 1) = ctx->x[26];
    glabel366:
    ctx->x[27] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[27], 8, 0);
    glabel367:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 1) = ctx->x[23];
    glabel368:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[27], 8, 1) = ctx->x[8];
    glabel369:
    /* unimplemented opcode */
    glabel370:
    ctx->x[8] = ctx->x[8] + ctx->x[9];
    glabel371:
    /* unimplemented opcode */
    glabel372:
    ctx->x[0] = 31 | ctx->x[8];
    glabel373:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_55:
    glabel374:
    ctx->x[0] = 31 + 8;
    glabel375:
    ctx->x[0] = 31 + 0;
    glabel376:
    ctx->x[0] = 31 | ctx->x[20];
    glabel377:
    sr_rt_call(ctx, 485331317159);
bb_56:
    glabel378:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_57:
    glabel379:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 0);
    glabel380:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(5)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(5)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(5)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(5)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(5)))) >> 31) << 28));
    glabel381:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel7;
bb_58:
    glabel382:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 0);
    glabel383:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(3)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(3)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(3)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(3)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(3)))) >> 31) << 28));
    glabel384:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel7;
bb_59:
    glabel385:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 0);
    glabel386:
    /* unimplemented opcode */
    glabel387:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel7;
bb_60:
    glabel388:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel389:
    ctx->x[21] = ctx->x[21] + ctx->x[8];
    glabel390:
    ctx->x[23] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[23], 8, 0);
    glabel391:
    /* unimplemented opcode */
    glabel392:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_61:
    glabel393:
    /* unimplemented opcode */
    glabel394:
    ctx->x[23] = ctx->x[23] + ctx->x[9];
    glabel395:
    ctx->x[23] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[23], 8, 0);
    glabel396:
    ctx->x[23] = ctx->x[23] + ctx->x[10];
    glabel397:
    ctx->x[23] = ctx->x[23] + ctx->x[9];
    glabel398:
    /* unimplemented opcode */
    glabel399:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_62:
    glabel400:
    ctx->x[0] = 31 | 31;
    glabel401:
    sr_rt_call(ctx, 485331317475);
bb_63:
    glabel402:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel403:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 0);
    glabel404:
    goto glabel0;
bb_64:
    glabel405:
    goto glabel0;
bb_65:
    glabel406:
    goto glabel0;
bb_66:
    glabel407:
    ctx->x[26] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[26], 8, 0);
    glabel408:
    /* unimplemented opcode */
    glabel409:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_67:
    glabel410:
    ctx->x[0] = 485331312640 + 12288;
    glabel411:
    ctx->x[0] = 485331312640 + 12288;
    glabel412:
    ctx->x[28] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[28], 8, 0);
    glabel413:
    ctx->x[27] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[27], 8, 0);
    glabel414:
    ctx->x[0] = 485331312640 + 8192;
    glabel415:
    ctx->x[20] = ctx->x[20] + 1973;
    glabel416:
    ctx->x[0] = 485331312640 + 8192;
    glabel417:
    ctx->x[21] = ctx->x[21] + 2165;
    glabel418:
    ctx->x[0] = 485331312640 + 8192;
    glabel419:
    ctx->x[22] = ctx->x[22] + 2131;
    glabel420:
    ctx->x[0] = 485331312640 + 8192;
    glabel421:
    ctx->x[23] = ctx->x[23] + 2039;
    glabel422:
    ctx->x[0] = 485331312640 + 8192;
    glabel423:
    ctx->x[24] = ctx->x[24] + 2202;
    glabel424:
    ctx->x[0] = 31 | ctx->x[19];
    glabel425:
    ctx->x[0] = 31 | ctx->x[20];
    glabel426:
    sr_rt_call(ctx, 485331313879);
bb_68:
    glabel427:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_69:
    glabel428:
    goto glabel4;
bb_70:
    glabel429:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel430:
    /* unimplemented opcode */
    glabel431:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_71:
    glabel432:
    ctx->x[0] = 31 | ctx->x[19];
    glabel433:
    ctx->x[0] = 31 | ctx->x[20];
    glabel434:
    sr_rt_call(ctx, 485331313879);
bb_72:
    glabel435:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_73:
    glabel436:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel437:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(16)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(16)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(16)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(16)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(16)))) >> 31) << 28));
    glabel438:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel4;
bb_74:
    glabel439:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel440:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel441:
    ctx->x[9] = ctx->x[9] + ctx->x[8];
    glabel442:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v682, 8, 1) = ctx->x[26];
    glabel443:
    ctx->x[0] = 31 | ctx->x[19];
    glabel444:
    ctx->x[0] = 31 | ctx->x[21];
    glabel445:
    sr_rt_call(ctx, 485331313879);
bb_75:
    glabel446:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_76:
    glabel447:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel448:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(16)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(16)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(16)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(16)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(16)))) >> 31) << 28));
    glabel449:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel4;
bb_77:
    glabel450:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel451:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel452:
    ctx->x[9] = ctx->x[9] + ctx->x[8];
    glabel453:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v696, 8, 1) = ctx->x[25];
    glabel454:
    ctx->x[0] = 31 | ctx->x[19];
    glabel455:
    ctx->x[0] = 31 | ctx->x[22];
    glabel456:
    sr_rt_call(ctx, 485331313879);
bb_78:
    glabel457:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_79:
    glabel458:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel459:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(16)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(16)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(16)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(16)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(16)))) >> 31) << 28));
    glabel460:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel4;
bb_80:
    glabel461:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel462:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel463:
    ctx->x[9] = ctx->x[9] + ctx->x[8];
    glabel464:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v710, 8, 1) = ctx->x[27];
    glabel465:
    ctx->x[0] = 31 | ctx->x[19];
    glabel466:
    ctx->x[0] = 31 | ctx->x[23];
    glabel467:
    sr_rt_call(ctx, 485331313879);
bb_81:
    glabel468:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_82:
    glabel469:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel470:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(16)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(16)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(16)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(16)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(16)))) >> 31) << 28));
    glabel471:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel4;
bb_83:
    glabel472:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel473:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel474:
    ctx->x[9] = ctx->x[9] + ctx->x[8];
    glabel475:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v724, 8, 1) = ctx->x[28];
    glabel476:
    ctx->x[0] = 31 | ctx->x[19];
    glabel477:
    ctx->x[0] = 31 | ctx->x[24];
    glabel478:
    sr_rt_call(ctx, 485331313879);
bb_84:
    glabel479:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_85:
    glabel480:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel481:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(16)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(16)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(16)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(16)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(16)))) >> 31) << 28));
    glabel482:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel0;
bb_86:
    glabel483:
    ctx->x[0] = 485331312640 + 12288;
    glabel484:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel485:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel486:
    ctx->x[10] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[10], 8, 0);
    glabel487:
    ctx->x[9] = ctx->x[9] + ctx->x[8];
    glabel488:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v742, 8, 1) = ctx->x[10];
    glabel489:
    goto glabel0;
bb_87:
    glabel490:
    ctx->x[26] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[26], 8, 0);
    glabel491:
    /* unimplemented opcode */
    glabel492:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_88:
    glabel493:
    ctx->x[0] = 31 | ctx->x[19];
    glabel494:
    ctx->x[1] = 1;
    glabel495:
    sr_rt_call(ctx, 485331315727);
bb_89:
    glabel496:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel497:
    /* unimplemented opcode */
    glabel498:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_90:
    glabel499:
    /* unimplemented opcode */
    glabel500:
    /* unimplemented opcode */
    glabel501:
    /* unimplemented opcode */
    glabel502:
    /* unimplemented opcode */
    glabel503:
    /* unimplemented opcode */
    glabel504:
    /* unimplemented opcode */
    glabel505:
    ctx->x[0] = 31 + 144;
    glabel506:
    return;
bb_91:
    glabel507:
    ctx->x[0] = 31 + ctx->x[29];
    glabel508:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v768, 8, 1) = ctx->x[21];
    glabel509:
    /* unimplemented opcode */
    glabel510:
    /* unimplemented opcode */
    glabel511:
    ctx->x[0] = 31 + 32;
    glabel512:
    ctx->x[0] = 485331312640 + 12288;
    glabel513:
    ctx->x[21] = ctx->x[21] + 624;
    glabel514:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel515:
    /* unimplemented opcode */
    glabel516:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_92:
    glabel517:
    ctx->x[0] = 31 | ctx->x[0];
    glabel518:
    ctx->x[0] = 31 | ctx->x[20];
    glabel519:
    ctx->x[0] = 31 | ctx->x[19];
    glabel520:
    sr_rt_call(ctx, 485331313879);
bb_93:
    glabel521:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_94:
    glabel522:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel523:
    /* unimplemented opcode */
    glabel524:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_95:
    glabel525:
    ctx->x[0] = 31 | ctx->x[20];
    glabel526:
    ctx->x[0] = 31 | ctx->x[19];
    glabel527:
    sr_rt_call(ctx, 485331313879);
bb_96:
    glabel528:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_97:
    glabel529:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel530:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(15)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(15)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(15)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(15)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(15)))) >> 31) << 28));
    glabel531:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_98:
    glabel532:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel533:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel534:
    ctx->x[9] = ctx->x[9] + ctx->x[8];
    glabel535:
    /* unimplemented opcode */
    glabel536:
    /* unimplemented opcode */
    glabel537:
    ctx->x[0] = 31 + ctx->x[3];
    glabel538:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v808, 8, 0);
    glabel539:
    return;
bb_99:
    glabel540:
    ctx->x[0] = 31 | 31;
    glabel541:
    /* unimplemented opcode */
    glabel542:
    /* unimplemented opcode */
    glabel543:
    ctx->x[0] = 31 + ctx->x[3];
    glabel544:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v814, 8, 0);
    glabel545:
    return;
bb_100:
    glabel546:
    /* unimplemented opcode */
    glabel547:
    /* unimplemented opcode */
    glabel548:
    ctx->x[0] = 31 + 16;
    glabel549:
    ctx->x[0] = 485331312640 + 12288;
    glabel550:
    ctx->x[8] = ctx->x[8] + 624;
    glabel551:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel552:
    /* unimplemented opcode */
    glabel553:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_101:
    glabel554:
    ctx->x[0] = 485331312640 + 12288;
    glabel555:
    ctx->x[0] = 485331312640 + 12288;
    glabel556:
    ctx->x[20] = ctx->x[20] + 624;
    glabel557:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel558:
    sr_rt_call(ctx, 485331314995);
bb_102:
    glabel559:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel560:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel561:
    /* unimplemented opcode */
    glabel562:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel3;
bb_103:
    glabel563:
    /* unimplemented opcode */
    glabel564:
    /* unimplemented opcode */
    glabel565:
    return;
bb_104:
    glabel566:
    /* unimplemented opcode */
    glabel567:
    /* unimplemented opcode */
    glabel568:
    ctx->x[0] = 31 + 16;
    glabel569:
    ctx->x[0] = 485331312640 + 12288;
    glabel570:
    ctx->x[20] = ctx->x[20] + 624;
    glabel571:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel572:
    /* unimplemented opcode */
    glabel573:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_105:
    glabel574:
    ctx->x[0] = 31 | ctx->x[19];
    glabel575:
    sr_rt_call(ctx, 485331315011);
bb_106:
    glabel576:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel577:
    /* unimplemented opcode */
    glabel578:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_107:
    glabel579:
    /* unimplemented opcode */
    glabel580:
    /* unimplemented opcode */
    glabel581:
    return;
bb_108:
    glabel582:
    goto glabel0;
bb_109:
    glabel583:
    /* unimplemented opcode */
    glabel584:
    /* unimplemented opcode */
    glabel585:
    /* unimplemented opcode */
    glabel586:
    /* unimplemented opcode */
    glabel587:
    ctx->x[0] = 31 + 48;
    glabel588:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel589:
    ctx->x[0] = 31 | ctx->x[1];
    glabel590:
    ctx->x[0] = 31 | ctx->x[0];
    glabel591:
    ctx->x[1] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[1], 8, 0);
    glabel592:
    if (ctx->x[11]) goto bb_1; else goto bb_0;
bb_110:
    glabel593:
    ctx->x[0] = 31 | 31;
    glabel594:
    ctx->x[19] = ctx->x[19] + 1;
    glabel595:
    ctx->x[9] = (uint64_t)ctx->x[9] >> 28;
    glabel596:
    /* unimplemented opcode */
    glabel597:
    /* unimplemented opcode */
    glabel598:
    ctx->x[11] = (uint64_t)ctx->x[11] >> 24;
    glabel599:
    /* unimplemented opcode */
    glabel600:
    /* unimplemented opcode */
    glabel601:
    /* unimplemented opcode */
    glabel602:
    ctx->x[11] = ctx->x[11] ^ ctx->x[9];
    glabel603:
    ctx->x[10] = ctx->x[10] + ctx->x[0];
    glabel604:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v897, 8, 0);
    glabel605:
    if (ctx->x[11]) goto bb_1; else goto bb_0;
bb_111:
    glabel606:
    goto glabel1;
bb_112:
    glabel607:
    ctx->x[0] = 31 | 31;
    glabel608:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel609:
    /* unimplemented opcode */
    glabel610:
    /* unimplemented opcode */
    glabel611:
    ctx->x[8] = ctx->x[8] + ctx->x[9];
    glabel612:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v906, 8, 0);
    glabel613:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_113:
    glabel614:
    ctx->x[22] = 24;
    glabel615:
    ctx->x[23] = 65522;
    glabel616:
    goto glabel0;
bb_114:
    glabel617:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel618:
    ctx->x[8] = ctx->x[8] + ctx->x[24];
    glabel619:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v916, 8, 0);
    glabel620:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_115:
    glabel621:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel622:
    /* unimplemented opcode */
    glabel623:
    ctx->x[0] = 31 | ctx->x[8];
    glabel624:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel625:
    /* unimplemented opcode */
    glabel626:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(4)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(4)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(4)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(4)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(4)))) >> 31) << 28));
    glabel627:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_116:
    glabel628:
    ctx->x[24] = ctx->x[24] * ctx->x[22];
    glabel629:
    ctx->x[0] = v928 + ctx->x[9];
    glabel630:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel631:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(0)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(0)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(0)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(0)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(0)))) >> 31) << 28));
    glabel632:
    /* unimplemented opcode */
    glabel633:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_117:
    glabel634:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel635:
    ctx->x[0] = 31 | ctx->x[19];
    glabel636:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel637:
    ctx->x[8] = ctx->x[8] + ctx->x[9];
    glabel638:
    sr_rt_call(ctx, 485331317267);
bb_118:
    glabel639:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_119:
    glabel640:
    goto glabel1;
bb_120:
    glabel641:
    ctx->x[0] = 31 | 31;
    glabel642:
    ctx->x[0] = 31 | ctx->x[21];
    glabel643:
    /* unimplemented opcode */
    glabel644:
    /* unimplemented opcode */
    glabel645:
    /* unimplemented opcode */
    glabel646:
    /* unimplemented opcode */
    glabel647:
    return;
bb_121:
    glabel648:
    /* unimplemented opcode */
    glabel649:
    /* unimplemented opcode */
    glabel650:
    ctx->x[0] = 31 + 16;
    glabel651:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel652:
    ctx->x[0] = 31 | ctx->x[0];
    glabel653:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_122:
    glabel654:
    ctx->x[0] = 31 | 31;
    glabel655:
    ctx->x[0] = 31 | 31;
    glabel656:
    ctx->x[0] = 31 | 31;
    glabel657:
    ctx->x[0] = 31 | 31;
    glabel658:
    sr_rt_call(ctx, 485331317183);
bb_123:
    glabel659:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel660:
    ctx->x[20] = ctx->x[20] + 1;
    glabel661:
    /* unimplemented opcode */
    glabel662:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel1;
bb_124:
    glabel663:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel664:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_125:
    glabel665:
    ctx->x[0] = 31 | 31;
    glabel666:
    ctx->x[0] = 31 | 31;
    glabel667:
    goto glabel0;
bb_126:
    glabel668:
    ctx->x[10] = ctx->x[10] + 1;
    glabel669:
    ctx->x[9] = ctx->x[9] + 24;
    glabel670:
    /* unimplemented opcode */
    glabel671:
    if (((ctx->nzcv >> 29) & 1u)) goto glabel0;
bb_127:
    glabel672:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel673:
    ctx->x[11] = ctx->x[11] + ctx->x[9];
    glabel674:
    ctx->x[11] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[11], 8, 0);
    glabel675:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[12]) < (int32_t)(1027)) << 31) | ((uint32_t)((uint32_t)(ctx->x[12]) == (uint32_t)(1027)) << 30) | ((uint32_t)((uint32_t)(ctx->x[12]) >= (uint32_t)(1027)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[12]) ^ (uint32_t)(1027)) & ((uint32_t)(ctx->x[12]) ^ ((uint32_t)(ctx->x[12]) - (uint32_t)(1027)))) >> 31) << 28));
    glabel676:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_128:
    glabel677:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel678:
    ctx->x[11] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[11], 8, 0);
    glabel679:
    ctx->x[11] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[11], 8, 0);
    glabel680:
    ctx->x[11] = ctx->x[11] + ctx->x[8];
    glabel681:
    ctx->x[12] = ctx->x[12] + ctx->x[8];
    glabel682:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v992, 8, 1) = ctx->x[11];
    glabel683:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel684:
    goto glabel0;
bb_129:
    glabel685:
    /* unimplemented opcode */
    glabel686:
    /* unimplemented opcode */
    glabel687:
    return;
bb_130:
    glabel688:
    ctx->x[0] = 31 + ctx->x[27];
    glabel689:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v1000, 8, 1) = ctx->x[25];
    glabel690:
    /* unimplemented opcode */
    glabel691:
    /* unimplemented opcode */
    glabel692:
    /* unimplemented opcode */
    glabel693:
    /* unimplemented opcode */
    glabel694:
    ctx->x[0] = 31 + 64;
    glabel695:
    ctx->x[0] = 31 | ctx->x[1];
    glabel696:
    ctx->x[0] = 31 | ctx->x[0];
    glabel697:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 1) = ctx->x[2];
    glabel698:
    ctx->x[0] = 31 | 31;
    glabel699:
    /* unimplemented opcode */
    glabel700:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 1) = ctx->x[0];
    glabel701:
    ctx->x[3] = ctx->x[3] + 8;
    glabel702:
    ctx->x[23] = 65529;
    glabel703:
    ctx->x[23] = ctx->x[23] & 18446744069414649855ULL;
    glabel704:
    ctx->x[0] = v1018 | 1878982656;
    glabel705:
    ctx->x[0] = 485331312640 + 8192;
    glabel706:
    ctx->x[24] = ctx->x[24] + 2258;
    glabel707:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 1) = ctx->x[8];
    glabel708:
    ctx->x[25] = 65530;
    glabel709:
    ctx->x[25] = ctx->x[25] & 18446744069414649855ULL;
    glabel710:
    ctx->x[0] = v1030 | 1878982656;
    glabel711:
    /* unimplemented opcode */
    glabel712:
    /* unimplemented opcode */
    glabel713:
    /* unimplemented opcode */
    glabel714:
    /* unimplemented opcode */
    glabel715:
    /* unimplemented opcode */
    glabel716:
    /* unimplemented opcode */
    glabel717:
    /* unimplemented opcode */
    glabel718:
    /* unimplemented opcode */
    glabel719:
    /* unimplemented opcode */
    glabel720:
    /* unimplemented opcode */
    glabel721:
    ctx->x[22] = ctx->x[22] + 31;
    glabel722:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1043, 8, 0);
    glabel723:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel724:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_131:
    glabel725:
    goto glabel0;
bb_132:
    glabel726:
    ctx->x[22] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[22], 8, 0);
    glabel727:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel728:
    ctx->x[22] = ctx->x[22] + 16;
    glabel729:
    ctx->x[22] = ctx->x[22] + 31;
    glabel730:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1055, 8, 0);
    glabel731:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel732:
    if ((((ctx->nzcv >> 29) & 1u) && (((ctx->nzcv >> 30) & 1u) == 0))) goto glabel0;
bb_133:
    glabel733:
    /* unimplemented opcode */
    glabel734:
    ctx->x[24] = ctx->x[24] + ctx->x[8];
    glabel735:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1062, 8, 0);
    glabel736:
    ctx->x[10] = ctx->x[10] << 2;
    glabel737:
    ctx->x[9] = ctx->x[9] + v1064;
    glabel738:
    /* unimplemented opcode */
    glabel739:
    ctx->x[22] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[22], 8, 0);
    glabel740:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(24)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(24)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(24)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(24)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(24)))) >> 63) << 28));
    glabel741:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel2;
bb_134:
    glabel742:
    goto glabel0;
bb_135:
    glabel743:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel744:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1074, 8, 0);
    glabel745:
    ctx->x[8] = ctx->x[8] + ctx->x[20];
    glabel746:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel747:
    ctx->x[22] = ctx->x[22] + 31;
    glabel748:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1078, 8, 0);
    glabel749:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel750:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_136:
    glabel751:
    /* unimplemented opcode */
    glabel752:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_137:
    glabel753:
    /* unimplemented opcode */
    glabel754:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel2;
bb_138:
    glabel755:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel756:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1090, 8, 0);
    glabel757:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel758:
    ctx->x[22] = ctx->x[22] + 31;
    glabel759:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1093, 8, 0);
    glabel760:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel761:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_139:
    glabel762:
    goto glabel0;
bb_140:
    glabel763:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel764:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1101, 8, 0);
    glabel765:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel766:
    ctx->x[22] = ctx->x[22] + 31;
    glabel767:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1104, 8, 0);
    glabel768:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel769:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_141:
    glabel770:
    goto glabel0;
bb_142:
    glabel771:
    ctx->x[22] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[22], 8, 0);
    glabel772:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(7)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(7)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(7)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(7)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(7)))) >> 63) << 28));
    glabel773:
    /* unimplemented opcode */
    glabel774:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel775:
    ctx->x[22] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[22], 8, 0);
    glabel776:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(7)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(7)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(7)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(7)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(7)))) >> 63) << 28));
    glabel777:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel2;
bb_143:
    glabel778:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(17)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(17)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(17)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(17)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(17)))) >> 63) << 28));
    glabel779:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel2;
bb_144:
    glabel780:
    goto glabel0;
bb_145:
    glabel781:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel782:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1127, 8, 0);
    glabel783:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel784:
    ctx->x[22] = ctx->x[22] + 31;
    glabel785:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1130, 8, 0);
    glabel786:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel787:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_146:
    glabel788:
    goto glabel0;
bb_147:
    glabel789:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel790:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1138, 8, 0);
    glabel791:
    ctx->x[8] = ctx->x[8] + ctx->x[20];
    glabel792:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel793:
    ctx->x[22] = ctx->x[22] + 31;
    glabel794:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1142, 8, 0);
    glabel795:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel796:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_148:
    glabel797:
    goto glabel0;
bb_149:
    glabel798:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel799:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1150, 8, 0);
    glabel800:
    ctx->x[8] = ctx->x[8] + ctx->x[20];
    glabel801:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel802:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[9];
    glabel803:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel804:
    ctx->x[8] = ctx->x[8] + 8;
    glabel805:
    ctx->x[9] = ctx->x[9] << 2;
    glabel806:
    ctx->x[8] = ctx->x[8] + v1157;
    glabel807:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[10];
    glabel808:
    /* unimplemented opcode */
    glabel809:
    ctx->x[22] = ctx->x[22] + 31;
    glabel810:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1161, 8, 0);
    glabel811:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel812:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_150:
    glabel813:
    goto glabel0;
bb_151:
    glabel814:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel815:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1169, 8, 0);
    glabel816:
    ctx->x[8] = ctx->x[8] + ctx->x[20];
    glabel817:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel818:
    ctx->x[22] = ctx->x[22] + 31;
    glabel819:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1173, 8, 0);
    glabel820:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel821:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_152:
    glabel822:
    goto glabel0;
bb_153:
    glabel823:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel824:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1181, 8, 0);
    glabel825:
    ctx->x[8] = ctx->x[8] + ctx->x[20];
    glabel826:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel827:
    ctx->x[22] = ctx->x[22] + 31;
    glabel828:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1185, 8, 0);
    glabel829:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel830:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_154:
    glabel831:
    goto glabel0;
bb_155:
    glabel832:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel833:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1193, 8, 0);
    glabel834:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel835:
    ctx->x[22] = ctx->x[22] + 31;
    glabel836:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1196, 8, 0);
    glabel837:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel838:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_156:
    glabel839:
    goto glabel0;
bb_157:
    glabel840:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel841:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1204, 8, 0);
    glabel842:
    ctx->x[8] = ctx->x[8] + ctx->x[20];
    glabel843:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel844:
    ctx->x[22] = ctx->x[22] + 31;
    glabel845:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1208, 8, 0);
    glabel846:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel847:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_158:
    glabel848:
    goto glabel0;
bb_159:
    glabel849:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel850:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1216, 8, 0);
    glabel851:
    ctx->x[8] = ctx->x[8] + ctx->x[20];
    glabel852:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel853:
    ctx->x[22] = ctx->x[22] + 31;
    glabel854:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1220, 8, 0);
    glabel855:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel856:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_160:
    glabel857:
    goto glabel0;
bb_161:
    glabel858:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel859:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1228, 8, 0);
    glabel860:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel861:
    ctx->x[22] = ctx->x[22] + 31;
    glabel862:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1231, 8, 0);
    glabel863:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel864:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_162:
    glabel865:
    goto glabel0;
bb_163:
    glabel866:
    ctx->x[22] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[22], 8, 0);
    glabel867:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(16)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(16)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(16)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(16)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(16)))) >> 63) << 28));
    glabel868:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel2;
bb_164:
    glabel869:
    ctx->x[0] = 31 | 31;
    glabel870:
    ctx->x[0] = 31 | 31;
    glabel871:
    ctx->x[0] = 31 | 31;
    glabel872:
    sr_rt_call(ctx, 485331317183);
bb_165:
    glabel873:
    ctx->x[22] = ctx->x[22] + 16;
    glabel874:
    ctx->x[22] = ctx->x[22] + 31;
    glabel875:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1251, 8, 0);
    glabel876:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel877:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_166:
    glabel878:
    goto glabel0;
bb_167:
    glabel879:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel880:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1259, 8, 0);
    glabel881:
    ctx->x[8] = ctx->x[8] + ctx->x[20];
    glabel882:
    ctx->x[22] = ctx->x[22] + 31;
    glabel883:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1262, 8, 0);
    glabel884:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel885:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_168:
    glabel886:
    goto glabel0;
bb_169:
    glabel887:
    ctx->x[22] = ctx->x[22] + ctx->x[1];
    glabel888:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1270, 8, 0);
    glabel889:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel890:
    ctx->x[22] = ctx->x[22] + 31;
    glabel891:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1273, 8, 0);
    glabel892:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(23)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(23)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(23)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(23)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(23)))) >> 63) << 28));
    glabel893:
    if ((((ctx->nzcv >> 29) & 1u) == 0 || ((ctx->nzcv >> 30) & 1u))) goto glabel0;
bb_170:
    glabel894:
    goto glabel0;
bb_171:
    glabel895:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[21];
    glabel896:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel897:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[0];
    glabel898:
    /* unimplemented opcode */
    glabel899:
    /* unimplemented opcode */
    glabel900:
    /* unimplemented opcode */
    glabel901:
    /* unimplemented opcode */
    glabel902:
    ctx->x[0] = 31 + ctx->x[5];
    glabel903:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1288, 8, 0);
    glabel904:
    return;
bb_172:
    glabel905:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel906:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_173:
    glabel907:
    /* unimplemented opcode */
    glabel908:
    return;
bb_174:
    glabel909:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel910:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_175:
    glabel911:
    /* unimplemented opcode */
    glabel912:
    return;
bb_176:
    glabel913:
    ctx->x[0] = 31 - 112;
    glabel914:
    *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 1) = ctx->x[23];
    glabel915:
    /* unimplemented opcode */
    glabel916:
    /* unimplemented opcode */
    glabel917:
    /* unimplemented opcode */
    glabel918:
    ctx->x[0] = 31 + 96;
    glabel919:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel920:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_177:
    glabel921:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel922:
    ctx->x[9] = 24;
    glabel923:
    /* unimplemented opcode */
    glabel924:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel925:
    ctx->x[0] = 31 | ctx->x[1];
    glabel926:
    ctx->x[29] = ctx->x[29] - 40;
    glabel927:
    ctx->x[10] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[10], 8, 0);
    glabel928:
    ctx->x[10] = ctx->x[10] * ctx->x[9];
    glabel929:
    ctx->x[0] = v1318 + ctx->x[8];
    glabel930:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel931:
    ctx->x[8] = ctx->x[8] + ctx->x[0];
    glabel932:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1321, 8, 0);
    glabel933:
    ctx->x[8] = ctx->x[8] + ctx->x[0];
    glabel934:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1323, 8, 0);
    glabel935:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel936:
    ctx->x[0] = 31 + 0;
    glabel937:
    ctx->x[0] = 31 | ctx->x[0];
    glabel938:
    *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 1) = ctx->x[8];
    glabel939:
    /* unimplemented opcode */
    glabel940:
    *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 1) = ctx->x[23];
    glabel941:
    ctx->x[0] = 31 + ctx->x[0];
    glabel942:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v1331, 8, 1) = ctx->x[9];
    glabel943:
    ctx->x[0] = 31 + ctx->x[0];
    glabel944:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v1333, 8, 1) = ctx->x[10];
    glabel945:
    *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 1) = ctx->x[8];
    glabel946:
    sr_rt_call(ctx, 485331315343);
bb_178:
    glabel947:
    /* unimplemented opcode */
    glabel948:
    ctx->x[29] = ctx->x[29] + ctx->x[29];
    glabel949:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1339, 8, 0);
    glabel950:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_179:
    glabel951:
    ctx->x[9] = 24;
    glabel952:
    ctx->x[22] = ctx->x[22] * ctx->x[9];
    glabel953:
    ctx->x[0] = v1344 + ctx->x[21];
    glabel954:
    ctx->x[9] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 0);
    glabel955:
    ctx->x[9] = ctx->x[9] + ctx->x[8];
    glabel956:
    ctx->x[29] = ctx->x[29] + ctx->x[29];
    glabel957:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v1348, 8, 1) = ctx->x[8];
    glabel958:
    goto glabel3;
bb_180:
    glabel959:
    ctx->x[0] = 31 | 31;
    glabel960:
    ctx->x[0] = 31 | 31;
    glabel961:
    ctx->x[0] = 31 | 31;
    glabel962:
    sr_rt_call(ctx, 485331317183);
bb_181:
    glabel963:
    ctx->x[0] = 31 | 31;
    glabel964:
    goto glabel13;
bb_182:
    glabel965:
    ctx->x[0] = 485331312640 + 8192;
    glabel966:
    ctx->x[20] = ctx->x[20] + 2008;
    glabel967:
    ctx->x[0] = 31 | ctx->x[20];
    glabel968:
    sr_rt_call(ctx, 485331317683);
bb_183:
    glabel969:
    ctx->x[0] = 31 | ctx->x[0];
    glabel970:
    ctx->x[0] = 31 | ctx->x[20];
    glabel971:
    sr_rt_call(ctx, 485331317191);
bb_184:
    glabel972:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel973:
    ctx->x[8] = ctx->x[8] + ctx->x[23];
    glabel974:
    ctx->x[0] = 31 | ctx->x[20];
    glabel975:
    sr_rt_call(ctx, 485331317683);
bb_185:
    glabel976:
    ctx->x[0] = 31 | ctx->x[0];
    glabel977:
    ctx->x[0] = 31 | ctx->x[20];
    glabel978:
    sr_rt_call(ctx, 485331317191);
bb_186:
    glabel979:
    ctx->x[0] = 485331312640 + 8192;
    glabel980:
    ctx->x[20] = ctx->x[20] + 2036;
    glabel981:
    ctx->x[0] = 31 | ctx->x[20];
    glabel982:
    sr_rt_call(ctx, 485331317683);
bb_187:
    glabel983:
    ctx->x[0] = 31 | ctx->x[0];
    glabel984:
    ctx->x[0] = 31 | ctx->x[20];
    glabel985:
    sr_rt_call(ctx, 485331317191);
bb_188:
    glabel986:
    ctx->x[0] = 31 | 31;
    glabel987:
    ctx->x[29] = ctx->x[29] + ctx->x[29];
    glabel988:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v1394, 8, 1) = ctx->x[0];
    glabel989:
    ctx->x[10] = 24;
    glabel990:
    ctx->x[22] = ctx->x[22] * ctx->x[10];
    glabel991:
    ctx->x[0] = v1398 + 31;
    glabel992:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel993:
    ctx->x[21] = ctx->x[21] + ctx->x[10];
    glabel994:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1401, 8, 0);
    glabel995:
    ctx->x[10] = ctx->x[10] + ctx->x[9];
    glabel996:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v1403, 8, 1) = ctx->x[8];
    glabel997:
    ctx->x[29] = ctx->x[29] + ctx->x[29];
    glabel998:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1405, 8, 0);
    glabel999:
    /* unimplemented opcode */
    glabel1000:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 0);
    glabel1001:
    /* unimplemented opcode */
    glabel1002:
    /* unimplemented opcode */
    glabel1003:
    ctx->x[0] = 31 + 112;
    glabel1004:
    return;
bb_189:
    glabel1005:
    /* unimplemented opcode */
    glabel1006:
    /* unimplemented opcode */
    glabel1007:
    /* unimplemented opcode */
    glabel1008:
    /* unimplemented opcode */
    glabel1009:
    /* unimplemented opcode */
    glabel1010:
    ctx->x[0] = 31 + 64;
    glabel1011:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel1012:
    ctx->x[0] = 31 | ctx->x[2];
    glabel1013:
    ctx->x[0] = 31 | ctx->x[1];
    glabel1014:
    ctx->x[2] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[2], 8, 0);
    glabel1015:
    ctx->x[0] = 31 | ctx->x[0];
    glabel1016:
    ctx->x[10] = ctx->x[10] + ctx->x[12];
    glabel1017:
    ctx->x[2] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[2], 8, 0);
    glabel1018:
    /* unimplemented opcode */
    glabel1019:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_190:
    glabel1020:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel1021:
    ctx->x[22] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[22], 8, 0);
    glabel1022:
    if (ctx->x[11]) goto bb_1; else goto bb_0;
bb_191:
    glabel1023:
    ctx->x[12] = ctx->x[12] + ctx->x[10];
    glabel1024:
    ctx->x[0] = 31 | 31;
    glabel1025:
    ctx->x[10] = ctx->x[10] + 1;
    glabel1026:
    ctx->x[9] = (uint64_t)ctx->x[9] >> 28;
    glabel1027:
    /* unimplemented opcode */
    glabel1028:
    /* unimplemented opcode */
    glabel1029:
    ctx->x[11] = (uint64_t)ctx->x[11] >> 24;
    glabel1030:
    /* unimplemented opcode */
    glabel1031:
    /* unimplemented opcode */
    glabel1032:
    /* unimplemented opcode */
    glabel1033:
    ctx->x[11] = ctx->x[11] ^ ctx->x[9];
    glabel1034:
    ctx->x[10] = ctx->x[10] + ctx->x[0];
    glabel1035:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1447, 8, 0);
    glabel1036:
    if (ctx->x[11]) goto bb_1; else goto bb_0;
bb_192:
    glabel1037:
    goto glabel1;
bb_193:
    glabel1038:
    ctx->x[0] = 31 | ctx->x[22];
    glabel1039:
    sr_rt_call(ctx, 485331313587);
bb_194:
    glabel1040:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_195:
    glabel1041:
    ctx->x[0] = 485331312640 + 12288;
    glabel1042:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1043:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1044:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_196:
    glabel1045:
    ctx->x[0] = 31 | ctx->x[21];
    glabel1046:
    ctx->x[0] = 31 | ctx->x[22];
    glabel1047:
    /* unimplemented opcode */
    glabel1048:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_197:
    glabel1049:
    goto glabel0;
bb_198:
    glabel1050:
    ctx->x[0] = 31 | 31;
    glabel1051:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel1052:
    /* unimplemented opcode */
    glabel1053:
    /* unimplemented opcode */
    glabel1054:
    ctx->x[8] = ctx->x[8] + ctx->x[9];
    glabel1055:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1472, 8, 0);
    glabel1056:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_199:
    glabel1057:
    ctx->x[23] = 24;
    glabel1058:
    ctx->x[24] = 65522;
    glabel1059:
    goto glabel0;
bb_200:
    glabel1060:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel1061:
    ctx->x[8] = ctx->x[8] + ctx->x[25];
    glabel1062:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1482, 8, 0);
    glabel1063:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_201:
    glabel1064:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel1065:
    ctx->x[0] = 31 | ctx->x[8];
    glabel1066:
    /* unimplemented opcode */
    glabel1067:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1068:
    /* unimplemented opcode */
    glabel1069:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(4)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(4)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(4)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(4)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(4)))) >> 31) << 28));
    glabel1070:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_202:
    glabel1071:
    ctx->x[25] = ctx->x[25] * ctx->x[23];
    glabel1072:
    ctx->x[0] = v1493 + ctx->x[26];
    glabel1073:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1074:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(0)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(0)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(0)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(0)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(0)))) >> 31) << 28));
    glabel1075:
    /* unimplemented opcode */
    glabel1076:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_203:
    glabel1077:
    ctx->x[25] = ctx->x[25] * ctx->x[23];
    glabel1078:
    ctx->x[0] = v1500 + 31;
    glabel1079:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel1080:
    ctx->x[0] = 31 | ctx->x[22];
    glabel1081:
    ctx->x[26] = ctx->x[26] + ctx->x[9];
    glabel1082:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1504, 8, 0);
    glabel1083:
    ctx->x[8] = ctx->x[8] + ctx->x[9];
    glabel1084:
    sr_rt_call(ctx, 485331317267);
bb_204:
    glabel1085:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_205:
    glabel1086:
    ctx->x[9] = 24;
    glabel1087:
    ctx->x[25] = ctx->x[25] * ctx->x[9];
    glabel1088:
    ctx->x[0] = v1512 + ctx->x[26];
    glabel1089:
    ctx->x[21] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[21], 8, 0);
    glabel1090:
    ctx->x[9] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 0);
    glabel1091:
    ctx->x[9] = ctx->x[9] + ctx->x[8];
    glabel1092:
    if (ctx->x[0]) goto bb_1; else goto bb_0;
bb_206:
    glabel1093:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel1094:
    /* unimplemented opcode */
    glabel1095:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(32)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(32)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(32)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(32)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(32)))) >> 31) << 28));
    glabel1096:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_207:
    glabel1097:
    ctx->x[0] = 31 | 31;
    glabel1098:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[0];
    glabel1099:
    ctx->x[0] = 1;
    glabel1100:
    goto glabel1;
bb_208:
    glabel1101:
    ctx->x[0] = 31 | 31;
    glabel1102:
    /* unimplemented opcode */
    glabel1103:
    /* unimplemented opcode */
    glabel1104:
    /* unimplemented opcode */
    glabel1105:
    /* unimplemented opcode */
    glabel1106:
    /* unimplemented opcode */
    glabel1107:
    return;
bb_209:
    glabel1108:
    ctx->x[0] = 31 - 160;
    glabel1109:
    /* unimplemented opcode */
    glabel1110:
    /* unimplemented opcode */
    glabel1111:
    /* unimplemented opcode */
    glabel1112:
    /* unimplemented opcode */
    glabel1113:
    /* unimplemented opcode */
    glabel1114:
    /* unimplemented opcode */
    glabel1115:
    ctx->x[0] = 31 + 144;
    glabel1116:
    *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 1) = ctx->x[1];
    glabel1117:
    /* unimplemented opcode */
    glabel1118:
    ctx->x[0] = 31 | ctx->x[0];
    glabel1119:
    /* unimplemented opcode */
    glabel1120:
    if (((ctx->nzcv >> 29) & 1u)) goto glabel0;
bb_210:
    glabel1121:
    ctx->x[0] = 31 | 31;
    glabel1122:
    ctx->x[0] = 31 | 31;
    glabel1123:
    ctx->x[0] = 31 | 31;
    glabel1124:
    sr_rt_call(ctx, 485331317183);
bb_211:
    glabel1125:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1126:
    ctx->x[20] = ctx->x[20] + 1;
    glabel1127:
    /* unimplemented opcode */
    glabel1128:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel0;
bb_212:
    glabel1129:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1130:
    /* unimplemented opcode */
    glabel1131:
    ctx->x[24] = ctx->x[24] & 18446744073709486080ULL;
    glabel1132:
    ctx->x[0] = v1565 | 43691;
    glabel1133:
    /* unimplemented opcode */
    glabel1134:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1135:
    /* unimplemented opcode */
    glabel1136:
    if (((ctx->nzcv >> 29) & 1u)) goto glabel0;
bb_213:
    glabel1137:
    ctx->x[0] = 31 + 32;
    glabel1138:
    /* unimplemented opcode */
    glabel1139:
    ctx->x[25] = ctx->x[25] << 1;
    glabel1140:
    ctx->x[25] = ctx->x[25] + v1576;
    glabel1141:
    ctx->x[8] = (uint64_t)ctx->x[8] >> 61;
    glabel1142:
    ctx->x[0] = 485331312640 + 4096;
    glabel1143:
    ctx->x[21] = ctx->x[21] + 2093;
    glabel1144:
    ctx->x[20] = 24;
    glabel1145:
    ctx->x[0] = 485331312640 + 4096;
    glabel1146:
    ctx->x[22] = ctx->x[22] + 2036;
    glabel1147:
    goto glabel0;
bb_214:
    glabel1148:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 0);
    glabel1149:
    ctx->x[23] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[23], 8, 0);
    glabel1150:
    ctx->x[11] = ctx->x[11] + ctx->x[10];
    glabel1151:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1152:
    ctx->x[23] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[23], 8, 0);
    glabel1153:
    ctx->x[9] = ctx->x[9] + ctx->x[8];
    glabel1154:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v1599, 8, 1) = ctx->x[10];
    glabel1155:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1156:
    /* unimplemented opcode */
    glabel1157:
    ctx->x[25] = ctx->x[25] + 1;
    glabel1158:
    ctx->x[28] = ctx->x[28] + 24;
    glabel1159:
    /* unimplemented opcode */
    glabel1160:
    if (((ctx->nzcv >> 29) & 1u)) goto glabel0;
bb_215:
    glabel1161:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1162:
    ctx->x[8] = ctx->x[8] + ctx->x[28];
    glabel1163:
    ctx->x[23] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[23], 8, 0);
    glabel1164:
    ctx->x[8] = ctx->x[8] - 257;
    glabel1165:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[9]) < (int32_t)(2)) << 31) | ((uint32_t)((uint32_t)(ctx->x[9]) == (uint32_t)(2)) << 30) | ((uint32_t)((uint32_t)(ctx->x[9]) >= (uint32_t)(2)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[9]) ^ (uint32_t)(2)) & ((uint32_t)(ctx->x[9]) ^ ((uint32_t)(ctx->x[9]) - (uint32_t)(2)))) >> 31) << 28));
    glabel1166:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel0;
bb_216:
    glabel1167:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(1024)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(1024)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(1024)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(1024)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(1024)))) >> 31) << 28));
    glabel1168:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_217:
    glabel1169:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(1025)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(1025)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(1025)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(1025)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(1025)))) >> 31) << 28));
    glabel1170:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel7;
bb_218:
    glabel1171:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1172:
    ctx->x[8] = (uint64_t)ctx->x[8] >> 32;
    glabel1173:
    ctx->x[8] = ctx->x[8] * ctx->x[20];
    glabel1174:
    ctx->x[0] = v1628 + ctx->x[9];
    glabel1175:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1176:
    ctx->x[8] = ctx->x[8] + ctx->x[0];
    glabel1177:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1631, 8, 0);
    glabel1178:
    ctx->x[8] = ctx->x[8] + ctx->x[0];
    glabel1179:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1633, 8, 0);
    glabel1180:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1181:
    ctx->x[0] = 31 + 56;
    glabel1182:
    ctx->x[0] = 31 + 32;
    glabel1183:
    ctx->x[0] = 31 | ctx->x[19];
    glabel1184:
    *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 1) = ctx->x[8];
    glabel1185:
    *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 1) = ctx->x[27];
    glabel1186:
    /* unimplemented opcode */
    glabel1187:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[26], 8, 1) = ctx->x[8];
    glabel1188:
    /* unimplemented opcode */
    glabel1189:
    sr_rt_call(ctx, 485331315343);
bb_219:
    glabel1190:
    /* unimplemented opcode */
    glabel1191:
    ctx->x[0] = 485331312640 + 8192;
    glabel1192:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1193:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1194:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_220:
    glabel1195:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1196:
    ctx->x[0] = 31 | ctx->x[21];
    glabel1197:
    ctx->x[8] = ctx->x[8] + ctx->x[27];
    glabel1198:
    sr_rt_call(ctx, 485331317683);
bb_221:
    glabel1199:
    ctx->x[0] = 31 | ctx->x[0];
    glabel1200:
    ctx->x[0] = 31 | ctx->x[21];
    glabel1201:
    sr_rt_call(ctx, 485331317191);
bb_222:
    glabel1202:
    ctx->x[0] = 31 | ctx->x[23];
    glabel1203:
    sr_rt_call(ctx, 485331317683);
bb_223:
    glabel1204:
    ctx->x[0] = 31 | ctx->x[0];
    glabel1205:
    ctx->x[0] = 31 | ctx->x[23];
    glabel1206:
    sr_rt_call(ctx, 485331317191);
bb_224:
    glabel1207:
    ctx->x[0] = 31 | ctx->x[22];
    glabel1208:
    sr_rt_call(ctx, 485331317683);
bb_225:
    glabel1209:
    ctx->x[0] = 31 | ctx->x[0];
    glabel1210:
    ctx->x[0] = 31 | ctx->x[22];
    glabel1211:
    sr_rt_call(ctx, 485331317191);
bb_226:
    glabel1212:
    goto glabel7;
bb_227:
    glabel1213:
    ctx->x[0] = 31 | 31;
    glabel1214:
    ctx->x[0] = 31 | 31;
    glabel1215:
    ctx->x[0] = 31 | 31;
    glabel1216:
    sr_rt_call(ctx, 485331317183);
bb_228:
    glabel1217:
    goto glabel7;
bb_229:
    glabel1218:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1219:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1220:
    if (ctx->x[9]) goto bb_1; else goto bb_0;
bb_230:
    glabel1221:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(24)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(24)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(24)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(24)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(24)))) >> 63) << 28));
    glabel1222:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel0;
bb_231:
    glabel1223:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 0);
    glabel1224:
    ctx->x[19] = ctx->x[19] + 32;
    glabel1225:
    /* unimplemented opcode */
    glabel1226:
    ctx->x[0] = 31 | 31;
    glabel1227:
    ctx->x[0] = 31 | 31;
    glabel1228:
    goto glabel0;
bb_232:
    glabel1229:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel1230:
    ctx->x[0] = 31 | ctx->x[22];
    glabel1231:
    ctx->x[22] = ctx->x[22] + ctx->x[25];
    glabel1232:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1704, 8, 0);
    glabel1233:
    ctx->x[9] = ctx->x[9] + ctx->x[8];
    glabel1234:
    ctx->x[22] = ctx->x[22] + ctx->x[25];
    glabel1235:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v1707, 8, 1) = ctx->x[8];
    glabel1236:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1237:
    /* unimplemented opcode */
    glabel1238:
    ctx->x[21] = ctx->x[21] + 1;
    glabel1239:
    ctx->x[20] = ctx->x[20] + 24;
    glabel1240:
    /* unimplemented opcode */
    glabel1241:
    if (((ctx->nzcv >> 29) & 1u)) goto glabel0;
bb_233:
    glabel1242:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1243:
    ctx->x[9] = ctx->x[9] + ctx->x[20];
    glabel1244:
    ctx->x[10] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[10], 8, 0);
    glabel1245:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[10]) < (int32_t)(1026)) << 31) | ((uint32_t)((uint32_t)(ctx->x[10]) == (uint32_t)(1026)) << 30) | ((uint32_t)((uint32_t)(ctx->x[10]) >= (uint32_t)(1026)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[10]) ^ (uint32_t)(1026)) & ((uint32_t)(ctx->x[10]) ^ ((uint32_t)(ctx->x[10]) - (uint32_t)(1026)))) >> 31) << 28));
    glabel1246:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel8;
bb_234:
    glabel1247:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1248:
    ctx->x[9] = ctx->x[9] + ctx->x[20];
    glabel1249:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1725, 8, 0);
    glabel1250:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1251:
    ctx->x[25] = ctx->x[25] + ctx->x[22];
    glabel1252:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1728, 8, 0);
    glabel1253:
    ctx->x[8] = ctx->x[8] + ctx->x[22];
    glabel1254:
    if (ctx->x[9]) goto bb_1; else goto bb_0;
bb_235:
    glabel1255:
    /* unimplemented opcode */
    glabel1256:
    ctx->x[0] = 31 | ctx->x[22];
    glabel1257:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel2;
bb_236:
    glabel1258:
    ctx->x[0] = 31 | 31;
    glabel1259:
    ctx->x[0] = 31 | 31;
    glabel1260:
    ctx->x[0] = 31 | 31;
    glabel1261:
    sr_rt_call(ctx, 485331317183);
bb_237:
    glabel1262:
    ctx->x[23] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[23], 8, 0);
    glabel1263:
    goto glabel2;
bb_238:
    glabel1264:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[8]) < (int64_t)(16)) << 31) | ((uint32_t)((uint64_t)(ctx->x[8]) == (uint64_t)(16)) << 30) | ((uint32_t)((uint64_t)(ctx->x[8]) >= (uint64_t)(16)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[8]) ^ (uint64_t)(16)) & ((uint64_t)(ctx->x[8]) ^ ((uint64_t)(ctx->x[8]) - (uint64_t)(16)))) >> 63) << 28));
    glabel1265:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel0;
bb_239:
    glabel1266:
    ctx->x[0] = 31 | 31;
    glabel1267:
    ctx->x[0] = 31 | 31;
    glabel1268:
    ctx->x[0] = 31 | 31;
    glabel1269:
    ctx->x[0] = 31 | 31;
    glabel1270:
    sr_rt_call(ctx, 485331317183);
bb_240:
    glabel1271:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1272:
    ctx->x[20] = ctx->x[20] + 1;
    glabel1273:
    /* unimplemented opcode */
    glabel1274:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel1;
bb_241:
    glabel1275:
    goto glabel0;
bb_242:
    glabel1276:
    ctx->x[0] = 31 + 32;
    glabel1277:
    ctx->x[0] = 31 | 31;
    glabel1278:
    ctx->x[0] = 31 | 31;
    glabel1279:
    /* unimplemented opcode */
    glabel1280:
    ctx->x[27] = 24;
    glabel1281:
    goto glabel0;
bb_243:
    glabel1282:
    ctx->x[22] = ctx->x[22] + 16;
    glabel1283:
    ctx->x[0] = 31 + 56;
    glabel1284:
    ctx->x[9] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 0);
    glabel1285:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1286:
    ctx->x[8] = ctx->x[8] + ctx->x[9];
    glabel1287:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 1) = ctx->x[8];
    glabel1288:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1289:
    /* unimplemented opcode */
    glabel1290:
    ctx->x[26] = ctx->x[26] + 1;
    glabel1291:
    ctx->x[25] = ctx->x[25] + 24;
    glabel1292:
    /* unimplemented opcode */
    glabel1293:
    if (((ctx->nzcv >> 29) & 1u)) goto glabel0;
bb_244:
    glabel1294:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1295:
    ctx->x[10] = ctx->x[10] + ctx->x[25];
    glabel1296:
    ctx->x[22] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[22], 8, 0);
    glabel1297:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[9]) < (int32_t)(1026)) << 31) | ((uint32_t)((uint32_t)(ctx->x[9]) == (uint32_t)(1026)) << 30) | ((uint32_t)((uint32_t)(ctx->x[9]) >= (uint32_t)(1026)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[9]) ^ (uint32_t)(1026)) & ((uint32_t)(ctx->x[9]) ^ ((uint32_t)(ctx->x[9]) - (uint32_t)(1026)))) >> 31) << 28));
    glabel1298:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel7;
bb_245:
    glabel1299:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1300:
    ctx->x[10] = ctx->x[10] + ctx->x[25];
    glabel1301:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1794, 8, 0);
    glabel1302:
    ctx->x[10] = ctx->x[10] + ctx->x[8];
    glabel1303:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1304:
    ctx->x[20] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[20], 8, 0);
    glabel1305:
    ctx->x[11] = ctx->x[11] + ctx->x[8];
    glabel1306:
    if (ctx->x[10]) goto bb_1; else goto bb_0;
bb_246:
    glabel1307:
    /* unimplemented opcode */
    glabel1308:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel1;
bb_247:
    glabel1309:
    ctx->x[0] = 31 | 31;
    glabel1310:
    ctx->x[0] = 31 | 31;
    glabel1311:
    ctx->x[0] = 31 | 31;
    glabel1312:
    sr_rt_call(ctx, 485331317183);
bb_248:
    glabel1313:
    ctx->x[22] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[22], 8, 0);
    glabel1314:
    goto glabel1;
bb_249:
    glabel1315:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 1) = ctx->x[8];
    glabel1316:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1317:
    ctx->x[9] = (uint64_t)ctx->x[9] >> 32;
    glabel1318:
    ctx->x[9] = ctx->x[9] * ctx->x[27];
    glabel1319:
    ctx->x[0] = v1816 + ctx->x[8];
    glabel1320:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1321:
    ctx->x[8] = ctx->x[8] + ctx->x[0];
    glabel1322:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1819, 8, 0);
    glabel1323:
    ctx->x[8] = ctx->x[8] + ctx->x[0];
    glabel1324:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1821, 8, 0);
    glabel1325:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1326:
    ctx->x[0] = 31 + 56;
    glabel1327:
    ctx->x[0] = 31 + 32;
    glabel1328:
    ctx->x[0] = 31 | ctx->x[19];
    glabel1329:
    *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 1) = ctx->x[8];
    glabel1330:
    *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 1) = ctx->x[21];
    glabel1331:
    /* unimplemented opcode */
    glabel1332:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[28], 8, 1) = ctx->x[8];
    glabel1333:
    /* unimplemented opcode */
    glabel1334:
    sr_rt_call(ctx, 485331315343);
bb_250:
    glabel1335:
    /* unimplemented opcode */
    glabel1336:
    ctx->x[0] = 485331312640 + 8192;
    glabel1337:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1338:
    ctx->x[8] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 0);
    glabel1339:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_251:
    glabel1340:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1341:
    ctx->x[8] = ctx->x[8] + ctx->x[21];
    glabel1342:
    ctx->x[0] = 485331312640 + 4096;
    glabel1343:
    ctx->x[21] = ctx->x[21] + 2093;
    glabel1344:
    ctx->x[0] = 31 | ctx->x[21];
    glabel1345:
    sr_rt_call(ctx, 485331317683);
bb_252:
    glabel1346:
    ctx->x[0] = 31 | ctx->x[0];
    glabel1347:
    ctx->x[0] = 31 | ctx->x[21];
    glabel1348:
    sr_rt_call(ctx, 485331317191);
bb_253:
    glabel1349:
    ctx->x[0] = 31 | ctx->x[22];
    glabel1350:
    sr_rt_call(ctx, 485331317683);
bb_254:
    glabel1351:
    ctx->x[0] = 31 | ctx->x[0];
    glabel1352:
    ctx->x[0] = 31 | ctx->x[22];
    glabel1353:
    sr_rt_call(ctx, 485331317191);
bb_255:
    glabel1354:
    ctx->x[0] = 485331312640 + 4096;
    glabel1355:
    ctx->x[21] = ctx->x[21] + 2036;
    glabel1356:
    ctx->x[0] = 31 | ctx->x[21];
    glabel1357:
    sr_rt_call(ctx, 485331317683);
bb_256:
    glabel1358:
    ctx->x[0] = 31 | ctx->x[0];
    glabel1359:
    ctx->x[0] = 31 | ctx->x[21];
    glabel1360:
    sr_rt_call(ctx, 485331317191);
bb_257:
    glabel1361:
    ctx->x[0] = 31 | ctx->x[20];
    glabel1362:
    ctx->x[0] = 31 | ctx->x[23];
    glabel1363:
    goto glabel2;
bb_258:
    glabel1364:
    ctx->x[19] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[19], 8, 0);
    glabel1365:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_259:
    glabel1366:
    ctx->x[0] = 485331312640 + 8192;
    glabel1367:
    ctx->x[9] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[9], 8, 0);
    glabel1368:
    /* unimplemented opcode */
    glabel1369:
    /* unimplemented opcode */
    glabel1370:
    /* unimplemented opcode */
    glabel1371:
    /* unimplemented opcode */
    glabel1372:
    /* unimplemented opcode */
    glabel1373:
    /* unimplemented opcode */
    glabel1374:
    /* unimplemented opcode */
    glabel1375:
    ctx->x[0] = 31 + 160;
    glabel1376:
    return;
bb_260:
    glabel1377:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 0);
    glabel1378:
    *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 1) = ctx->x[29];
    glabel1379:
    /* unimplemented opcode */
    glabel1380:
    /* unimplemented opcode */
    glabel1381:
    /* unimplemented opcode */
    glabel1382:
    /* unimplemented opcode */
    glabel1383:
    /* unimplemented opcode */
    glabel1384:
    /* unimplemented opcode */
    glabel1385:
    /* unimplemented opcode */
    glabel1386:
    /* unimplemented opcode */
    glabel1387:
    /* unimplemented opcode */
    glabel1388:
    ctx->x[0] = 31 + 0;
    glabel1389:
    ctx->x[17] = ctx->x[17] - ctx->x[16];
    glabel1390:
    ctx->x[1] = ctx->x[1] - 8;
    glabel1391:
    ctx->x[1] = (uint64_t)ctx->x[1] >> 3;
    glabel1392:
    ctx->x[16] = ctx->x[16] + 31;
    glabel1393:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1911, 8, 0);
    glabel1394:
    sr_rt_call(ctx, 485331313875);
bb_261:
    glabel1395:
    ctx->x[0] = 31 | ctx->x[0];
    glabel1396:
    /* unimplemented opcode */
    glabel1397:
    /* unimplemented opcode */
    glabel1398:
    /* unimplemented opcode */
    glabel1399:
    /* unimplemented opcode */
    glabel1400:
    /* unimplemented opcode */
    glabel1401:
    /* unimplemented opcode */
    glabel1402:
    /* unimplemented opcode */
    glabel1403:
    /* unimplemented opcode */
    glabel1404:
    /* unimplemented opcode */
    glabel1405:
    /* unimplemented opcode */
    glabel1406:
    /* unimplemented opcode */
    glabel1407:
    ctx->x[0] = 31 | ctx->x[30];
    glabel1408:
    sr_rt_call(ctx, 485331316875);
bb_262:
    glabel1409:
    /* unimplemented opcode */
    glabel1410:
    ctx->x[30] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[30], 8, 0);
    glabel1411:
    ctx->x[2] = (int64_t)ctx->x[2] >> 0;
    glabel1412:
    ctx->x[2] = ctx->x[2] + ctx->x[30];
    glabel1413:
    ctx->x[1] = 1;
    glabel1414:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[2], 8, 1) = ctx->x[1];
    glabel1415:
    ctx->x[0] = 31 | ctx->x[0];
    glabel1416:
    return;
bb_263:
    glabel1417:
    /* unimplemented opcode */
    glabel1418:
    /* unimplemented opcode */
    glabel1419:
    /* unimplemented opcode */
    glabel1420:
    ctx->x[0] = 31 | 31;
    glabel1421:
    ctx->x[0] = 31 | 31;
    glabel1422:
    ctx->x[0] = 31 | 31;
    glabel1423:
    ctx->x[0] = 31 | 31;
    glabel1424:
    ctx->x[1] = ctx->x[1] + 8;
    glabel1425:
    ctx->x[13] = 65528;
    glabel1426:
    ctx->x[13] = ctx->x[13] & 18446744069414649855ULL;
    glabel1427:
    ctx->x[0] = v1949 | 1878982656;
    glabel1428:
    ctx->x[14] = 65529;
    glabel1429:
    ctx->x[14] = ctx->x[14] & 18446744069414649855ULL;
    glabel1430:
    ctx->x[0] = v1955 | 1878982656;
    glabel1431:
    ctx->x[15] = 65530;
    glabel1432:
    ctx->x[15] = ctx->x[15] & 18446744069414649855ULL;
    glabel1433:
    ctx->x[0] = v1961 | 1878982656;
    glabel1434:
    ctx->x[12] = ctx->x[12] + 31;
    glabel1435:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1964, 8, 0);
    glabel1436:
    /* unimplemented opcode */
    glabel1437:
    if ((((ctx->nzcv >> 30) & 1u) || (((ctx->nzcv >> 31) & 1u) != ((ctx->nzcv >> 28) & 1u)))) goto glabel0;
bb_264:
    glabel1438:
    goto glabel0;
bb_265:
    glabel1439:
    ctx->x[12] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[12], 8, 0);
    glabel1440:
    ctx->x[12] = ctx->x[12] + 16;
    glabel1441:
    ctx->x[12] = ctx->x[12] + 31;
    glabel1442:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1975, 8, 0);
    glabel1443:
    /* unimplemented opcode */
    glabel1444:
    if (((((ctx->nzcv >> 30) & 1u) == 0) && (((ctx->nzcv >> 31) & 1u) == ((ctx->nzcv >> 28) & 1u)))) goto glabel0;
bb_266:
    glabel1445:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[16]) < (int64_t)(7)) << 31) | ((uint32_t)((uint64_t)(ctx->x[16]) == (uint64_t)(7)) << 30) | ((uint32_t)((uint64_t)(ctx->x[16]) >= (uint64_t)(7)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[16]) ^ (uint64_t)(7)) & ((uint64_t)(ctx->x[16]) ^ ((uint64_t)(ctx->x[16]) - (uint64_t)(7)))) >> 63) << 28));
    glabel1446:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_267:
    glabel1447:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[16]) < (int64_t)(9)) << 31) | ((uint32_t)((uint64_t)(ctx->x[16]) == (uint64_t)(9)) << 30) | ((uint32_t)((uint64_t)(ctx->x[16]) >= (uint64_t)(9)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[16]) ^ (uint64_t)(9)) & ((uint64_t)(ctx->x[16]) ^ ((uint64_t)(ctx->x[16]) - (uint64_t)(9)))) >> 63) << 28));
    glabel1448:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_268:
    glabel1449:
    ctx->x[12] = ctx->x[12] + ctx->x[1];
    glabel1450:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1987, 8, 0);
    glabel1451:
    ctx->x[12] = ctx->x[12] + 31;
    glabel1452:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v1989, 8, 0);
    glabel1453:
    /* unimplemented opcode */
    glabel1454:
    if ((((ctx->nzcv >> 30) & 1u) || (((ctx->nzcv >> 31) & 1u) != ((ctx->nzcv >> 28) & 1u)))) goto glabel0;
bb_269:
    glabel1455:
    /* unimplemented opcode */
    glabel1456:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_270:
    glabel1457:
    /* unimplemented opcode */
    glabel1458:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel1;
bb_271:
    glabel1459:
    ctx->x[12] = ctx->x[12] + ctx->x[1];
    glabel1460:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v2000, 8, 0);
    glabel1461:
    ctx->x[12] = ctx->x[12] + 31;
    glabel1462:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v2002, 8, 0);
    glabel1463:
    /* unimplemented opcode */
    glabel1464:
    if ((((ctx->nzcv >> 30) & 1u) || (((ctx->nzcv >> 31) & 1u) != ((ctx->nzcv >> 28) & 1u)))) goto glabel0;
bb_272:
    glabel1465:
    goto glabel0;
bb_273:
    glabel1466:
    ctx->x[12] = ctx->x[12] + ctx->x[1];
    glabel1467:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v2009, 8, 0);
    glabel1468:
    ctx->x[11] = ctx->x[11] + ctx->x[0];
    glabel1469:
    ctx->x[12] = ctx->x[12] + 31;
    glabel1470:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v2012, 8, 0);
    glabel1471:
    /* unimplemented opcode */
    glabel1472:
    if ((((ctx->nzcv >> 30) & 1u) || (((ctx->nzcv >> 31) & 1u) != ((ctx->nzcv >> 28) & 1u)))) goto glabel0;
bb_274:
    glabel1473:
    goto glabel0;
bb_275:
    glabel1474:
    if (ctx->x[16]) goto bb_1; else goto bb_0;
bb_276:
    glabel1475:
    if (ctx->x[9]) goto bb_1; else goto bb_0;
bb_277:
    glabel1476:
    ctx->x[11] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[11], 8, 0);
    glabel1477:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[12]) < (int32_t)(1027)) << 31) | ((uint32_t)((uint32_t)(ctx->x[12]) == (uint32_t)(1027)) << 30) | ((uint32_t)((uint32_t)(ctx->x[12]) >= (uint32_t)(1027)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[12]) ^ (uint32_t)(1027)) & ((uint32_t)(ctx->x[12]) ^ ((uint32_t)(ctx->x[12]) - (uint32_t)(1027)))) >> 31) << 28));
    glabel1478:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_278:
    glabel1479:
    ctx->x[11] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[11], 8, 0);
    glabel1480:
    ctx->x[12] = ctx->x[12] + ctx->x[0];
    glabel1481:
    ctx->x[11] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[11], 8, 0);
    glabel1482:
    ctx->x[9] = ctx->x[9] - 1;
    glabel1483:
    ctx->x[11] = ctx->x[11] + ctx->x[10];
    glabel1484:
    ctx->x[13] = ctx->x[13] + ctx->x[0];
    glabel1485:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v2031, 8, 1) = ctx->x[12];
    glabel1486:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_279:
    glabel1487:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_280:
    glabel1488:
    goto glabel0;
bb_281:
    glabel1489:
    goto glabel0;
bb_282:
    glabel1490:
    return;
bb_283:
    glabel1491:
    /* unimplemented opcode */
    glabel1492:
    return;
bb_284:
    glabel1493:
    ctx->x[0] = 31 + 31;
    glabel1494:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v2043, 8, 1) = ctx->x[1];
    glabel1495:
    sr_rt_handle_svc(ctx, 6);
bb_285:
    glabel1496:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, 31, 8, 0);
    glabel1497:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[2], 8, 1) = ctx->x[1];
    glabel1498:
    ctx->x[0] = 31 + 16;
    glabel1499:
    return;
bb_286:
    glabel1500:
    sr_rt_handle_svc(ctx, 38);
bb_287:
    glabel1501:
    return;
bb_288:
    glabel1502:
    sr_rt_handle_svc(ctx, 39);
bb_289:
    glabel1503:
    return;
bb_290:
    glabel1504:
    sr_rt_handle_svc(ctx, 40);
bb_291:
    glabel1505:
    return;
bb_292:
    glabel1506:
    /* unimplemented opcode */
    glabel1507:
    /* unimplemented opcode */
    glabel1508:
    /* unimplemented opcode */
    glabel1509:
    /* unimplemented opcode */
    glabel1510:
    /* unimplemented opcode */
    glabel1511:
    /* unimplemented opcode */
    glabel1512:
    /* unimplemented opcode */
    glabel1513:
    /* unimplemented opcode */
    glabel1514:
    /* unimplemented opcode */
    glabel1515:
    /* unimplemented opcode */
    glabel1516:
    /* unimplemented opcode */
    glabel1517:
    /* unimplemented opcode */
    glabel1518:
    /* unimplemented opcode */
    glabel1519:
    /* unimplemented opcode */
    glabel1520:
    /* unimplemented opcode */
    glabel1521:
    ctx->x[1] = ctx->x[1] ^ ctx->x[0];
    glabel1522:
    /* unimplemented opcode */
    glabel1523:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_293:
    glabel1524:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel1525:
    ctx->x[1] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[1], 8, 0);
    glabel1526:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(0)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(0)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(0)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(0)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(0)))) >> 31) << 28));
    glabel1527:
    /* unimplemented opcode */
    glabel1528:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_294:
    glabel1529:
    ctx->x[0] = ctx->x[0] + 1;
    glabel1530:
    ctx->x[1] = ctx->x[1] + 1;
    glabel1531:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel1532:
    ctx->x[1] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[1], 8, 0);
    glabel1533:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(0)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(0)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(0)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(0)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(0)))) >> 31) << 28));
    glabel1534:
    /* unimplemented opcode */
    glabel1535:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_295:
    glabel1536:
    ctx->x[8] = ctx->x[8] - ctx->x[9];
    glabel1537:
    return;
bb_296:
    glabel1538:
    ctx->x[0] = ctx->x[0] + ctx->x[0];
    glabel1539:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v2095, 8, 0);
    glabel1540:
    ctx->x[1] = ctx->x[1] + ctx->x[0];
    glabel1541:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v2097, 8, 0);
    glabel1542:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(0)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(0)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(0)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(0)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(0)))) >> 31) << 28));
    glabel1543:
    /* unimplemented opcode */
    glabel1544:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_297:
    glabel1545:
    /* unimplemented opcode */
    glabel1546:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_298:
    glabel1547:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel1548:
    ctx->x[1] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[1], 8, 0);
    glabel1549:
    ctx->nzcv = (uint32_t)(((uint32_t)((int32_t)(ctx->x[8]) < (int32_t)(0)) << 31) | ((uint32_t)((uint32_t)(ctx->x[8]) == (uint32_t)(0)) << 30) | ((uint32_t)((uint32_t)(ctx->x[8]) >= (uint32_t)(0)) << 29) | ((uint32_t)((((uint32_t)(ctx->x[8]) ^ (uint32_t)(0)) & ((uint32_t)(ctx->x[8]) ^ ((uint32_t)(ctx->x[8]) - (uint32_t)(0)))) >> 31) << 28));
    glabel1550:
    /* unimplemented opcode */
    glabel1551:
    if ((((ctx->nzcv >> 30) & 1u) == 0)) goto glabel0;
bb_299:
    glabel1552:
    /* unimplemented opcode */
    glabel1553:
    ctx->x[0] = 31 | 31;
    glabel1554:
    ctx->x[10] = ctx->x[10] & 18446744073709486080ULL;
    glabel1555:
    ctx->x[0] = v2114 | 65279;
    glabel1556:
    ctx->x[0] = ctx->x[0] + ctx->x[0];
    glabel1557:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v2117, 8, 0);
    glabel1558:
    ctx->x[8] = ctx->x[8] + ctx->x[10];
    glabel1559:
    ctx->x[1] = ctx->x[1] + ctx->x[0];
    glabel1560:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v2120, 8, 0);
    glabel1561:
    ctx->x[9] = ctx->x[9] ^ ctx->x[8];
    glabel1562:
    /* unimplemented opcode */
    glabel1563:
    /* unimplemented opcode */
    glabel1564:
    ctx->x[11] = ctx->x[11] | ctx->x[12];
    glabel1565:
    if (ctx->x[11]) goto bb_1; else goto bb_0;
bb_300:
    glabel1566:
    if (ctx->x[12]) goto bb_1; else goto bb_0;
bb_301:
    glabel1567:
    ctx->x[0] = 31 | 31;
    glabel1568:
    return;
bb_302:
    glabel1569:
    /* unimplemented opcode */
    glabel1570:
    /* unimplemented opcode */
    glabel1571:
    /* unimplemented opcode */
    glabel1572:
    /* unimplemented opcode */
    glabel1573:
    /* unimplemented opcode */
    glabel1574:
    /* unimplemented opcode */
    glabel1575:
    /* unimplemented opcode */
    glabel1576:
    return;
bb_303:
    glabel1577:
    /* unimplemented opcode */
    glabel1578:
    if (ctx->x[2]) goto bb_1; else goto bb_0;
bb_304:
    glabel1579:
    ctx->x[2] = ctx->x[2] + ctx->x[0];
    glabel1580:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[2]) < (int64_t)(3)) << 31) | ((uint32_t)((uint64_t)(ctx->x[2]) == (uint64_t)(3)) << 30) | ((uint32_t)((uint64_t)(ctx->x[2]) >= (uint64_t)(3)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[2]) ^ (uint64_t)(3)) & ((uint64_t)(ctx->x[2]) ^ ((uint64_t)(ctx->x[2]) - (uint64_t)(3)))) >> 63) << 28));
    glabel1581:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 1) = ctx->x[1];
    glabel1582:
    ctx->x[8] = ctx->x[8] + 31;
    glabel1583:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v2143, 8, 1) = ctx->x[1];
    glabel1584:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel0;
bb_305:
    glabel1585:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[2]) < (int64_t)(7)) << 31) | ((uint32_t)((uint64_t)(ctx->x[2]) == (uint64_t)(7)) << 30) | ((uint32_t)((uint64_t)(ctx->x[2]) >= (uint64_t)(7)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[2]) ^ (uint64_t)(7)) & ((uint64_t)(ctx->x[2]) ^ ((uint64_t)(ctx->x[2]) - (uint64_t)(7)))) >> 63) << 28));
    glabel1586:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 1) = ctx->x[1];
    glabel1587:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 1) = ctx->x[1];
    glabel1588:
    ctx->x[8] = ctx->x[8] + 31;
    glabel1589:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v2150, 8, 1) = ctx->x[1];
    glabel1590:
    ctx->x[8] = ctx->x[8] + 31;
    glabel1591:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v2152, 8, 1) = ctx->x[1];
    glabel1592:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel0;
bb_306:
    glabel1593:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[2]) < (int64_t)(9)) << 31) | ((uint32_t)((uint64_t)(ctx->x[2]) == (uint64_t)(9)) << 30) | ((uint32_t)((uint64_t)(ctx->x[2]) >= (uint64_t)(9)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[2]) ^ (uint64_t)(9)) & ((uint64_t)(ctx->x[2]) ^ ((uint64_t)(ctx->x[2]) - (uint64_t)(9)))) >> 63) << 28));
    glabel1594:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 1) = ctx->x[1];
    glabel1595:
    ctx->x[8] = ctx->x[8] + 31;
    glabel1596:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v2158, 8, 1) = ctx->x[1];
    glabel1597:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel0;
bb_307:
    glabel1598:
    ctx->x[0] = 31 - ctx->x[0];
    glabel1599:
    /* unimplemented opcode */
    glabel1600:
    /* unimplemented opcode */
    glabel1601:
    /* unimplemented opcode */
    glabel1602:
    ctx->x[2] = ctx->x[2] - ctx->x[8];
    glabel1603:
    ctx->x[0] = ctx->x[0] + ctx->x[8];
    glabel1604:
    ctx->x[9] = ctx->x[9] * ctx->x[10];
    glabel1605:
    ctx->x[0] = v2168 + 31;
    glabel1606:
    /* unimplemented opcode */
    glabel1607:
    ctx->x[8] = ctx->x[8] + ctx->x[10];
    glabel1608:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[10]) < (int64_t)(9)) << 31) | ((uint32_t)((uint64_t)(ctx->x[10]) == (uint64_t)(9)) << 30) | ((uint32_t)((uint64_t)(ctx->x[10]) >= (uint64_t)(9)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[10]) ^ (uint64_t)(9)) & ((uint64_t)(ctx->x[10]) ^ ((uint64_t)(ctx->x[10]) - (uint64_t)(9)))) >> 63) << 28));
    glabel1609:
    *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[8], 8, 1) = ctx->x[9];
    glabel1610:
    ctx->x[11] = ctx->x[11] + 31;
    glabel1611:
    *(uint64_t*)sr_rt_guest_to_host(ctx, v2174, 8, 1) = ctx->x[9];
    glabel1612:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel0;
bb_308:
    glabel1613:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[10]) < (int64_t)(25)) << 31) | ((uint32_t)((uint64_t)(ctx->x[10]) == (uint64_t)(25)) << 30) | ((uint32_t)((uint64_t)(ctx->x[10]) >= (uint64_t)(25)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[10]) ^ (uint64_t)(25)) & ((uint64_t)(ctx->x[10]) ^ ((uint64_t)(ctx->x[10]) - (uint64_t)(25)))) >> 63) << 28));
    glabel1614:
    /* unimplemented opcode */
    glabel1615:
    /* unimplemented opcode */
    glabel1616:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel0;
bb_309:
    glabel1617:
    /* unimplemented opcode */
    glabel1618:
    /* unimplemented opcode */
    glabel1619:
    /* unimplemented opcode */
    glabel1620:
    /* unimplemented opcode */
    glabel1621:
    ctx->x[10] = ctx->x[10] - ctx->x[12];
    glabel1622:
    /* unimplemented opcode */
    glabel1623:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[10]) < (int64_t)(32)) << 31) | ((uint32_t)((uint64_t)(ctx->x[10]) == (uint64_t)(32)) << 30) | ((uint32_t)((uint64_t)(ctx->x[10]) >= (uint64_t)(32)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[10]) ^ (uint64_t)(32)) & ((uint64_t)(ctx->x[10]) ^ ((uint64_t)(ctx->x[10]) - (uint64_t)(32)))) >> 63) << 28));
    glabel1624:
    /* unimplemented opcode */
    glabel1625:
    if ((((ctx->nzcv >> 29) & 1u) == 0)) goto glabel0;
bb_310:
    glabel1626:
    ctx->x[0] = 31 | ctx->x[9];
    glabel1627:
    ctx->x[8] = ctx->x[8] + ctx->x[12];
    glabel1628:
    /* unimplemented opcode */
    glabel1629:
    /* unimplemented opcode */
    glabel1630:
    ctx->x[10] = ctx->x[10] - 32;
    glabel1631:
    /* unimplemented opcode */
    glabel1632:
    ctx->x[8] = ctx->x[8] + 32;
    glabel1633:
    ctx->nzcv = (uint32_t)(((uint32_t)((int64_t)(ctx->x[10]) < (int64_t)(31)) << 31) | ((uint32_t)((uint64_t)(ctx->x[10]) == (uint64_t)(31)) << 30) | ((uint32_t)((uint64_t)(ctx->x[10]) >= (uint64_t)(31)) << 29) | ((uint32_t)((((uint64_t)(ctx->x[10]) ^ (uint64_t)(31)) & ((uint64_t)(ctx->x[10]) ^ ((uint64_t)(ctx->x[10]) - (uint64_t)(31)))) >> 63) << 28));
    glabel1634:
    if ((((ctx->nzcv >> 29) & 1u) && (((ctx->nzcv >> 30) & 1u) == 0))) goto glabel3;
bb_311:
    glabel1635:
    return;
bb_312:
    glabel1636:
    ctx->x[0] = 31 | ctx->x[0];
    glabel1637:
    /* unimplemented opcode */
    glabel1638:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_313:
    glabel1639:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[0], 8, 0);
    glabel1640:
    if (ctx->x[8]) goto bb_1; else goto bb_0;
bb_314:
    glabel1641:
    ctx->x[0] = ctx->x[0] + 1;
    glabel1642:
    /* unimplemented opcode */
    glabel1643:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_315:
    glabel1644:
    ctx->x[8] = ctx->x[8] + ctx->x[0];
    glabel1645:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v2219, 8, 0);
    glabel1646:
    if (ctx->x[9]) goto bb_1; else goto bb_0;
bb_316:
    glabel1647:
    ctx->x[8] = ctx->x[8] - 1;
    glabel1648:
    ctx->x[8] = ctx->x[8] - ctx->x[0];
    glabel1649:
    return;
bb_317:
    glabel1650:
    /* unimplemented opcode */
    glabel1651:
    ctx->x[8] = ctx->x[8] - 8;
    glabel1652:
    ctx->x[9] = ctx->x[9] & 18446744073709486080ULL;
    glabel1653:
    ctx->x[0] = v2228 | 65279;
    glabel1654:
    ctx->x[8] = ctx->x[8] + ctx->x[0];
    glabel1655:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v2231, 8, 0);
    glabel1656:
    ctx->x[10] = ctx->x[10] + ctx->x[9];
    glabel1657:
    /* unimplemented opcode */
    glabel1658:
    /* unimplemented opcode */
    glabel1659:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel4;
bb_318:
    glabel1660:
    /* unimplemented opcode */
    glabel1661:
    if (((ctx->nzcv >> 30) & 1u)) goto glabel0;
bb_319:
    glabel1662:
    ctx->x[8] = ctx->x[8] + ctx->x[0];
    glabel1663:
    ctx->x[0] = *(uint64_t*)sr_rt_guest_to_host(ctx, v2241, 8, 0);
    glabel1664:
    if (ctx->x[9]) goto bb_1; else goto bb_0;
bb_320:
    glabel1665:
    ctx->x[8] = ctx->x[8] - ctx->x[0];
    glabel1666:
    return;
bb_321:
    glabel1667:
    ctx->x[0] = ctx->x[0] - ctx->x[0];
    glabel1668:
    return;
bb_322:
    glabel1669:
    /* unimplemented opcode */
    glabel1670:
    /* unimplemented opcode */
    glabel1671:
    /* unimplemented opcode */
    glabel1672:
    /* unimplemented opcode */
    glabel1673:
    ctx->x[0] = 485331316736 + 8192;
    glabel1674:
    ctx->x[16] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[16], 8, 0);
    glabel1675:
    ctx->x[16] = ctx->x[16] + 304;
    glabel1676:
    /* unimplemented opcode */
    glabel1677:
    /* nop */
    glabel1678:
    /* nop */
    glabel1679:
    /* nop */
    glabel1680:
    ctx->x[0] = 485331316736 + 8192;
    glabel1681:
    ctx->x[16] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[16], 8, 0);
    glabel1682:
    ctx->x[16] = ctx->x[16] + 312;
    glabel1683:
    /* unimplemented opcode */
    glabel1684:
    ctx->x[0] = 485331316736 + 8192;
    glabel1685:
    ctx->x[16] = *(uint64_t*)sr_rt_guest_to_host(ctx, ctx->x[16], 8, 0);
    glabel1686:
    ctx->x[16] = ctx->x[16] + 320;
    glabel1687:
    /* unimplemented opcode */
}

