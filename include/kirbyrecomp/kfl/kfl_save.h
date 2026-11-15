#pragma once

// KFL Save Data Support
// Implements DISF/DPFS save data format and AES-CMAC verification

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <span>
#include <array>
#include <unordered_map>
#include <openssl/aes.h>
#include <openssl/cmac.h>

namespace kirbyrecomp::kfl {

// -----------------------------------------------------------------------------
// DISF (Data Integrity Save Format) / DPFS (Data Protection File System)
// Save data structures for Kirby and the Forgotten Land
// -----------------------------------------------------------------------------

// DISF Header
#pragma pack(push, 1)
struct DisfHeader {
    char magic[4];              // "DISF"
    uint32_t version;           // Format version
    uint64_t save_id;           // Unique save identifier
    uint64_t timestamp;         // Creation timestamp (microseconds since epoch)
    uint32_t data_size;         // Size of save data
    uint32_t flags;             // Save flags
    uint8_t cmac[16];           // AES-CMAC of save data
    uint8_t reserved[28];
};
#pragma pack(pop)

// DPFS Header (enhanced save format)
#pragma pack(push, 1)
struct DpfsHeader {
    char magic[4];              // "DPFS"
    uint32_t version;           // Format version
    uint64_t save_id;           // Unique save identifier
    uint64_t timestamp;         // Creation timestamp
    uint64_t last_modified;     // Last modification timestamp
    uint32_t data_size;         // Total data size
    uint32_t block_count;       // Number of data blocks
    uint32_t block_size;        // Size of each block
    uint8_t master_cmac[16];    // AES-CMAC of all blocks combined
    uint8_t block_cmacs[0];     // Array of block CMACs (variable)
};
#pragma pack(pop)

// Save data block
struct SaveDataBlock {
    uint32_t block_id;
    uint32_t offset;
    uint32_t size;
    uint32_t flags;
    std::vector<uint8_t> data;
    std::array<uint8_t, 16> cmac;
};

// Save file metadata
struct SaveMetadata {
    uint64_t save_id;
    std::string save_name;
    uint64_t timestamp;
    uint64_t playtime;
    uint32_t completion_percent;
    std::string current_world;
    std::string current_stage;
    uint32_t flags;
};

// -----------------------------------------------------------------------------
// AES-CMAC Implementation
// -----------------------------------------------------------------------------

class AesCmac {
public:
    AesCmac() = default;
    ~AesCmac() = default;

    // Initialize with key (16 bytes for AES-128)
    bool set_key(const uint8_t* key, size_t key_len);

    // Compute CMAC of data
    // Returns 16-byte CMAC
    std::array<uint8_t, 16> compute(const uint8_t* data, size_t len) const;

    // Compute CMAC of multiple buffers
    std::array<uint8_t, 16> compute(const std::vector<std::span<const uint8_t>>& buffers) const;

    // Verify CMAC
    bool verify(const uint8_t* data, size_t len, const uint8_t* expected_cmac) const;

    // Generate random key
    static std::array<uint8_t, 16> generate_key();

private:
    std::array<uint8_t, 16> key_;
    bool key_set_ = false;
};

// -----------------------------------------------------------------------------
// Save Data Manager
// -----------------------------------------------------------------------------

class SaveDataManager {
public:
    SaveDataManager();
    ~SaveDataManager();

    // Initialize with save directory
    bool initialize(const std::string& save_dir);

    // Create new save file
    bool create_save(uint64_t save_id, const std::string& save_name,
                     const std::vector<uint8_t>& data, const SaveMetadata& metadata);

    // Load save file
    std::optional<std::vector<uint8_t>> load_save(uint64_t save_id,
                                                   SaveMetadata* metadata = nullptr);

    // Update existing save
    bool update_save(uint64_t save_id, const std::vector<uint8_t>& data,
                     const SaveMetadata& metadata);

    // Delete save
    bool delete_save(uint64_t save_id);

    // List all saves
    std::vector<SaveMetadata> list_saves() const;

    // Verify save integrity
    bool verify_save(uint64_t save_id) const;

    // Get save directory
    const std::string& get_save_dir() const { return save_dir_; }

    // Get last error
    std::string get_last_error() const { return last_error_; }

private:
    std::string save_dir_;
    std::string last_error_;
    AesCmac cmac_;
    std::array<uint8_t, 16> save_key_;

    // File paths
    std::string get_save_path(uint64_t save_id) const;
    std::string get_metadata_path(uint64_t save_id) const;

    // Encrypt/decrypt save data
    std::vector<uint8_t> encrypt_data(const std::vector<uint8_t>& data) const;
    std::vector<uint8_t> decrypt_data(const std::vector<uint8_t>& data) const;

    // Compute CMAC for save data
    std::array<uint8_t, 16> compute_save_cmac(const std::vector<uint8_t>& data) const;

    // Write DISF file
    bool write_disf_file(const std::string& path, const std::vector<uint8_t>& data,
                         const SaveMetadata& metadata);

    // Read DISF file
    std::optional<std::vector<uint8_t>> read_disf_file(const std::string& path,
                                                        SaveMetadata* metadata) const;

    // Generate random save ID
    uint64_t generate_save_id() const;
};

} // namespace kirbyrecomp::kfl