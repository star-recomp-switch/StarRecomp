// ASTC to BC7 Transcoding Tests
// Tests the complete ASTC decode -> RGBA -> BC7 encode pipeline

#include <gtest/gtest.h>
#include <cstring>
#include <vector>

#include <kirbyrecomp/nvn/astc_decoder.h>
#include <kirbyrecomp/nvn/bc7_encoder.h>

namespace kn_bc7 = kirbyrecomp::nvn::bc7;
namespace kn_astc = kirbyrecomp::nvn::astc;

// Test helper: create a void extent ASTC block
static void create_void_extent_block(uint8_t* block, uint16_t r, uint16_t g, uint16_t b, uint16_t a) {
    // Block mode: 0x1F (void extent) - 11 bits = 0x01F
    // In LSB-first: 0x1F, 0x00
    block[0] = 0xFF;
    block[1] = 0x0F;
    // Color values (16-bit each, little-endian)
    block[2] = r & 0xFF;
    block[3] = (r >> 8) & 0xFF;
    block[4] = g & 0xFF;
    block[5] = (g >> 8) & 0xFF;
    block[6] = b & 0xFF;
    block[7] = (b >> 8) & 0xFF;
    block[8] = a & 0xFF;
    block[9] = (a >> 8) & 0xFF;
    // Padding
    for (int i = 10; i < 16; ++i) block[i] = 0;
}

// Test the complete ASTC decode -> BC7 encode pipeline for void extent blocks
TEST(AstcBc7Transcoding, VoidExtent4x4ToBC7) {
    // Create a 4x4 ASTC void extent block (red)
    uint8_t astc_block[16];
    create_void_extent_block(astc_block, 0xFFFF, 0x0000, 0x0000, 0xFFFF);

    // Decode ASTC to RGBA
    uint8_t rgba[4 * 4 * 4];
    bool result = kn_astc::decode_block(astc_block, 4, 4, rgba, 16);
    ASSERT_TRUE(result) << "ASTC decode failed";

    // Verify decoded RGBA
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            uint8_t* pixel = rgba + y * 16 + x * 4;
            EXPECT_EQ(pixel[0], 0xFF) << "R at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[1], 0x00) << "G at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[2], 0x00) << "B at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[3], 0xFF) << "A at (" << x << "," << y << ")";
        }
    }

    // Encode RGBA to BC7 (using the internal BC7 encoder)
    kn_bc7::BC7Block bc7_block;
    kn_bc7::encode_mode6(rgba, 4, 4, 16, bc7_block);

    // Verify BC7 block was generated (16 bytes)
    EXPECT_NE(0, memcmp(bc7_block.data, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16))
        << "BC7 block should not be all zeros";
}

// Test 8x8 void extent block transcoding
TEST(AstcBc7Transcoding, VoidExtent8x8ToBC7) {
    // Create an 8x8 ASTC void extent block (blue)
    uint8_t astc_block[16];
    create_void_extent_block(astc_block, 0x0000, 0x0000, 0xFFFF, 0xFFFF);

    // Decode ASTC to RGBA (8x8 = 64 pixels)
    uint8_t rgba[8 * 8 * 4];
    bool result = kirbyrecomp::nvn::astc::decode_block(astc_block, 8, 8, rgba, 32);
    ASSERT_TRUE(result) << "ASTC decode failed";

    // Verify decoded RGBA
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            uint8_t* pixel = rgba + y * 32 + x * 4;
            EXPECT_EQ(pixel[0], 0x00) << "R at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[1], 0x00) << "G at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[2], 0xFF) << "B at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[3], 0xFF) << "A at (" << x << "," << y << ")";
        }
    }

    // Encode to BC7 - 8x8 requires 4 BC7 blocks (2x2 grid of 4x4)
    for (int by = 0; by < 2; ++by) {
        for (int bx = 0; bx < 2; ++bx) {
            uint8_t block_rgba[4 * 4 * 4];
            // Extract 4x4 region
            for (int y = 0; y < 4; ++y) {
                for (int x = 0; x < 4; ++x) {
                    int src_x = bx * 4 + x;
                    int src_y = by * 4 + y;
                    const uint8_t* src = rgba + src_y * 32 + src_x * 4;
                    uint8_t* dst = block_rgba + y * 16 + x * 4;
                    dst[0] = src[0];
                    dst[1] = src[1];
                    dst[2] = src[2];
                    dst[3] = src[3];
                }
            }

            kn_bc7::BC7Block bc7_block;
            kn_bc7::encode_mode6(block_rgba, 4, 4, 16, bc7_block);

            // Verify BC7 block was generated
            EXPECT_NE(0, memcmp(bc7_block.data, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16))
                << "BC7 block at (" << bx << "," << by << ") should not be all zeros";
        }
    }
}

