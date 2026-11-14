#include "kirbyrecomp/hle/service.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <vulkan/vulkan.h>
#include <cstring>

namespace fs = std::filesystem;

namespace kr::hle {

// ServiceManager implementation
ServiceManager& ServiceManager::instance() {
    static ServiceManager instance;
    return instance;
}

Result ServiceManager::register_service(const std::string& name, std::unique_ptr<IService> service) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (services_.find(name) != services_.end()) {
        return Result::InvalidState;
    }

    services_[name] = std::move(service);
    return Result::Success;
}

Result ServiceManager::unregister_service(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = services_.find(name);
    if (it == services_.end()) {
        return Result::NotFound;
    }

    services_.erase(it);
    return Result::Success;
}

IService* ServiceManager::get_service(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = services_.find(name);
    if (it == services_.end()) {
        return nullptr;
    }
    return it->second.get();
}

Session* ServiceManager::create_session(const std::string& service_name) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    IService* service = get_service(service_name);
    if (!service) {
        return nullptr;
    }

    uint64_t session_id = next_session_id_++;
    Session session;
    session.session_id = session_id;
    session.service = services_[service_name].get();
    session.user_data = nullptr;
    session.process_id = 0;
    session.thread_id = 0;

    auto [it, inserted] = sessions_.emplace(session_id, std::move(session));
    return &it->second;
}

void ServiceManager::close_session(uint64_t session_id) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    sessions_.erase(session_id);
}

// ============================================================================
// Built-in Services
// ============================================================================

// sm: - Service Manager
class SmService : public IService {
public:
    const char* get_name() const override { return "sm:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // sm: commands:
        // 0: RegisterService
        // 1: UnregisterService
        // 2: GetService
        // 3: RegisterServiceWithPolicy
        switch (command_id) {
            case 0: // RegisterService
                return Result::Success; // Not implemented yet
            case 1: // UnregisterService
                return Result::Success; // Not implemented yet
            case 2: // GetService
                return Result::Success; // Not implemented yet
            case 3: // RegisterServiceWithPolicy
                return Result::Success; // Not implemented yet
            default:
                return Result::InvalidArgument;
        }
    }
};

// fs: - File System
class FsService : public IService {
public:
    FsService() {
        // Default paths - can be configured
        exefs_path_ = "D:/Claude Stuff/KirbyRecomp/ExeFS";
        romfs_path_ = "D:/Claude Stuff/KirbyRecomp/RomFS";
    }

    void set_paths(const fs::path& exefs, const fs::path& romfs) {
        std::lock_guard<std::mutex> lock(mutex_);
        exefs_path_ = exefs;
        romfs_path_ = romfs;
    }

    const char* get_name() const override { return "fs:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        std::lock_guard<std::mutex> lock(mutex_);

        // fs: commands:
        // 0: OpenFile
        // 1: CreateFile
        // 2: DeleteFile
        // 3: RenameFile
        // 4: CreateDirectory
        // 5: OpenDirectory
        // 6: GetFileSize
        // 7: GetFileTimeStamp
        // 8: SetFileTimeStamp
        // 9: GetFreeSpaceSize
        // 10: Commit
        // 11: GetFileAttribute
        // 12: SetFileAttribute
        // 13: QueryEntry
        // 14: CreateFileWithSize
        // 15: OpenFileWithPolicy
        // ...

        switch (command_id) {
            case 0: // OpenFile
                return open_file(in_data, in_size, out_data, out_size);
            case 5: // OpenDirectory
                return open_directory(in_data, in_size, out_data, out_size);
            case 6: // GetFileSize
                return get_file_size(in_data, in_size, out_data, out_size);
            case 11: // GetFileAttribute
                return get_file_attribute(in_data, in_size, out_data, out_size);
            case 13: // QueryEntry
                return query_entry(in_data, in_size, out_data, out_size);
            default:
                return Result::Success; // Stub for unimplemented commands
        }
    }

private:
    mutable std::mutex mutex_;
    fs::path exefs_path_ = "D:/Claude Stuff/KirbyRecomp/ExeFS";
    fs::path romfs_path_ = "D:/Claude Stuff/KirbyRecomp/RomFS";
    uint64_t next_handle_id_ = 1;
    std::unordered_map<uint64_t, std::unique_ptr<FileHandle>> file_handles_;
    std::unordered_map<uint64_t, std::unique_ptr<DirectoryHandle>> dir_handles_;

    struct FileHandle {
        std::ifstream file;
        fs::path path;
        uint64_t size = 0;
        bool is_directory = false;
        size_t dir_index = 0;
        std::vector<fs::directory_entry> dir_entries;
    };

    struct DirectoryHandle {
        fs::path path;
        std::vector<fs::directory_entry> entries;
        size_t index = 0;
    };

