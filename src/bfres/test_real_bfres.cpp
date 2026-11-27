// Test BFRES parser with real decompressed file

#include "kirbyrecomp/bfres/bfres.h"
#include "kirbyrecomp/nvn/astc_decoder.h"
#include "kirbyrecomp/nvn/bc7_encoder.h"
#include "kirbyrecomp/bfres/cmp_decompress.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.cmp or file.bfres>" << std::endl;
        return 1;
    }

    std::string input_path = argv[1];
    std::vector<uint8_t> bfres_data;

    // Check if it's a .cmp file
    if (input_path.size() >= 4 && input_path.substr(input_path.size() - 4) == ".cmp") {
        std::cout << "Decompressing .cmp file..." << std::endl;
        std::string output_path = "/tmp/test_output.bfres";
        if (!kirbyrecomp::bfres::decompress_cmp_file(input_path, output_path)) {
            std::cerr << "Failed to decompress .cmp file" << std::endl;
            return 1;
        }

        // Read decompressed file
        std::ifstream f("/tmp/test_output.bfres", std::ios::binary | std::ios::ate);
        if (!f) {
            std::cerr << "Failed to open decompressed file" << std::endl;
            return 1;
        }
        size_t size = f.tellg();
        f.seekg(0, std::ios::beg);
        bfres_data.resize(size);
        f.read(reinterpret_cast<char*>(bfres_data.data()), size);
        f.close();
    } else {
        // Read BFRES file directly
        std::ifstream f(input_path, std::ios::binary | std::ios::ate);
        if (!f) {
            std::cerr << "Failed to open file" << std::endl;
            return 1;
        }
        size_t size = f.tellg();
        f.seekg(0, std::ios::beg);
        bfres_data.resize(size);
        f.read(reinterpret_cast<char*>(bfres_data.data()), size);
        f.close();
    }

    std::cout << "File size: " << bfres_data.size() << " bytes" << std::endl;

    // Parse BFRES
    kirbyrecomp::bfres::BfresFile bfres(bfres_data);
    if (!bfres.parse()) {
        std::cerr << "Failed to parse BFRES: " << bfres.get_error() << std::endl;
        return 1;
    }

    std::cout << "BFRES parsed successfully!" << std::endl;
    std::cout << "Textures found: " << bfres.get_textures().size() << std::endl;

    // Check for all known sections
    static const char* section_names[] = {
        "FTEX", "FMDL", "FSKA", "FMAA", "FSHA", "FSCN", "FBNV", "FSHPA",
        "TEXPAT", "BNTX", "_STR", "FVTX", "FRAG", "FVSH", "FPRO", "FLOD"
    };
    for (const char* name : section_names) {
        auto section = bfres.get_section(name);
        if (section) {
            std::cout << "Section " << name << " found (size: " << section->size() << ")" << std::endl;
        }
    }

    std::cout << "FTEX Textures found: " << bfres.get_textures().size() << std::endl;
    std::cout << "BNTX Textures found: " << bfres.get_bntx_textures().size() << std::endl;

    // Test BNTX texture extraction with ASTC->BC7 transcoding
    int success_count = 0;
    for (size_t i = 0; i < bfres.get_bntx_textures().size(); ++i) {
        const auto& tex = bfres.get_bntx_textures()[i];
        std::cout << "  BNTX Texture " << i << ": " << tex.name << std::endl;
        std::cout << "    Offset: " << tex.data_offset << ", Size: " << tex.data_size << std::endl;

        auto tex_data = bfres.get_bntx_texture_data(i);
        if (tex_data) {
            std::cout << "    Extracted data size: " << tex_data->size() << std::endl;

            // If ASTC, try to decode first block and transcode to BC7
            if (static_cast<uint32_t>(tex.format) >= 0x93B0 && static_cast<uint32_t>(tex.format) <= 0x93DD) {
                std::cout << "    Format is ASTC, attempting decode and BC7 transcode..." << std::endl;
                uint32_t block_w = 4, block_h = 4;
                switch (tex.format) {
                    case kirbyrecomp::bfres::TextureFormat::ASTC_4x4_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_4x4_SRGB:
                        block_w = 4; block_h = 4; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_5x4_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_5x4_SRGB:
                        block_w = 5; block_h = 4; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_5x5_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_5x5_SRGB:
                        block_w = 5; block_h = 5; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_6x5_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_6x5_SRGB:
                        block_w = 6; block_h = 5; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_6x6_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_6x6_SRGB:
                        block_w = 6; block_h = 6; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_8x5_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_8x5_SRGB:
                        block_w = 8; block_h = 5; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_8x6_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_8x6_SRGB:
                        block_w = 8; block_h = 6; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_8x8_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_8x8_SRGB:
                        block_w = 8; block_h = 8; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_10x5_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_10x5_SRGB:
                        block_w = 10; block_h = 5; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_10x6_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_10x6_SRGB:
                        block_w = 10; block_h = 6; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_10x8_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_10x8_SRGB:
                        block_w = 10; block_h = 8; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_10x10_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_10x10_SRGB:
                        block_w = 10; block_h = 10; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_12x10_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_12x10_SRGB:
                        block_w = 12; block_h = 10; break;
                    case kirbyrecomp::bfres::TextureFormat::ASTC_12x12_UNORM:
                    case kirbyrecomp::bfres::TextureFormat::ASTC_12x12_SRGB:
                        block_w = 12; block_h = 12; break;
                }

                // Decode first ASTC block
                std::vector<uint8_t> output(block_w * block_h * 4, 0);
                bool result = kirbyrecomp::nvn::astc::decode_block(
                    tex_data->data(), block_w, block_h, output.data(), block_w * 4
                );
                std::cout << "    ASTC decode result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
                if (result) {
                    std::cout << "    First pixel RGBA: "
                              << (int)output[0] << ", "
                              << (int)output[1] << ", "
                              << (int)output[2] << ", "
                              << (int)output[3] << std::endl;

                    // Transcode to BC7
                    kirbyrecomp::nvn::bc7::BC7Block bc7_block;
                    kirbyrecomp::nvn::bc7::encode_mode6(output.data(), block_w, block_h, block_w * 4, bc7_block);
                    std::cout << "    BC7 transcode result: SUCCESS" << std::endl;
                    std::cout << "    BC7 block: ";
                    for (int j = 0; j < 16; ++j) {
                        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)bc7_block.data[j] << " ";
                    }
                    std::cout << std::dec << std::endl;
                    success_count++;
                } else {
                    std::cout << "    ASTC decode FAILED" << std::endl;
                }
            }
        }
    }

    std::cout << "\n=== SUMMARY ===" << std::endl;
    std::cout << "Total textures tested: " << bfres.get_bntx_textures().size() << std::endl;
    std::cout << "Successful ASTC->BC7 transcodes: " << success_count << std::endl;

    return 0;
}