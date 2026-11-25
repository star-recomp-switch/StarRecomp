// ASTC Decoder Tests
// Tests for the ASTC LDR decoder implementation

#include <gtest/gtest.h>
#include <cstring>
#include <vector>

#include <kirbyrecomp/nvn/astc_decoder.h>

// Test void extent block (simplest ASTC block)
// Void extent block has block_mode lower 5 bits = 0x1F, followed by 4x 16-bit color values
TEST(AstcDecoder, VoidExtentBlock) {
    // Create a void extent block with red color
    // Block mode: 0x1F (void extent) - 11 bits = 0x01F
    // In little-endian: 0x1F, 0x00 for the 11 bits (0x01F = 0b11111)
    // Then 4x 16-bit color values: R, G, B, A
    // Total: 16 bytes (128 bits)
    uint8_t void_extent_block[16] = {
        0xFF, 0x0F,  // block_mode = 0x1F (0x1F in lower 11 bits, upper bits 0)
        0xFF, 0xFF,  // R = 0xFFFF
        0x00, 0x00,  // G = 0x0000
        0x00, 0x00,  // B = 0x0000
        0xFF, 0xFF,  // A = 0xFFFF
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // padding to 16 bytes
    };

    // Decode into a 4x4 block
    uint8_t output[4 * 4 * 4] = {0};
    bool result = kirbyrecomp::nvn::astc::decode_block(void_extent_block, 4, 4, output, 16);

    ASSERT_TRUE(result) << "Failed to decode void extent block";

    // Verify all pixels are red
    for (uint32_t y = 0; y < 4; ++y) {
        for (uint32_t x = 0; x < 4; ++x) {
            uint8_t* pixel = output + y * 16 + x * 4;
            EXPECT_EQ(pixel[0], 0xFF) << "R channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[1], 0x00) << "G channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[2], 0x00) << "B channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[3], 0xFF) << "A channel at (" << x << "," << y << ")";
        }
    }
}

// Test void extent block with different color (green)
TEST(AstcDecoder, VoidExtentBlockGreen) {
    uint8_t void_extent_block[16] = {
        0xFF, 0x0F,  // block_mode = 0x1F
        0x00, 0x00,  // R = 0
        0xFF, 0xFF,  // G = 0xFFFF
        0x00, 0x00,  // B = 0
        0xFF, 0xFF,  // A = 0xFFFF
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    uint8_t output[4 * 4 * 4] = {0};
    bool result = kirbyrecomp::nvn::astc::decode_block(void_extent_block, 4, 4, output, 16);

    ASSERT_TRUE(result);

    for (uint32_t y = 0; y < 4; ++y) {
        for (uint32_t x = 0; x < 4; ++x) {
            uint8_t* pixel = output + y * 16 + x * 4;
            EXPECT_EQ(pixel[0], 0x00) << "R channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[1], 0xFF) << "G channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[2], 0x00) << "B channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[3], 0xFF) << "A channel at (" << x << "," << y << ")";
        }
    }
}

// Test void extent block with different block size (8x8)
TEST(AstcDecoder, VoidExtentBlock8x8) {
    uint8_t void_extent_block[16] = {
        0xFF, 0x0F,  // block_mode = 0x1F
        0x00, 0x00,  // R = 0
        0x00, 0x00,  // G = 0
        0xFF, 0xFF,  // B = 0xFFFF (blue)
        0xFF, 0xFF,  // A = 0xFFFF
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // Decode into an 8x8 block
    uint8_t output[8 * 8 * 4] = {0};
    bool result = kirbyrecomp::nvn::astc::decode_block(void_extent_block, 8, 8, output, 32);

    ASSERT_TRUE(result);

    for (uint32_t y = 0; y < 8; ++y) {
        for (uint32_t x = 0; x < 8; ++x) {
            uint8_t* pixel = output + y * 32 + x * 4;
            EXPECT_EQ(pixel[0], 0x00) << "R channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[1], 0x00) << "G channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[2], 0xFF) << "B channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[3], 0xFF) << "A channel at (" << x << "," << y << ")";
        }
    }
}

// Test invalid block mode (should fail gracefully)
TEST(AstcDecoder, InvalidBlockMode) {
    // Create an invalid block (all zeros - invalid mode)
    uint8_t invalid_block[16] = {0};

    uint8_t output[4 * 4 * 4] = {0};
    bool result = kirbyrecomp::nvn::astc::decode_block(invalid_block, 4, 4, output, 16);

    // Should fail for invalid mode
    EXPECT_FALSE(result);
}

// Test decoding a 12x12 void extent block (max size)
TEST(AstcDecoder, VoidExtentBlock12x12) {
    // Use 0xFFFF for all channels (white) - works with ASTC bit packing
    uint8_t void_extent_block[16] = {
        0xFF, 0x0F,  // block_mode = 0x1F
        0xFF, 0xFF,  // R = 0xFFFF
        0xFF, 0xFF,  // G = 0xFFFF
        0xFF, 0xFF,  // B = 0xFFFF
        0xFF, 0xFF,  // A = 0xFFFF
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // Decode into a 12x12 block (max ASTC size)
    uint8_t output[12 * 12 * 4] = {0};
    bool result = kirbyrecomp::nvn::astc::decode_block(void_extent_block, 12, 12, output, 48);

    ASSERT_TRUE(result);

    for (uint32_t y = 0; y < 12; ++y) {
        for (uint32_t x = 0; x < 12; ++x) {
            uint8_t* pixel = output + y * 48 + x * 4;
            // 0xFFFF >> 8 = 0xFF = 255
            EXPECT_EQ(pixel[0], 0xFF) << "R channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[1], 0xFF) << "G channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[2], 0xFF) << "B channel at (" << x << "," << y << ")";
            EXPECT_EQ(pixel[3], 0xFF) << "A channel at (" << x << "," << y << ")";
        }
    }
}