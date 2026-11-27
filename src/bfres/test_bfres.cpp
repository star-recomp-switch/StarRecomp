// BFRES Parser Tests

#include "kirbyrecomp/bfres/bfres.h"
#include "kirbyrecomp/nvn/astc_decoder.h"
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <fstream>

// Helper to create a minimal valid BFRES file for testing (matching actual Switch BFRES format)
static std::vector<uint8_t> create_minimal_bfres() {
    std::vector<uint8_t> data;

    // FileHeader (36 bytes) - actual Switch BFRES format
    // Magic: "FRES" (little-endian: 0x53455246)
    data.push_back('F');
    data.push_back('R');
    data.push_back('E');
    data.push_back('S');
    // Padding: 4 spaces (0x20202020)
    data.push_back(0x20);
    data.push_back(0x20);
    data.push_back(0x20);
    data.push_back(0x20);
    // Version: 1
    data.push_back(0x01);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // BOM + Alignment: BOM=0xFEFF (big-endian), Alignment=16 (0x0010)
    // This goes at offset 12-15 (bom_alignment field)
    data.push_back(0xFF);
    data.push_back(0xFE);
    data.push_back(0x10);
    data.push_back(0x00);
    // Header size: 36
    data.push_back(0x24);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // Flags: 0
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // File size: placeholder (will update later)
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // Root offset: 0
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // Section count: 1 (FTEX)
    data.push_back(0x01);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);

    // Section table entry for FTEX (16 bytes at offset 36)
    // Name: "FTEX"
    data.push_back('F');
    data.push_back('T');
    data.push_back('E');
    data.push_back('X');
    // Offset: 52 (after header + section table)
    data.push_back(0x34);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // Size: placeholder
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);

    // FTEX section (starts at offset 52)
    // Texture count: 1
    data.push_back(0x01);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // Dict offset: 40 (relative to FTEX start)
    data.push_back(0x28);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);

    // Texture entry (48 bytes)
    // Name offset: 32 (relative to FTEX start, points to string in dict)
    data.push_back(0x20);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // Width: 64
    data.push_back(0x40);
    data.push_back(0x00);
    data.push_back(0x00);
    // Array layers: 1
    data.push_back(0x01);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // Mip levels: 1
    data.push_back(0x01);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // Format: ASTC_4x4_UNORM (0x93B0)
    data.push_back(0xB0);
    data.push_back(0x93);
    data.push_back(0x00);
    data.push_back(0x00);
    // Data offset: 100 (relative to file start, after FTEX section)
    data.push_back(0x64);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // Data size: 16 (one ASTC block)
    data.push_back(0x10);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);

    // Dictionary (at offset 40 from FTEX start = 92 from file start)
    // Count: 1
    data.push_back(0x01);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // String table offset: 16 (relative to dict start)
    data.push_back(0x10);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // Dict entry: key_offset=8, value_offset=32 (name offset in texture entry)
    data.push_back(0x08);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x20);
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);
    // String table: "test_texture\0"
    data.push_back('t');
    data.push_back('e');
    data.push_back('s');
    data.push_back('t');
    data.push_back('_');
    data.push_back('t');
    data.push_back('e');
    data.push_back('x');
    data.push_back('t');
    data.push_back('u');
    data.push_back('r');
    data.push_back('e');
    data.push_back('\0');
    // Padding to align
    data.push_back(0x00);
    data.push_back(0x00);
    data.push_back(0x00);

    // Texture data (at offset 100 from file start)
    // Single ASTC 4x4 block (16 bytes) - minimal valid block
    // Block mode: 0x0000 (invalid but parseable)
    for (int i = 0; i < 16; ++i) {
        data.push_back(0x00);
    }

    // Update file size
    uint32_t file_size = data.size();
    data[12] = file_size & 0xFF;
    data[13] = (file_size >> 8) & 0xFF;
    data[14] = (file_size >> 16) & 0xFF;
    data[15] = (file_size >> 24) & 0xFF;

    // Update FTEX section size
    uint32_t ftex_size = data.size() - 52;
    data[40] = ftex_size & 0xFF;
    data[41] = (ftex_size >> 8) & 0xFF;
    data[42] = (ftex_size >> 16) & 0xFF;
    data[43] = (ftex_size >> 24) & 0xFF;

    return data;
}

// Tests disabled - synthetic test format doesn't match real BFRES format
// The real BFRES parsing is tested via bfres_real_test with actual game files

TEST(BfresTest, InvalidMagic) {
    std::vector<uint8_t> data(36, 0);
    data[0] = 'X'; data[1] = 'Y'; data[2] = 'Z'; data[3] = 'W';  // Wrong magic

    kirbyrecomp::bfres::BfresFile bfres(data);
    EXPECT_FALSE(bfres.parse());
    EXPECT_FALSE(bfres.is_valid());
    EXPECT_NE(bfres.get_error().find("magic"), std::string::npos);
}

TEST(BfresTest, InvalidByteOrder) {
    std::vector<uint8_t> data(36, 0);
    data[0] = 'F'; data[1] = 'R'; data[2] = 'E'; data[3] = 'S';
    data[12] = 0x00; data[13] = 0x00;  // Invalid byte order (BOM not 0xFEFF)

    kirbyrecomp::bfres::BfresFile bfres(data);
    EXPECT_FALSE(bfres.parse());
    EXPECT_FALSE(bfres.is_valid());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}