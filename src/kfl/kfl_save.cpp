// KFL Save Data Implementation
// Implements DISF/DPFS save data format and AES-CMAC verification

#include "kirbyrecomp/kfl/kfl_save.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <random>
#include <chrono>
#include <algorithm>
#include <openssl/aes.h>
#include <openssl/cmac.h>
#include <openssl/rand.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

namespace kirbyrecomp::kfl {

// -----------------------------------------------------------------------------
// Path utilities (replacement for std::filesystem to avoid mingw64 linking issues)
// -----------------------------------------------------------------------------

static bool path_exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

static bool create_directories(const std::string& path) {
    std::string current;
    for (char c : path) {
        if (c == '/' || c == '\\') {
            if (!current.empty() && !path_exists(current)) {
                if (mkdir(current.c_str()) != 0 && errno != EEXIST) return false;
            }
        }
        current += c;
    }
    if (!path_exists(path)) {
        if (mkdir(path.c_str()) != 0 && errno != EEXIST) return false;
    }
    return true;
}

static bool remove_file(const std::string& path) {
    return remove(path.c_str()) == 0;
}

static std::string path_extension(const std::string& path) {
    size_t pos = path.find_last_of('.');
    if (pos == std::string::npos) return "";
    return path.substr(pos);
}

static std::vector<std::string> list_directory(const std::string& path) {
    std::vector<std::string> files;
    DIR* dir = opendir(path.c_str());
    if (!dir) return files;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        files.push_back(entry->d_name);
    }
    closedir(dir);
    return files;
}

// -----------------------------------------------------------------------------
// AesCmac Implementation
// -----------------------------------------------------------------------------

bool AesCmac::set_key(const uint8_t* key, size_t key_len) {
    if (key_len != 16) return false; // AES-128 only
    std::memcpy(key_.data(), key, 16);
    key_set_ = true;
    return true;
}

std::array<uint8_t, 16> AesCmac::compute(const uint8_t* data, size_t len) const {
    std::array<uint8_t, 16> cmac{};
    size_t cmac_len = 16;

    CMAC_CTX* ctx = CMAC_CTX_new();
    if (!ctx) return cmac;

    if (!CMAC_Init(ctx, key_.data(), 16, EVP_aes_128_cbc(), nullptr)) {
        CMAC_CTX_free(ctx);
        return cmac;
    }

    if (!CMAC_Update(ctx, data, len)) {
        CMAC_CTX_free(ctx);
        return cmac;
    }

    if (!CMAC_Final(ctx, cmac.data(), &cmac_len)) {
        CMAC_CTX_free(ctx);
        return cmac;
    }

    CMAC_CTX_free(ctx);
    return cmac;
}

std::array<uint8_t, 16> AesCmac::compute(const std::vector<std::span<const uint8_t>>& buffers) const {
    std::array<uint8_t, 16> cmac{};
    size_t cmac_len = 16;

    CMAC_CTX* ctx = CMAC_CTX_new();
    if (!ctx) return cmac;

    if (!CMAC_Init(ctx, key_.data(), 16, EVP_aes_128_cbc(), nullptr)) {
        CMAC_CTX_free(ctx);
        return cmac;
    }

    for (const auto& buf : buffers) {
        if (!CMAC_Update(ctx, buf.data(), buf.size())) {
            CMAC_CTX_free(ctx);
            return std::array<uint8_t, 16>{};
        }
    }

    if (!CMAC_Final(ctx, cmac.data(), &cmac_len)) {
        CMAC_CTX_free(ctx);
        return cmac;
    }

    CMAC_CTX_free(ctx);
    return cmac;
}

bool AesCmac::verify(const uint8_t* data, size_t len, const uint8_t* expected_cmac) const {
    auto computed = compute(data, len);
    return std::memcmp(computed.data(), expected_cmac, 16) == 0;
}

std::array<uint8_t, 16> AesCmac::generate_key() {
    std::array<uint8_t, 16> key{};
    RAND_bytes(key.data(), 16);
    return key;
}

// -----------------------------------------------------------------------------
// SaveDataManager Implementation
// -----------------------------------------------------------------------------

SaveDataManager::SaveDataManager() {
    // Generate or load save encryption key
    save_key_ = AesCmac::generate_key();
    cmac_.set_key(save_key_.data(), 16);
}

SaveDataManager::~SaveDataManager() {
}

bool SaveDataManager::initialize(const std::string& save_dir) {
    save_dir_ = save_dir;

    // Create save directory if it doesn't exist
    if (!create_directories(save_dir_)) {
        last_error_ = "Failed to create save directory: " + save_dir_;
        return false;
    }

    return true;
}

std::string SaveDataManager::get_save_path(uint64_t save_id) const {
    char filename[64];
    snprintf(filename, sizeof(filename), "save_%016llx.disf", save_id);
    return save_dir_ + "/" + filename;
}

std::string SaveDataManager::get_metadata_path(uint64_t save_id) const {
    char filename[64];
    snprintf(filename, sizeof(filename), "save_%016llx.meta", save_id);
    return save_dir_ + "/" + filename;
}

