#include "switchrecomp/common/numeric.h"
#include <cstdio>
int main() {
  printf("zext=%llu\n", (unsigned long long)sr::numeric::Zext32(0xFFFFFFFF00000001ull));
  printf("shl64=%llu\n", (unsigned long long)sr::numeric::Shl(1ull, 64ull, false));
  printf("shl65=%llu\n", (unsigned long long)sr::numeric::Shl(1ull, 65ull, false));
  { auto r = sr::numeric::AddWithFlags(0xFFFFFFFFull, 1ull, true); printf("add=%llu c=%u\n", (unsigned long long)r.value, (unsigned)(((r.nzcv) >> 29) & 1u)); }
  printf("cvt_nan=%lld\n", (long long)sr::numeric::FpToSigned(sr::numeric::BitsF64(0x7FF8000000000000ull), false));
  printf("cvt_sat=%lld\n", (long long)sr::numeric::FpToSigned(1e300, false));
  printf("sar=%llx\n", (unsigned long long)sr::numeric::Sar(0xFFFFFFFF80000000ull, 4ull, true));
  { auto r = sr::numeric::AddWithFlags(0x7FFFFFFFull, 1ull, true); printf("gt=%d\n", sr::numeric::ConditionHolds(r.nzcv, 12) ? 1 : 0); }
  return 0;
}