    fs::path resolve_path(const char* path) const {
        if (!path) return "";
        std::string p(path);
        // Remove leading slash
        if (!p.empty() && p[0] == '/') p = p.substr(1);

        // Check ExeFS first
        fs::path exefs_full = exefs_path_ / p;
        if (fs::exists(exefs_full)) return exefs_full;

        // Check RomFS
        fs::path romfs_full = romfs_path_ / p;
        if (fs::exists(romfs_full)) return romfs_full;

        return "";
    }

    Result open_file(const void* in_data, size_t in_size,
                     void* out_data, size_t out_size) {
        if (!in_data || in_size == 0) return Result::InvalidArgument;

        const char* path_str = static_cast<const char*>(in_data);
        fs::path full_path = resolve_path(path_str);

        if (full_path.empty() || !fs::is_regular_file(full_path)) {
            return Result::NotFound;
        }

        auto handle = std::make_unique<FileHandle>();
        handle->file.open(full_path, std::ios::binary);
        if (!handle->file.is_open()) {
            return Result::InvalidState;
        }
        handle->path = full_path;
        handle->size = fs::file_size(full_path);
        handle->is_directory = false;

        uint64_t handle_id = next_handle_id_++;
        file_handles_[handle_id] = std::move(handle);

        if (out_data && out_size >= 8) {
            *static_cast<uint64_t*>(out_data) = handle_id;
        }
        return Result::Success;
    }

    Result open_directory(const void* in_data, size_t in_size,
                          void* out_data, size_t out_size) {
        if (!in_data || in_size == 0) return Result::InvalidArgument;

        const char* path_str = static_cast<const char*>(in_data);
        fs::path full_path = resolve_path(path_str);

        if (full_path.empty() || !fs::is_directory(full_path)) {
            return Result::NotFound;
        }

        auto handle = std::make_unique<DirectoryHandle>();
        handle->path = full_path;
        for (const auto& entry : fs::directory_iterator(full_path)) {
            handle->entries.push_back(entry);
        }

        uint64_t handle_id = next_handle_id_++;
        dir_handles_[handle_id] = std::move(handle);

        if (out_data && out_size >= 8) {
            *static_cast<uint64_t*>(out_data) = handle_id;
        }
        return Result::Success;
    }

    Result get_file_size(const void* in_data, size_t in_size,
                         void* out_data, size_t out_size) {
        if (!in_data || in_size < 8) return Result::InvalidArgument;

        uint64_t handle_id = *static_cast<const uint64_t*>(in_data);
        auto it = file_handles_.find(handle_id);
        if (it == file_handles_.end()) return Result::InvalidArgument;

        if (out_data && out_size >= 8) {
            *static_cast<uint64_t*>(out_data) = it->second->size;
        }
        return Result::Success;
    }

    Result get_file_attribute(const void* in_data, size_t in_size,
                              void* out_data, size_t out_size) {
        // Simplified: return basic file attribute
        if (out_data && out_size >= 4) {
            *static_cast<uint32_t*>(out_data) = 0; // Regular file
        }
        return Result::Success;
    }

    Result query_entry(const void* in_data, size_t in_size,
                       void* out_data, size_t out_size) {
        if (!in_data || in_size < 8) return Result::InvalidArgument;

        uint64_t handle_id = *static_cast<const uint64_t*>(in_data);
        auto it = dir_handles_.find(handle_id);
        if (it == dir_handles_.end()) return Result::InvalidArgument;

        auto& handle = it->second;
        if (handle->index >= handle->entries.size()) {
            return Result::NotFound; // End of directory
        }

        const auto& entry = handle->entries[handle->index++];
        // Output: entry name, type, size
        // This is a simplified version
        if (out_data && out_size >= 256) {
            char* out = static_cast<char*>(out_data);
            std::strncpy(out, entry.path().filename().string().c_str(), 255);
            out[255] = '\0';
        }
        return Result::Success;
    }

private:
    mutable std::mutex mutex_;
    fs::path exefs_path_ = "D:/Claude Stuff/KirbyRecomp/ExeFS";
    fs::path romfs_path_ = "D:/Claude Stuff/KirbyRecomp/RomFS";
    uint64_t next_handle_id_ = 1;
    std::unordered_map<uint64_t, std::unique_ptr<FileHandle>> file_handles_;
    std::unordered_map<uint64_t, std::unique_ptr<DirectoryHandle>> dir_handles_;

    struct FileHandle {
        std::ifstream file;
        fs::path path;
        uint64_t size = 0;
        bool is_directory = false;
        size_t dir_index = 0;
        std::vector<fs::directory_entry> dir_entries;
    };

    struct DirectoryHandle {
        fs::path path;
        std::vector<fs::directory_entry> entries;
        size_t index = 0;
    };
};

