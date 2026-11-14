// Test file to verify numeric.h compiles correctly
// Compile with: g++ -std=c++17 -I include test_numeric.cpp -o test_numeric

#include "kirbyrecomp/numeric.h"
#include <cstdio>
#include <cassert>

using namespace sr::numeric;
using namespace sr::common;

int main() {
    // Test basic type definitions
    u8  u8_val  = 0xFF;
    u16 u16_val = 0xFFFF;
    u32 u32_val = 0xFFFFFFFF;
    u64 u64_val = 0xFFFFFFFFFFFFFFFFull;
    s32 s32_val = -1;
    s64 s64_val = -1;
    f32 f32_val = 1.5f;
    f64 f64_val = 2.5;

    // Test bit casts
    u32 f32_bits = F32Bits(1.0f);
    f32 f32_back = BitsF32(f32_bits);
    assert(f32_back == 1.0f);

    u64 f64_bits = F64Bits(1.0);
    f64 f64_back = BitsF64(f64_bits);
    assert(f64_back == 1.0);

    // Test width helpers
    assert(W(0xFFFFFFFFFFFFFFFFull) == 0xFFFFFFFFu);
    assert(Zext32(0xFFFFFFFF00000000ull) == 0x00000000ull); // low 32 bits are 0
    assert(Zext32(0x00000000FFFFFFFFull) == 0xFFFFFFFFull); // low 32 bits are all 1
    assert(Sext32(0x80000000ull) == 0xFFFFFFFF80000000ull);
    assert(Sext(0x80, 8) == 0xFFFFFFFFFFFFFF80ull);

    // Test shift masking
    assert(MaskShift32(33) == 1);
    assert(MaskShift64(65) == 1);

    // Test shifts
    assert(Shl(1ull, 1, false) == 2ull);
    assert(Shr(8ull, 1, false) == 4ull);
    assert(Sar(0xFFFFFFFFFFFFFFFFull, 1, false) == 0xFFFFFFFFFFFFFFFFull);
    // 32-bit SAR: value is zero-extended first, then sign bit is bit 31
    // Result is 64-bit but upper 32 bits are zero (zero-extended W result)
    assert(Sar(0x80000000ull, 1, true) == 0xC0000000ull); // sign bit set
    assert(Sar(0x7FFFFFFFull, 1, true) == 0x3FFFFFFFull); // sign bit clear

    // Test AddWithFlags
    auto add = AddWithFlags(1ull, 2ull, false);
    assert(add.value == 3ull);
    assert((add.nzcv & (1u << 30)) == 0); // Z=0 (result not zero)
    assert((add.nzcv & (1u << 29)) == 0); // C=0 (no carry)

    auto add32 = AddWithFlags(0xFFFFFFFFull, 1ull, true);
    assert(add32.value == 0ull); // Wraps to 0
    assert((add32.nzcv & (1u << 30)) != 0); // Z=1 (result is zero)
    assert((add32.nzcv & (1u << 29)) != 0); // C=1 (carry out)

    // Test SubWithFlags
    auto sub = SubWithFlags(5ull, 3ull, false);
    assert(sub.value == 2ull);
    assert((sub.nzcv & (1u << 29)) != 0); // C=1 (no borrow)

    auto sub32 = SubWithFlags(0ull, 1ull, true);
    assert(sub32.value == 0xFFFFFFFFull); // Wraps
    assert((sub32.nzcv & (1u << 29)) == 0); // C=0 (borrow)
    // 0 - 1: sa=0, sbb=0, sr=1 -> V = (0!=0) && (1!=0) = false
    assert((sub32.nzcv & (1u << 28)) == 0); // V=0 (no overflow for 0-1)

    // Test overflow: 0x7FFFFFFF - (-1) = 0x80000000 (sign change)
    auto sub32_ov = SubWithFlags(0x7FFFFFFFull, 0xFFFFFFFFull, true);
    assert(sub32_ov.value == 0x80000000ull);
    assert((sub32_ov.nzcv & (1u << 28)) != 0); // V=1 (overflow)

    // Test LogicFlags
    u32 logic = LogicFlags(0ull, false);
    assert((logic & (1u << 30)) != 0); // Z=1

    logic = LogicFlags(5ull, false);
    assert((logic & (1u << 30)) == 0); // Z=0
    assert((logic & (1u << 31)) == 0); // N=0 (sign bit not set for 5)

    // Test N flag for negative (sign bit set in 64-bit)
    logic = LogicFlags(0x8000000000000000ull, false);
    assert((logic & (1u << 31)) != 0); // N=1 (sign bit set)

    // Test ConditionHolds
    u32 nzcv = (0u << 31) | (1u << 30) | (0u << 29) | (0u << 28); // Z=1
    assert(ConditionHolds(nzcv, 0));  // EQ
    assert(!ConditionHolds(nzcv, 1)); // NE

    nzcv = (0u << 31) | (0u << 30) | (1u << 29) | (0u << 28); // C=1
    assert(ConditionHolds(nzcv, 2));  // CS/HS
    assert(!ConditionHolds(nzcv, 3)); // CC/LO

    // Test FP bit helpers
    assert(FpSign32(-1.0f) == 1);
    assert(FpSign32(1.0f) == 0);
    assert(FpSign64(-1.0) == 1);
    assert(FpSign64(1.0) == 0);

    f32 abs_val = FpAbs32(-5.0f);
    assert(abs_val == 5.0f);

    f32 neg_val = FpNeg32(5.0f);
    assert(neg_val == -5.0f);

    // Test NaN handling
    assert(IsNan32(DefaultNan32()));
    assert(IsNan64(DefaultNan64()));
    assert(!IsNan32(1.0f));
    assert(!IsNan64(1.0));

    // Test FP compare
    u32 fp_nzcv = FpCompareNZCV(2.0, 1.0);
    assert((fp_nzcv & (1u << 29)) != 0); // C=1 (a >= b)

    fp_nzcv = FpCompareNZCV(1.0, 2.0);
    assert((fp_nzcv & (1u << 29)) == 0); // C=0 (a < b)

    fp_nzcv = FpCompareNZCV(1.0, 1.0);
    assert((fp_nzcv & (1u << 30)) != 0); // Z=1 (a == b)

    fp_nzcv = FpCompareNZCV(DefaultNan64(), 1.0);
    assert(fp_nzcv == 0); // NaN -> all flags 0

    // Test FP to unsigned integer conversion
    assert(FpToUnsigned(1.5, false) == 1ull);
    assert(FpToUnsigned(-1.5, false) == 0ull);
    assert(FpToUnsigned(0.0, false) == 0ull);
    assert(FpToUnsigned(DefaultNan64(), false) == 0ull);
    assert(FpToUnsigned(18446744073709551616.0, false) == ~0ull); // saturate to u64 max
    assert(FpToUnsigned(4294967295.0, true) == 0xFFFFFFFFull); // 32-bit saturate

    // Test FP to signed integer conversion
    assert(FpToSigned(1.5, false) == 1ll);
    assert(FpToSigned(-1.5, false) == -1ll);
    assert(FpToSigned(0.0, false) == 0ll);
    assert(FpToSigned(DefaultNan64(), false) == 0ll);
    assert(FpToSigned(9223372036854775808.0, false) == 9223372036854775807ll); // saturate
    assert(FpToSigned(-9223372036854775808.0, false) == -9223372036854775808ll); // saturate
    assert(FpToSigned(2147483648.0, true) == 2147483647ll); // 32-bit saturate

    // Test integer to FP
    f64 fp1 = IntToFp(42ull, false, false);
    assert(fp1 == 42.0);

    f64 fp2 = IntToFp(0xFFFFFFFFFFFFFFFFull, true, false);
    assert(fp2 == -1.0);

    printf("All tests passed!\n");
    return 0;
}