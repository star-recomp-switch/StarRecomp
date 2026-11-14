// Debug hash verification

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
#include <array>
#include <lz4.h>
#include <openssl/sha.h>

static std::array<uint8_t, 32> sha256(const uint8_t* data, size_t len) {
    std::array<uint8_t, 32> hash{};
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(hash.data(), &ctx);
    return hash;
}

int main() {
    const char* path = "D:/Claude Stuff/KirbyRecomp/ExeFS/main";
    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("Failed to open\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    std::vector<uint8_t> buffer(file_size);
    fread(buffer.data(), 1, file_size, f);
    fclose(f);

    const uint8_t* base = buffer.data();

    // Read section info from header
    for (int i = 0; i < 3; ++i) {
        const uint8_t* hdr = buffer.data() + 0x10 + i * 0x10;
        uint32_t file_off = *reinterpret_cast<const uint32_t*>(hdr);
        uint32_t mem_off = *reinterpret_cast<const uint32_t*>(hdr + 4);
        uint32_t dec_size = *reinterpret_cast<const uint32_t*>(hdr + 8);
        uint32_t comp_size = *reinterpret_cast<const uint32_t*>(buffer.data() + 0x60 + i * 4);

        const char* names[] = {"Text", "Rodata", "Data"};
        printf("Section %s: file_off=0x%x, dec_size=%u, comp_size=%u\n",
               names[i], file_off, dec_size, comp_size);

        if (comp_size > 0) {
            const uint8_t* src = buffer.data() + file_off;
            std::vector<uint8_t> dst(dec_size);
            int result = LZ4_decompress_safe(
                reinterpret_cast<const char*>(src),
                reinterpret_cast<char*>(dst.data()),
                comp_size,
                dec_size
            );

            if (result >= 0) {
                printf("  Decompress: OK (dst_size=%d)\n", result);

                // Compute hash of decompressed data
                auto hash = sha256(dst.data(), dec_size);

                // Read expected hash from header
                const uint8_t* expected_hash = buffer.data() + 0x80 + i * 32;

                bool match = memcmp(hash.data(), expected_hash, 32) == 0;
                printf("  Hash: %s\n", match ? "MATCH" : "MISMATCH");

                if (!match) {
                    printf("  Expected: ");
                    for (int j = 0; j < 32; ++j) printf("%02x", buffer.data()[0x80 + i * 32 + j]);
                    printf("\n  Got:      ");
                    for (int j = 0; j < 32; ++j) printf("%02x", hash[j]);
                    printf("\n");
                }

                // Also check if hash is of compressed data
                auto hash_comp = sha256(buffer.data() + *reinterpret_cast<const uint32_t*>(buffer.data() + 0x10 + i * 0x10), comp_size);
                bool match_comp = memcmp(hash_comp.data(), buffer.data() + 0x80 + i * 32, 32) == 0;
                printf("  Hash of compressed: %s\n", match_comp ? "MATCH" : "MISMATCH");
            } else {
                printf("  Decompress: FAILED (%d)\n", result);
            }
        }
    }

    return 0;
}