// am: - Application Manager
class AmService : public IService {
public:
    const char* get_name() const override { return "am:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // am: commands:
        // 0: GetAppletType
        // 1: GetPerformanceMode
        // 2: GetOperationMode
        // 3: GetHomeMenuDisplayMode
        // ...
        switch (command_id) {
            case 0: // GetAppletType
                if (out_data && out_size >= 4) {
                    *static_cast<uint32_t*>(out_data) = 0; // Application
                    return Result::Success;
                }
                return Result::InvalidArgument;
            case 1: // GetPerformanceMode
                if (out_data && out_size >= 4) {
                    *static_cast<uint32_t*>(out_data) = 0; // Normal
                    return Result::Success;
                }
                return Result::InvalidArgument;
            default:
                return Result::InvalidArgument;
        }
    }
};

// hid: - Human Interface Device
class HidService : public IService {
public:
    const char* get_name() const override { return "hid:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // hid: commands:
        // 0: GetNpadState
        // 1: GetMouseState
        // 2: GetTouchScreenState
        // 3: GetKeyboardState
        // 4: GetNpadStyleSet
        // 5: SetNpadStyleSet
        // ...
        return Result::Success; // Stub
    }
};

// time: - Time Service
class TimeService : public IService {
public:
    const char* get_name() const override { return "time:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // time: commands:
        // 0: GetCurrentTime
        // 1: GetSystemTick
        // 2: GetTimeZoneOffset
        // ...
        if (command_id == 0 && out_data && out_size >= 8) {
            *static_cast<uint64_t*>(out_data) = 0; // Unix timestamp
            return Result::Success;
        }
        return Result::Success; // Stub
    }
};

// set: - Settings Service
class SetService : public IService {
public:
    const char* get_name() const override { return "set:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // set: commands:
        // 0: GetSettingValue
        // 1: SetSettingValue
        // 2: Create
        // 3: Delete
        // ...
        return Result::Success; // Stub
    }
};

// audren: - Audio Renderer
class AudrenService : public IService {
public:
    const char* get_name() const override { return "audren:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // audren: commands:
        // 0: OpenAudioRenderer
        // 1: CloseAudioRenderer
        // 2: StartAudioRenderer
        // 2: StopAudioRenderer
        // 3: RequestUpdate
        // 4: AppendAudioRendererBuffer
        // 5: RegisterBufferEvent
        // 6: UnregisterBufferEvent
        // 7: GetReleasedBuffer
        // 8: GetAudioRendererState
        // 9: SetAudioRendererVolume
        // 10: GetAudioRendererVolume
        // ...
        return Result::Success; // Stub
    }
};

// audout: - Audio Output
class AudoutService : public IService {
public:
    const char* get_name() const override { return "audout:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // audout: commands:
        // 0: OpenAudioOut
        // 1: CloseAudioOut
        // 2: StartAudioOut
        // 3: StopAudioOut
        // 4: AppendAudioOutBuffer
        // 5: RegisterBufferEvent
        // 6: UnregisterBufferEvent
        // 7: SetAudioOutVolume
        // 8: GetAudioOutVolume
        // ...
        return Result::Success; // Stub
    }
};

// hwopus: - Hardware Opus Decoder
class HwOpusService : public IService {
public:
    const char* get_name() const override { return "hwopus:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // hwopus: commands:
        // 0: OpenHardwareOpusDecoder
        // 1: CloseHardwareOpusDecoder
        // 2: DecodeInterleaved
        // ...
        return Result::Success; // Stub
    }
};

// nvn: - NVN Graphics (stub - will be implemented properly later)
class NvnService : public IService {
public:
    const char* get_name() const override { return "nvn:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // nvn: commands (many, will implement properly later):
        // DeviceCreate, QueueSubmit, MemoryMap, etc.
        return Result::Success; // Stub
    }
};

// Service Factory
Result ServiceFactory::create_builtin_services(ServiceManager& sm) {
    Result result;

    result = sm.register_service("sm:", std::make_unique<SmService>());
    if (result != Result::Success) return result;

    result = sm.register_service("fs:", std::make_unique<FsService>());
    if (result != Result::Success) return result;

    result = sm.register_service("am:", std::make_unique<AmService>());
    if (result != Result::Success) return result;

    result = sm.register_service("hid:", std::make_unique<HidService>());
    if (result != Result::Success) return result;

    result = sm.register_service("time:", std::make_unique<TimeService>());
    if (result != Result::Success) return result;

    result = sm.register_service("set:", std::make_unique<SetService>());
    if (result != Result::Success) return result;

    result = sm.register_service("audren:", std::make_unique<AudrenService>());
    if (result != Result::Success) return result;

    result = sm.register_service("audout:", std::make_unique<AudoutService>());
    if (result != Result::Success) return result;

    result = sm.register_service("hwopus:", std::make_unique<HwOpusService>());
    if (result != Result::Success) return result;

    result = sm.register_service("nvn:", std::make_unique<NvnService>());
    if (result != Result::Success) return result;

    return Result::Success;
}

} // namespace kr::hle