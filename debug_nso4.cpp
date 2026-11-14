// Debug LZ4 decompression

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>

bool lz4_decompress(const uint8_t* src, size_t src_size,
                    uint8_t* dst, size_t dst_capacity, size_t& dst_size) {
    size_t src_pos = 0;
    size_t dst_pos = 0;

    while (src_pos < src_size) {
        if (dst_pos >= dst_capacity) return false;

        uint8_t token = src[src_pos++];
        uint8_t literal_len = token >> 4;

        // Copy literals
        if (literal_len == 15) {
            while (src_pos < src_size && src[src_pos] == 255) {
                literal_len += 255;
                src_pos++;
            }
            if (src_pos < src_size) literal_len += src[src_pos++];
        }

        if (src_pos + literal_len > src_size) return false;
        if (dst_pos + literal_len > dst_capacity) return false;

        std::memcpy(dst + dst_pos, src + src_pos, literal_len);
        src_pos += literal_len;
        dst_pos += literal_len;

        if (src_pos >= src_size) break;

        // Match copy
        if (src_pos + 2 > src_size) return false;
        uint16_t offset = static_cast<uint16_t>(src[src_pos] | (src[src_pos + 1] << 8));
        src_pos += 2;

        if (offset == 0 || offset > dst_pos) return false;

        uint8_t match_len = token & 0xF;
        if (match_len == 15) {
            while (src_pos < src_size && src[src_pos] == 255) {
                match_len += 255;
                src_pos++;
            }
            if (src_pos < src_size) match_len += src[src_pos++];
        }
        match_len += 4;

        if (dst_pos + match_len > dst_capacity) return false;

        size_t src_copy = dst_pos - offset;
        for (size_t i = 0; i < match_len; ++i) {
            dst[dst_pos++] = dst[src_copy++];
        }
    }

    dst_size = dst_pos;
    return true;
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
        printf("Section %s: file_off=0x%x, mem_off=0x%x, dec_size=%u, comp_size=%u\n",
               names[i], file_off, mem_off, dec_size, comp_size);

        if (comp_size > 0 && comp_size != dec_size) {
            // Try decompress
            const uint8_t* src = buffer.data() + file_off;
            std::vector<uint8_t> dst(dec_size);
            size_t dst_size;
            bool ok = lz4_decompress(src, comp_size, dst.data(), dec_size, dst_size);
            printf("  Decompress: %s (src_size=%u, dst_size=%zu)\n", ok ? "OK" : "FAILED", comp_size, dst_size);

            if (ok) {
                // Check hash
                // Just print first few bytes
                printf("  First 16 bytes: ");
                for (int j = 0; j < 16 && j < (int)dst.size(); ++j) {
                    printf("%02x ", dst[j]);
                }
                printf("\n");
            }
        }
    }

    return 0;
}