// Test 12x12 void extent block transcoding
TEST(AstcBc7Transcoding, VoidExtent12x12ToBC7) {
    // Create a 12x12 ASTC void extent block (white)
    uint8_t astc_block[16];
    create_void_extent_block(astc_block, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);

    // Decode ASTC to RGBA (12x12 = 144 pixels)
    uint8_t rgba[12 * 12 * 4];
    bool result = kirbyrecomp::nvn::astc::decode_block(astc_block, 12, 12, rgba, 48);
    ASSERT_TRUE(result) << "ASTC decode failed";

    // Verify decoded RGBA
    for (int y = 0; y < 12; ++y) {
        for (int x = 0; x < 12; ++x) {
            uint8_t* pixel = rgba + y * 48 + x * 4;
            EXPECT_EQ(pixel[0], 0xFF) << "R at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[1], 0xFF) << "G at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[2], 0xFF) << "B at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[3], 0xFF) << "A at (" << x << "," << y << ")";
        }
    }

    // Encode to BC7 - 12x12 requires 9 BC7 blocks (3x3 grid of 4x4)
    for (int by = 0; by < 3; ++by) {
        for (int bx = 0; bx < 3; ++bx) {
            uint8_t block_rgba[4 * 4 * 4];
            // Extract 4x4 region
            for (int y = 0; y < 4; ++y) {
                for (int x = 0; x < 4; ++x) {
                    int src_x = bx * 4 + x;
                    int src_y = by * 4 + y;
                    if (src_x < 12 && src_y < 12) {
                        const uint8_t* src = rgba + src_y * 48 + src_x * 4;
                        uint8_t* dst = block_rgba + y * 16 + x * 4;
                        dst[0] = src[0];
                        dst[1] = src[1];
                        dst[2] = src[2];
                        dst[3] = src[3];
                    } else {
                        // Pad with last pixel
                        const uint8_t* src = rgba + 11 * 48 + 11 * 4;
                        uint8_t* dst = block_rgba + y * 16 + x * 4;
                        dst[0] = src[0];
                        dst[1] = src[1];
                        dst[2] = src[2];
                        dst[3] = src[3];
                    }
                }
            }

            kn_bc7::BC7Block bc7_block;
            kn_bc7::encode_mode6(block_rgba, 4, 4, 16, bc7_block);

            // Verify BC7 block was generated
            EXPECT_NE(0, memcmp(bc7_block.data, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16))
                << "BC7 block at (" << bx << "," << by << ") should not be all zeros";
        }
    }
}

// Test ASTC->BC7 round-trip for non-void-extent block (if decoder supports it)
TEST(AstcBc7Transcoding, InvalidBlockModeHandledGracefully) {
    // Create an invalid block (all zeros)
    uint8_t invalid_block[16] = {0};

    uint8_t rgba[4 * 4 * 4];
    bool result = kirbyrecomp::nvn::astc::decode_block(invalid_block, 4, 4, rgba, 16);
    EXPECT_FALSE(result) << "Invalid block should fail to decode";

    // Output should be unchanged (or zeroed)
    // The decoder should not crash
}

// Test different colors in void extent blocks
TEST(AstcBc7Transcoding, DifferentColors) {
    struct TestColor {
        uint16_t r, g, b, a;
        const char* name;
    };

    TestColor colors[] = {
        {0xFFFF, 0x0000, 0x0000, 0xFFFF, "Red"},
        {0x0000, 0xFFFF, 0x0000, 0xFFFF, "Green"},
        {0x0000, 0x0000, 0xFFFF, 0xFFFF, "Blue"},
        {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, "White"},
        {0x0000, 0x0000, 0x0000, 0xFFFF, "Black"},
        {0xFFFF, 0xFFFF, 0x0000, 0xFFFF, "Yellow"},
        {0xFFFF, 0x0000, 0xFFFF, 0xFFFF, "Magenta"},
        {0x0000, 0xFFFF, 0xFFFF, 0xFFFF, "Cyan"},
    };

    for (const auto& color : colors) {
        uint8_t astc_block[16];
        create_void_extent_block(astc_block, color.r, color.g, color.b, color.a);

        uint8_t rgba[4 * 4 * 4];
        bool result = kirbyrecomp::nvn::astc::decode_block(astc_block, 4, 4, rgba, 16);
        ASSERT_TRUE(result) << "Failed to decode " << color.name;

        // Verify all pixels have the expected color
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                uint8_t* pixel = rgba + y * 16 + x * 4;
                // High byte of 16-bit value
                EXPECT_EQ(pixel[0], (color.r >> 8) & 0xFF) << color.name << " R at (" << x << "," << y << ")";
                EXPECT_EQ(pixel[1], (color.g >> 8) & 0xFF) << color.name << " G at (" << x << "," << y << ")";
                EXPECT_EQ(pixel[2], (color.b >> 8) & 0xFF) << color.name << " B at (" << x << "," << y << ")";
                EXPECT_EQ(pixel[3], (color.a >> 8) & 0xFF) << color.name << " A at (" << x << "," << y << ")";
            }
        }

        // Test BC7 encoding
        kn_bc7::BC7Block bc7_block;
        kn_bc7::encode_mode6(rgba, 4, 4, 16, bc7_block);
        EXPECT_NE(0, memcmp(bc7_block.data, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16))
            << "BC7 encoding failed for " << color.name;
    }
}