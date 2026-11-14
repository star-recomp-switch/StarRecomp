// Debug NSO header

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>

#pragma pack(push, 1)
struct NsoHeader {
    char magic[4];
    uint32_t version;
    uint32_t flags;
    uint64_t module_name_offset;
    uint32_t module_name_size;
    uint32_t reserved1;
    struct SectionInfo {
        uint64_t offset;
        uint64_t size;
        uint64_t compressed_size;
    } sections[3];
    uint8_t build_id[32];
    uint8_t reserved2[32];
    uint8_t text_hash[32];
    uint8_t rodata_hash[32];
    uint8_t data_hash[32];
};
#pragma pack(pop)

int main() {
    const char* path = "D:/Claude Stuff/KirbyRecomp/ExeFS/main";
    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("Failed to open %s\n", path);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    printf("File size: %ld bytes (%.2f MB)\n", file_size, file_size / 1024.0 / 1024.0);

    std::vector<uint8_t> buffer(256);
    fread(buffer.data(), 1, 256, f);
    fclose(f);

    NsoHeader header;
    memcpy(&header, buffer.data(), sizeof(NsoHeader));

    printf("Magic: %.4s\n", header.magic);
    printf("Version: %u\n", header.version);
    printf("Flags: 0x%x\n", header.flags);
    printf("Module name offset: %llu\n", header.module_name_offset);
    printf("Module name size: %u\n", header.module_name_size);

    const char* section_names[] = {"Text", "Rodata", "Data"};
    for (int i = 0; i < 3; ++i) {
        printf("\nSection %s:\n", section_names[i]);
        printf("  Offset: %llu (0x%llx)\n", header.sections[i].offset, header.sections[i].offset);
        printf("  Size: %llu (0x%llx)\n", header.sections[i].size, header.sections[i].size);
        printf("  Compressed size: %llu (0x%llx)\n", header.sections[i].compressed_size, header.sections[i].compressed_size);
    }

    // Check if any section exceeds file size
    for (int i = 0; i < 3; ++i) {
        uint64_t end = header.sections[i].offset + header.sections[i].compressed_size;
        if (end > (uint64_t)file_size) {
            printf("\nWARNING: Section %s exceeds file size! End offset: %llu, File size: %ld\n",
                   section_names[i], end, file_size);
        }
    }

    return 0;
}