// Test file to verify kirbyrecomp.h compiles correctly
// Compile with: g++ -std=c++17 -I include test_kirbyrecomp.cpp -o test_kirbyrecomp

#include "kirbyrecomp/kirbyrecomp.h"
#include <cstdio>

using namespace kr;

int main() {
    // Test version functions
    printf("Version: %s\n", version_string());
    printf("Major: %d, Minor: %d, Patch: %d\n", version_major(), version_minor(), version_patch());

    // Test that numeric functions are accessible via kr namespace
    u32 bits = F32Bits(1.0f);
    f32 val = BitsF32(bits);
    printf("F32Bits/BitsF32 roundtrip: %f\n", val);

    // Test W/Zext32
    u64 zext = Zext32(0xFFFFFFFF00000000ull);
    printf("Zext32(0xFFFFFFFF00000000) = 0x%llx\n", zext);

    printf("kirbyrecomp.h test passed!\n");
    return 0;
}