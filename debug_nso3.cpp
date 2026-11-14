// Debug NSO header parsing - check compressed sizes

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

    const uint8_t* base = buffer.data();

    // Check header layout
    printf("Header (first 256 bytes):\n");
    for (int i = 0; i < 256; i += 16) {
        printf("%04x: ", i);
        for (int j = 0; j < 16; ++j) {
            printf("%02x ", base[i + j]);
        }
        printf("\n");
    }

    return 0;
}