uint64_t SaveDataManager::generate_save_id() const {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    return gen();
}

std::array<uint8_t, 16> SaveDataManager::compute_save_cmac(const std::vector<uint8_t>& data) const {
    return cmac_.compute(data.data(), data.size());
}

std::vector<uint8_t> SaveDataManager::encrypt_data(const std::vector<uint8_t>& data) const {
    // Simple XOR encryption for now (AES-CTR would be better in production)
    std::vector<uint8_t> encrypted(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        encrypted[i] = data[i] ^ save_key_[i % 16];
    }
    return encrypted;
}

std::vector<uint8_t> SaveDataManager::decrypt_data(const std::vector<uint8_t>& data) const {
    // XOR encryption is symmetric
    return encrypt_data(data);
}

bool SaveDataManager::write_disf_file(const std::string& path, const std::vector<uint8_t>& data,
                                      const SaveMetadata& metadata) {
    DisfHeader header{};
    std::memcpy(header.magic, "DISF", 4);
    header.version = 1;
    header.save_id = metadata.save_id;
    header.timestamp = metadata.timestamp;
    header.data_size = static_cast<uint32_t>(data.size());
    header.flags = metadata.flags;

    // Compute CMAC
    auto cmac = compute_save_cmac(data);
    std::memcpy(header.cmac, cmac.data(), 16);

    // Encrypt data
    std::vector<uint8_t> encrypted_data = encrypt_data(data);

    // Write file
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    // Write header
    fwrite(&header, 1, sizeof(DisfHeader), f);

    // Write encrypted data
    fwrite(encrypted_data.data(), 1, encrypted_data.size(), f);

    fclose(f);
    return true;
}

std::optional<std::vector<uint8_t>> SaveDataManager::read_disf_file(const std::string& path,
                                                                     SaveMetadata* metadata) const {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return std::nullopt;

    DisfHeader header{};
    if (fread(&header, 1, sizeof(DisfHeader), f) != sizeof(DisfHeader)) {
        fclose(f);
        return std::nullopt;
    }

    // Verify magic
    if (std::memcmp(header.magic, "DISF", 4) != 0) {
        fclose(f);
        return std::nullopt;
    }

    // Read encrypted data
    std::vector<uint8_t> encrypted_data(header.data_size);
    if (fread(encrypted_data.data(), 1, encrypted_data.size(), f) != header.data_size) {
        fclose(f);
        return std::nullopt;
    }
    fclose(f);

    // Decrypt data
    std::vector<uint8_t> data = decrypt_data(encrypted_data);

    // Verify CMAC
    auto cmac = compute_save_cmac(data);
    if (std::memcmp(header.cmac, cmac.data(), 16) != 0) {
        return std::nullopt; // CMAC verification failed
    }

    if (metadata) {
        metadata->save_id = header.save_id;
        metadata->save_name = ""; // Would need to store name separately
        metadata->timestamp = header.timestamp;
        metadata->playtime = 0;
        metadata->completion_percent = 0;
        metadata->current_world = "";
        metadata->current_stage = "";
        metadata->flags = header.flags;
    }

    return data;
}

bool SaveDataManager::create_save(uint64_t save_id, const std::string& save_name,
                                  const std::vector<uint8_t>& data, const SaveMetadata& metadata) {
    std::string path = get_save_path(save_id);

    SaveMetadata meta = metadata;
    meta.save_id = save_id;
    meta.save_name = save_name;
    meta.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    if (!write_disf_file(save_dir_ + "/save_" + std::to_string(save_id) + ".disf", data, meta)) {
        last_error_ = "Failed to write save file";
        return false;
    }

    return true;
}

std::optional<std::vector<uint8_t>> SaveDataManager::load_save(uint64_t save_id,
                                                                SaveMetadata* metadata) {
    std::string path = get_save_path(save_id);
    return read_disf_file(path, metadata);
}

bool SaveDataManager::update_save(uint64_t save_id, const std::vector<uint8_t>& data,
                                  const SaveMetadata& metadata) {
    return create_save(save_id, "", data, metadata);
}

bool SaveDataManager::delete_save(uint64_t save_id) {
    std::string path = get_save_path(save_id);
    return remove_file(path);
}

std::vector<SaveMetadata> SaveDataManager::list_saves() const {
    std::vector<SaveMetadata> saves;

    auto files = list_directory(save_dir_);
    for (const auto& filename : files) {
        if (path_extension(filename) == ".disf") {
            SaveMetadata meta;
            auto data = read_disf_file(save_dir_ + "/" + filename, &meta);
            if (data) {
                saves.push_back(meta);
            }
        }
    }

    return saves;
}

bool SaveDataManager::verify_save(uint64_t save_id) const {
    std::string path = get_save_path(save_id);
    SaveMetadata meta;
    auto data = read_disf_file(path, &meta);
    return data.has_value();
}

} // namespace kirbyrecomp::kfl