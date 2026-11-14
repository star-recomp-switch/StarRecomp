// Debug NSO header parsing

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>

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
    printf("File size: %ld bytes\n", file_size);

    std::vector<uint8_t> buffer(file_size);
    fread(buffer.data(), 1, file_size, f);
    fclose(f);

    const uint8_t* data = buffer.data();
    size_t pos = 0;

    // Read magic
    char magic[5] = {0};
    memcpy(magic, data + pos, 4);
    pos += 4;
    printf("Magic: %s\n", magic);

    // Version
    uint32_t version = *reinterpret_cast<const uint32_t*>(data + pos);
    pos += 4;
    printf("Version: %u\n", version);

    // Flags
    uint32_t flags = *reinterpret_cast<const uint32_t*>(data + pos);
    pos += 4;
    printf("Flags: 0x%x\n", flags);

    // Module name offset
    uint64_t module_name_offset = *reinterpret_cast<const uint64_t*>(data + pos);
    pos += 8;
    printf("Module name offset: %llu\n", module_name_offset);

    // Module name size
    uint32_t module_name_size = *reinterpret_cast<const uint32_t*>(data + pos);
    pos += 4;
    printf("Module name size: %u\n", module_name_size);

    // Reserved1
    uint32_t reserved1 = *reinterpret_cast<const uint32_t*>(data + pos);
    pos += 4;
    printf("Reserved1: %u\n", reserved1);

    // Sections
    const char* names[] = {"Text", "Rodata", "Data"};
    for (int i = 0; i < 3; ++i) {
        uint64_t offset = *reinterpret_cast<const uint64_t*>(data + pos);
        pos += 8;
        uint64_t size = *reinterpret_cast<const uint64_t*>(data + pos);
        pos += 8;
        uint64_t compressed_size = *reinterpret_cast<const uint64_t*>(data + pos);
        pos += 8;
        printf("Section %s: offset=%llu, size=%llu, compressed_size=%llu\n",
               names[i], offset, size, compressed_size);
    }

    // Skip remaining header
    pos += 32 + 32 + 32 * 3;

    printf("\nFinal pos: %zu\n", pos);

    // Read module name
    if (module_name_size > 0) {
        char* name = new char[module_name_size + 1];
        memcpy(name, buffer.data() + pos, module_name_size);
        name[module_name_size] = '\0';
        printf("Module name: %s\n", name);
        delete[] name;
    }

    return 0;
}