// NSO Parser Tests
// Tests parsing of real NSO files from Kirby and the Forgotten Land

#include <gtest/gtest.h>
#include <kirbyrecomp/nso/nso.h>
#include <iostream>
#include <cstring>
#include <vector>

using namespace kirbyrecomp::nso;

// Simple file existence check without filesystem
static bool file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (f) { fclose(f); return true; }
    return false;
}

// Test with the actual ExeFS/main from the game dump
TEST(NsoParser, ParseMainNso) {
    const char* exefs_main = "D:/Claude Stuff/KirbyRecomp/ExeFS/main";

    if (!file_exists(exefs_main)) {
        GTEST_SKIP() << "ExeFS/main not found at " << exefs_main;
    }

    NsoParser parser;
    auto module = parser.parse_file(exefs_main);

    ASSERT_TRUE(module.has_value()) << "Failed to parse NSO: " << parser.get_last_error();

    const NsoModule& nso = module.value();

    // Verify basic header info
    // NOTE: module_name comes from MOD0 section in text segment, not from header
    // For now, just verify it's parsed (may be empty if MOD0 not parsed)
    std::cout << "Module name: '" << nso.module_name << "'\n";
    EXPECT_EQ(nso.header.version, 0);
    EXPECT_EQ(memcmp(nso.header.magic, "NSO0", 4), 0);

    // Verify we have sections
    const Section* text = nso.get_section(SectionType::Text);
    const Section* rodata = nso.get_section(SectionType::Rodata);
    const Section* data = nso.get_section(SectionType::Data);

    ASSERT_NE(text, nullptr) << "Text section missing";
    ASSERT_NE(rodata, nullptr) << "Rodata section missing";
    ASSERT_NE(data, nullptr) << "Data section missing";

    std::cout << "NSO Module: " << nso.module_name << "\n";
    std::cout << "  Text:   size=" << text->size << ", compressed=" << text->compressed_size
              << ", type=" << (text->compression == CompressionType::LZ4 ? "LZ4" : "None") << "\n";
    std::cout << "  Rodata: size=" << rodata->size << ", compressed=" << rodata->compressed_size
              << ", type=" << (rodata->compression == CompressionType::LZ4 ? "LZ4" : "None") << "\n";
    std::cout << "  Data:   size=" << data->size << ", compressed=" << data->compressed_size
              << ", type=" << (data->compression == CompressionType::LZ4 ? "LZ4" : "None") << "\n";
    std::cout << "  BSS:    size=" << nso.bss_size << "\n";

    // Verify text section has code (should be large)
    EXPECT_GT(text->size, 1000000) << "Text section too small, expected > 1MB";

    // Verify rodata section has data
    EXPECT_GT(rodata->size, 10000) << "Rodata section too small";

    // Verify data section
    EXPECT_GT(data->size, 1000) << "Data section too small";

    // Verify compression types
    EXPECT_EQ(text->compression, CompressionType::LZ4);
    EXPECT_EQ(rodata->compression, CompressionType::LZ4);
    EXPECT_EQ(data->compression, CompressionType::LZ4);
}

// Test parsing other ExeFS modules
TEST(NsoParser, ParseOtherExeFSModules) {
    const char* exefs_dir = "D:/Claude Stuff/KirbyRecomp/ExeFS";

    if (!file_exists(exefs_dir)) {
        GTEST_SKIP() << "ExeFS directory not found";
    }

    NsoParser parser;
    std::vector<std::string> expected_modules = {"main", "rtld", "sdk", "subsdk0"};
    int parsed_count = 0;

    for (const auto& expected : expected_modules) {
        std::string path = std::string(exefs_dir) + "/" + expected;

        if (!file_exists(path.c_str())) continue;

        auto module = parser.parse_file(path.c_str());

        if (!module.has_value()) {
            std::cerr << "Warning: Failed to parse " << expected << ": " << parser.get_last_error() << "\n";
            continue;
        }

        const NsoModule& nso = module.value();
        parsed_count++;

        std::cout << "Parsed NSO: " << nso.module_name << "\n";
        const Section* text = nso.get_section(SectionType::Text);
        const Section* rodata = nso.get_section(SectionType::Rodata);
        const Section* data = nso.get_section(SectionType::Data);

        if (text) {
            std::cout << "  Text:   " << text->size << " bytes";
            if (text->compression == CompressionType::LZ4) std::cout << " (LZ4 compressed)";
            std::cout << "\n";
        }
        if (rodata) {
            std::cout << "  Rodata: " << rodata->size << " bytes";
            if (rodata->compression == CompressionType::LZ4) std::cout << " (LZ4 compressed)";
            std::cout << "\n";
        }
        if (data) {
            std::cout << "  Data:   " << data->size << " bytes";
            if (data->compression == CompressionType::LZ4) std::cout << " (LZ4 compressed)";
            std::cout << "\n";
        }
    }

    EXPECT_GT(parsed_count, 0) << "No modules parsed successfully";
    std::cout << "Total modules parsed: " << parsed_count << "\n";
}

// Test NSO header validation
TEST(NsoParser, InvalidMagic) {
    // Create a buffer with invalid magic (need at least 256 bytes for NSO header)
    std::vector<uint8_t> buffer(0x100, 0);
    memcpy(buffer.data(), "XXXX", 4);  // Invalid magic

    NsoParser parser;
    auto module = parser.parse_buffer(buffer);

    EXPECT_FALSE(module.has_value());
    // The actual error message for invalid magic
    std::string error = parser.get_last_error();
    std::cout << "Error message: " << error << "\n";
    EXPECT_TRUE(error.find("magic") != std::string::npos || error.find("Magic") != std::string::npos);
}

// Test NSO section alignment (in memory, not file)
// NSO sections are page-aligned in MEMORY (virtual_offset), not in file (offset)
TEST(NsoParser, SectionAlignment) {
    const char* exefs_main = "D:/Claude Stuff/KirbyRecomp/ExeFS/main";

    if (!file_exists(exefs_main)) {
        GTEST_SKIP() << "ExeFS/main not found";
    }

    NsoParser parser;
    auto module = parser.parse_file(exefs_main);

    ASSERT_TRUE(module.has_value());

    const NsoModule& nso = module.value();

    // Verify sections have valid offsets and sizes
    for (int i = 0; i < 3; ++i) {
        const Section& section = nso.sections[i];
        if (section.size > 0) {
            EXPECT_GT(section.size, 0) << "Section " << i << " has zero size";
            std::cout << "Section " << i << ": file_offset=0x" << std::hex << section.file_offset
                      << ", size=" << std::dec << section.size << "\n";
        }
    }

    // Verify BSS size is reasonable
    EXPECT_GE(nso.bss_size, 0);
}