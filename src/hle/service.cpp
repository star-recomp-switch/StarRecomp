#include "kirbyrecomp/hle/service.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <fstream>
#include <vulkan/vulkan.h>
#include <cstring>
#include <condition_variable>
#include <algorithm>
#include <string>
#include <sys/stat.h>

// Windows-specific Vulkan surface extension
#if defined(_WIN32) || defined(__CYGWIN__)
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif

// SDL3 for audio
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

// Opus for audio decoding
#include <opus/opus.h>

// Use std::string for paths to avoid std::filesystem linking issues on mingw64
namespace fs = std;  // Alias for convenience

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

    void set_paths(const std::string& exefs, const std::string& romfs) {
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
            case 1: // CreateFile
                return create_file(in_data, in_size, out_data, out_size);
            case 2: // DeleteFile
                return delete_file(in_data, in_size, out_data, out_size);
            case 3: // RenameFile
                return rename_file(in_data, in_size, out_data, out_size);
            case 4: // CreateDirectory
                return create_directory(in_data, in_size, out_data, out_size);
            case 5: // OpenDirectory
                return open_directory(in_data, in_size, out_data, out_size);
            case 6: // GetFileSize
                return get_file_size(in_data, in_size, out_data, out_size);
            case 7: // GetFileTimeStamp
                return get_file_timestamp(in_data, in_size, out_data, out_size);
            case 8: // SetFileTimeStamp
                return set_file_timestamp(in_data, in_size, out_data, out_size);
            case 9: // GetFreeSpaceSize
                return get_free_space_size(in_data, in_size, out_data, out_size);
            case 10: // Commit
                return commit(in_data, in_size, out_data, out_size);
            case 11: // GetFileAttribute
                return get_file_attribute(in_data, in_size, out_data, out_size);
            case 12: // SetFileAttribute
                return set_file_attribute(in_data, in_size, out_data, out_size);
            case 13: // QueryEntry
                return query_entry(in_data, in_size, out_data, out_size);
            case 14: // CreateFileWithSize
                return create_file_with_size(in_data, in_size, out_data, out_size);
            default:
                return Result::Success; // Stub for unimplemented commands
        }
    }

private:
    // Struct definitions must come before their use in member variables
    struct DirEntry {
        std::string name;
        bool is_directory = false;
        uint64_t size = 0;
    };

    struct FileHandle {
        std::ifstream file;
        std::string path;
        uint64_t size = 0;
        bool is_directory = false;
    };

    struct DirectoryHandle {
        std::string path;
        std::vector<DirEntry> entries;
        size_t index = 0;
    };

    mutable std::mutex mutex_;
    std::string exefs_path_ = "D:/Claude Stuff/KirbyRecomp/ExeFS";
    std::string romfs_path_ = "D:/Claude Stuff/KirbyRecomp/RomFS";
    uint64_t next_handle_id_ = 1;
    std::unordered_map<uint64_t, std::unique_ptr<FileHandle>> file_handles_;
    std::unordered_map<uint64_t, std::unique_ptr<DirectoryHandle>> dir_handles_;

    // Platform-independent path join
    static std::string path_join(const std::string& a, const std::string& b) {
        if (a.empty()) return b;
        if (b.empty()) return a;
        if (a.back() == '/' || a.back() == '\\') {
            return a + b;
        }
        return a + "/" + b;
    }

    // Check if path exists and is a regular file
    static bool is_regular_file(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
    }

    // Check if path exists and is a directory
    static bool is_directory(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
    }

    // Get file size
    static uint64_t file_size(const std::string& path) {
        struct stat st;
        if (stat(path.c_str(), &st) == 0) {
            return static_cast<uint64_t>(st.st_size);
        }
        return 0;
    }

    // Check if path exists
    static bool exists(const std::string& path) {
        struct stat st;
        return stat(path.c_str(), &st) == 0;
    }

    // List directory entries
    static std::vector<DirEntry> list_directory(const std::string& path) {
        std::vector<DirEntry> entries;
#ifdef _WIN32
        std::string search_path = path_join(path, "*");
        WIN32_FIND_DATAA find_data;
        HANDLE handle = FindFirstFileA(search_path.c_str(), &find_data);
        if (handle != INVALID_HANDLE_VALUE) {
            do {
                std::string name = find_data.cFileName;
                if (name != "." && name != "..") {
                    DirEntry entry;
                    entry.name = name;
                    entry.is_directory = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                    entry.size = (static_cast<uint64_t>(find_data.nFileSizeHigh) << 32) | find_data.nFileSizeLow;
                    entries.push_back(entry);
                }
            } while (FindNextFileA(handle, &find_data));
            FindClose(handle);
        }
#else
        DIR* dir = opendir(path.c_str());
        if (dir) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if (name != "." && name != "..") {
                    DirEntry dir_entry;
                    dir_entry.name = name;
                    std::string full_path = path_join(path, name);
                    dir_entry.is_directory = is_directory(full_path);
                    dir_entry.size = file_size(full_path);
                    entries.push_back(dir_entry);
                }
            }
            closedir(dir);
        }
#endif
        return entries;
    }

    std::string resolve_path(const char* path) const {
        if (!path) return "";
        std::string p(path);
        // Remove leading slash
        if (!p.empty() && p[0] == '/') p = p.substr(1);

        // Check ExeFS first
        std::string exefs_full = path_join(exefs_path_, p);
        if (exists(exefs_full)) return exefs_full;

        // Check RomFS
        std::string romfs_full = path_join(romfs_path_, p);
        if (exists(romfs_full)) return romfs_full;

        return "";
    }

    Result open_file(const void* in_data, size_t in_size,
                     void* out_data, size_t out_size) {
        if (!in_data || in_size == 0) return Result::InvalidArgument;

        const char* path_str = static_cast<const char*>(in_data);
        std::string full_path = resolve_path(path_str);

        if (full_path.empty() || !is_regular_file(full_path)) {
            return Result::NotFound;
        }

        auto handle = std::make_unique<FileHandle>();
        handle->file.open(full_path, std::ios::binary);
        if (!handle->file.is_open()) {
            return Result::InvalidState;
        }
        handle->path = full_path;
        handle->size = file_size(full_path);
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
        std::string full_path = resolve_path(path_str);

        if (full_path.empty() || !is_directory(full_path)) {
            return Result::NotFound;
        }

        auto handle = std::make_unique<DirectoryHandle>();
        handle->path = full_path;
        handle->entries = list_directory(full_path);

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
            std::strncpy(out, entry.name.c_str(), 255);
            out[255] = '\0';
        }
        return Result::Success;
    }

    // Additional FsService commands

    Result create_file(const void* in_data, size_t in_size,
                       void* out_data, size_t out_size) {
        // Input: path (char*), size (u64)
        if (!in_data || in_size < 16) return Result::InvalidArgument;

        const char* path_str = static_cast<const char*>(in_data);
        uint64_t size = *reinterpret_cast<const uint64_t*>(path_str + strlen(path_str) + 1);

        std::string full_path = resolve_path(path_str);
        if (full_path.empty()) {
            // Create in ExeFS directory
            full_path = path_join(exefs_path_, path_str);
            std::string p = path_str;
            if (!p.empty() && p[0] == '/') p = p.substr(1);
            full_path = path_join(exefs_path_, p);
        }

        // Create parent directories if needed
        // Note: In read-only ExeFS/RomFS, this would fail
        // For now, return success as a stub
        if (out_data && out_size >= 8) {
            uint64_t handle_id = next_handle_id_++;
            *static_cast<uint64_t*>(out_data) = handle_id;
        }
        return Result::Success;
    }

    Result delete_file(const void* in_data, size_t in_size,
                       void* out_data, size_t out_size) {
        if (!in_data || in_size == 0) return Result::InvalidArgument;

        const char* path_str = static_cast<const char*>(in_data);
        std::string full_path = resolve_path(path_str);

        if (full_path.empty() || !is_regular_file(full_path)) {
            return Result::NotFound;
        }

        // In read-only filesystem, this would fail
        // For now, return success as a stub
        return Result::Success;
    }

    Result rename_file(const void* in_data, size_t in_size,
                       void* out_data, size_t out_size) {
        // Input: old_path (char*), new_path (char*)
        if (!in_data || in_size == 0) return Result::InvalidArgument;

        // Stub implementation
        return Result::Success;
    }

    Result create_directory(const void* in_data, size_t in_size,
                            void* out_data, size_t out_size) {
        if (!in_data || in_size == 0) return Result::InvalidArgument;

        const char* path_str = static_cast<const char*>(in_data);
        std::string full_path = resolve_path(path_str);

        // In read-only filesystem, this would fail
        // For now, return success as a stub
        if (out_data && out_size >= 8) {
            *static_cast<uint64_t*>(out_data) = 0;
        }
        return Result::Success;
    }

    Result get_file_timestamp(const void* in_data, size_t in_size,
                              void* out_data, size_t out_size) {
        if (!in_data || in_size < 8) return Result::InvalidArgument;

        uint64_t handle_id = *static_cast<const uint64_t*>(in_data);
        auto it = file_handles_.find(handle_id);
        if (it == file_handles_.end()) return Result::InvalidArgument;

        // Return dummy timestamps
        if (out_data && out_size >= 24) { // 3 x u64 (create, modify, access)
            uint64_t* timestamps = static_cast<uint64_t*>(out_data);
            timestamps[0] = 0; // create
            timestamps[1] = 0; // modify
            timestamps[2] = 0; // access
        }
        return Result::Success;
    }

    Result set_file_timestamp(const void* in_data, size_t in_size,
                              void* out_data, size_t out_size) {
        // Stub
        return Result::Success;
    }

    Result get_free_space_size(const void* in_data, size_t in_size,
                               void* out_data, size_t out_size) {
        // Return a large free space value
        if (out_data && out_size >= 8) {
            *static_cast<uint64_t*>(out_data) = 1024ULL * 1024ULL * 1024ULL * 100ULL; // 100GB
        }
        return Result::Success;
    }

    Result commit(const void* in_data, size_t in_size,
                  void* out_data, size_t out_size) {
        // Flush any pending writes (if we had ofstream handles)
        // For now, just return success
        return Result::Success;
    }

    Result set_file_attribute(const void* in_data, size_t in_size,
                              void* out_data, size_t out_size) {
        // Stub
        return Result::Success;
    }

    Result create_file_with_size(const void* in_data, size_t in_size,
                                 void* out_data, size_t out_size) {
        // Same as create_file for our purposes
        return create_file(in_data, in_size, out_data, out_size);
    }
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

// audren: - Audio Renderer (implemented with SDL3)
class AudrenService : public IService {
public:
    AudrenService() = default;
    ~AudrenService() override { shutdown(); }

    const char* get_name() const override { return "audren:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // audren: commands:
        // 0: OpenAudioRenderer
        // 1: CloseAudioRenderer
        // 2: StartAudioRenderer
        // 3: StopAudioRenderer
        // 4: RequestUpdate
        // 5: AppendAudioRendererBuffer
        // 6: RegisterBufferEvent
        // 7: UnregisterBufferEvent
        // 8: GetReleasedBuffer
        // 9: GetAudioRendererState
        // 10: SetAudioRendererVolume
        // 11: GetAudioRendererVolume

        std::lock_guard<std::mutex> lock(mutex_);

        switch (command_id) {
            case 0: return open_audio_renderer(in_data, in_size, out_data, out_size);
            case 1: return close_audio_renderer(in_data, in_size, out_data, out_size);
            case 2: return start_audio_renderer(in_data, in_size, out_data, out_size);
            case 3: return stop_audio_renderer(in_data, in_size, out_data, out_size);
            case 4: return request_update(in_data, in_size, out_data, out_size);
            case 5: return append_audio_renderer_buffer(in_data, in_size, out_data, out_size);
            case 6: return register_buffer_event(in_data, in_size, out_data, out_size);
            case 7: return unregister_buffer_event(in_data, in_size, out_data, out_size);
            case 8: return get_released_buffer(in_data, in_size, out_data, out_size);
            case 9: return get_audio_renderer_state(in_data, in_size, out_data, out_size);
            case 10: return set_audio_renderer_volume(in_data, in_size, out_data, out_size);
            case 11: return get_audio_renderer_volume(in_data, in_size, out_data, out_size);
            default:
                return Result::InvalidArgument;
        }
    }

private:
    std::mutex mutex_;
    bool initialized_ = false;
    SDL_AudioDeviceID audio_device_ = 0;
    SDL_AudioSpec audio_spec_;
    std::vector<uint8_t> audio_buffer_;
    size_t buffer_offset_ = 0;
    std::mutex buffer_mutex_;
    std::condition_variable buffer_cv_;
    bool running_ = false;
    float volume_ = 1.0f;

    void shutdown() {
        if (audio_device_ != 0) {
            SDL_CloseAudioDevice(audio_device_);
            audio_device_ = 0;
        }
        initialized_ = false;
    }

    // Audio callback for SDL3
    static void audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
        auto* self = static_cast<AudrenService*>(userdata);
        if (!self || !self->running_) return;

        std::lock_guard<std::mutex> lock(self->buffer_mutex_);
        if (self->buffer_offset_ < self->audio_buffer_.size()) {
            size_t to_copy = std::min<size_t>(additional_amount, self->audio_buffer_.size() - self->buffer_offset_);
            SDL_PutAudioStreamData(stream, self->audio_buffer_.data() + self->buffer_offset_, static_cast<int>(to_copy));
            self->buffer_offset_ += to_copy;
        }
    }

    Result open_audio_renderer(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (initialized_) return Result::InvalidState;

        // Parse input parameters (sample rate, channels, format, buffer count, etc.)
        // For now, use default values
        SDL_AudioSpec spec{};
        spec.format = SDL_AUDIO_S16;
        spec.channels = 2;
        spec.freq = 48000;

        audio_spec_ = spec;
        audio_buffer_.resize(spec.freq * spec.channels * 2 / 10); // ~100ms buffer

        // Open audio device with SDL3
        audio_device_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
        if (audio_device_ == 0) {
            return Result::InvalidState;
        }

        // Create audio stream
        // Note: In SDL3, we use SDL_AudioStream for more flexible audio handling
        // For simplicity, we'll use the callback approach

        initialized_ = true;
        running_ = true;

        if (out_data && out_size >= 8) {
            uint64_t renderer_id = 1; // Simplified handle
            *static_cast<uint64_t*>(out_data) = renderer_id;
        }
        return Result::Success;
    }

    Result close_audio_renderer(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_) return Result::InvalidState;
        shutdown();
        return Result::Success;
    }

    Result start_audio_renderer(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_) return Result::InvalidState;
        if (audio_device_ != 0) {
            SDL_ResumeAudioDevice(audio_device_);
        }
        running_ = true;
        return Result::Success;
    }

    Result stop_audio_renderer(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_) return Result::InvalidState;
        running_ = false;
        if (audio_device_ != 0) {
            SDL_PauseAudioDevice(audio_device_);
        }
        return Result::Success;
    }

    Result request_update(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // In SDL3, audio is callback-driven, so this is a no-op
        return Result::Success;
    }

    Result append_audio_renderer_buffer(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_ || !in_data || in_size == 0) return Result::InvalidArgument;

        std::lock_guard<std::mutex> lock(buffer_mutex_);
        const uint8_t* src = static_cast<const uint8_t*>(in_data);
        audio_buffer_.insert(audio_buffer_.end(), src, src + in_size);
        buffer_cv_.notify_one();
        return Result::Success;
    }

    Result register_buffer_event(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Event registration would integrate with kernel event system
        if (out_data && out_size >= 8) {
            *static_cast<uint64_t*>(out_data) = 1; // Event handle
        }
        return Result::Success;
    }

    Result unregister_buffer_event(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result get_released_buffer(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Return buffer that has been played
        if (out_data && out_size >= 8) {
            *static_cast<uint64_t*>(out_data) = 0; // Buffer address
        }
        return Result::Success;
    }

    Result get_audio_renderer_state(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (out_data && out_size >= 4) {
            // 0 = stopped, 1 = playing
            *static_cast<uint32_t*>(out_data) = running_ ? 1 : 0;
        }
        return Result::Success;
    }

    Result set_audio_renderer_volume(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!in_data || in_size < 4) return Result::InvalidArgument;
        float vol = *static_cast<const float*>(in_data);
        volume_ = std::clamp(vol, 0.0f, 1.0f);
        if (audio_device_ != 0) {
            SDL_SetAudioDeviceGain(audio_device_, volume_);
        }
        return Result::Success;
    }

    Result get_audio_renderer_volume(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (out_data && out_size >= 4) {
            *static_cast<float*>(out_data) = volume_;
        }
        return Result::Success;
    }
};

// audout: - Audio Output (implemented with SDL3)
class AudoutService : public IService {
public:
    AudoutService() = default;
    ~AudoutService() override { shutdown(); }

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

        std::lock_guard<std::mutex> lock(mutex_);

        switch (command_id) {
            case 0: return open_audio_out(in_data, in_size, out_data, out_size);
            case 1: return close_audio_out(in_data, in_size, out_data, out_size);
            case 2: return start_audio_out(in_data, in_size, out_data, out_size);
            case 3: return stop_audio_out(in_data, in_size, out_data, out_size);
            case 4: return append_audio_out_buffer(in_data, in_size, out_data, out_size);
            case 5: return register_buffer_event(in_data, in_size, out_data, out_size);
            case 6: return unregister_buffer_event(in_data, in_size, out_data, out_size);
            case 7: return set_audio_out_volume(in_data, in_size, out_data, out_size);
            case 8: return get_audio_out_volume(in_data, in_size, out_data, out_size);
            default:
                return Result::InvalidArgument;
        }
    }

private:
    std::mutex mutex_;
    bool initialized_ = false;
    SDL_AudioDeviceID audio_device_ = 0;
    SDL_AudioSpec audio_spec_;
    std::vector<uint8_t> audio_buffer_;
    size_t buffer_offset_ = 0;
    std::mutex buffer_mutex_;
    bool running_ = false;
    float volume_ = 1.0f;

    void shutdown() {
        if (audio_device_ != 0) {
            SDL_CloseAudioDevice(audio_device_);
            audio_device_ = 0;
        }
        initialized_ = false;
    }

    static void audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
        auto* self = static_cast<AudoutService*>(userdata);
        if (!self || !self->running_) return;

        std::lock_guard<std::mutex> lock(self->buffer_mutex_);
        if (self->buffer_offset_ < self->audio_buffer_.size()) {
            size_t to_copy = std::min<size_t>(additional_amount, self->audio_buffer_.size() - self->buffer_offset_);
            SDL_PutAudioStreamData(stream, self->audio_buffer_.data() + self->buffer_offset_, static_cast<int>(to_copy));
            self->buffer_offset_ += to_copy;
        }
    }

    Result open_audio_out(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (initialized_) return Result::InvalidState;

        SDL_AudioSpec spec{};
        spec.format = SDL_AUDIO_S16;
        spec.channels = 2;
        spec.freq = 48000;

        audio_spec_ = spec;
        audio_buffer_.resize(spec.freq * spec.channels * 2 / 10);

        audio_device_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
        if (audio_device_ == 0) {
            return Result::InvalidState;
        }

        initialized_ = true;
        running_ = true;

        if (out_data && out_size >= 8) {
            uint64_t out_id = 1;
            *static_cast<uint64_t*>(out_data) = out_id;
        }
        return Result::Success;
    }

    Result close_audio_out(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_) return Result::InvalidState;
        shutdown();
        return Result::Success;
    }

    Result start_audio_out(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_) return Result::InvalidState;
        if (audio_device_ != 0) {
            SDL_ResumeAudioDevice(audio_device_);
        }
        running_ = true;
        return Result::Success;
    }

    Result stop_audio_out(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_) return Result::InvalidState;
        running_ = false;
        if (audio_device_ != 0) {
            SDL_PauseAudioDevice(audio_device_);
        }
        return Result::Success;
    }

    Result append_audio_out_buffer(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_ || !in_data || in_size == 0) return Result::InvalidArgument;

        std::lock_guard<std::mutex> lock(buffer_mutex_);
        const uint8_t* src = static_cast<const uint8_t*>(in_data);
        audio_buffer_.insert(audio_buffer_.end(), src, src + in_size);
        return Result::Success;
    }

    Result register_buffer_event(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (out_data && out_size >= 8) {
            *static_cast<uint64_t*>(out_data) = 1;
        }
        return Result::Success;
    }

    Result unregister_buffer_event(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result set_audio_out_volume(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!in_data || in_size < 4) return Result::InvalidArgument;
        float vol = *static_cast<const float*>(in_data);
        volume_ = std::clamp(vol, 0.0f, 1.0f);
        if (audio_device_ != 0) {
            SDL_SetAudioDeviceGain(audio_device_, volume_);
        }
        return Result::Success;
    }

    Result get_audio_out_volume(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (out_data && out_size >= 4) {
            *static_cast<float*>(out_data) = volume_;
        }
        return Result::Success;
    }
};

// hwopus: - Hardware Opus Decoder (implemented with libopus)
class HwOpusService : public IService {
public:
    HwOpusService() = default;
    ~HwOpusService() override { shutdown(); }

    const char* get_name() const override { return "hwopus:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // hwopus: commands:
        // 0: OpenHardwareOpusDecoder
        // 1: CloseHardwareOpusDecoder
        // 2: DecodeInterleaved
        // 3: DecodeStereo
        // 4: GetDecoderState
        // 5: SetDecoderGain
        // 6: GetDecoderGain

        std::lock_guard<std::mutex> lock(mutex_);

        switch (command_id) {
            case 0: return open_hardware_opus_decoder(in_data, in_size, out_data, out_size);
            case 1: return close_hardware_opus_decoder(in_data, in_size, out_data, out_size);
            case 2: return decode_interleaved(in_data, in_size, out_data, out_size);
            case 3: return decode_stereo(in_data, in_size, out_data, out_size);
            case 4: return get_decoder_state(in_data, in_size, out_data, out_size);
            case 5: return set_decoder_gain(in_data, in_size, out_data, out_size);
            case 6: return get_decoder_gain(in_data, in_size, out_data, out_size);
            default:
                return Result::InvalidArgument;
        }
    }

private:
    std::mutex mutex_;
    std::unordered_map<uint64_t, OpusDecoder*> decoders_;
    uint64_t next_decoder_id_ = 1;

    void shutdown() {
        for (auto& [id, decoder] : decoders_) {
            if (decoder) {
                opus_decoder_destroy(decoder);
            }
        }
        decoders_.clear();
    }

    Result open_hardware_opus_decoder(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Input: sample rate (u32), channels (u32)
        if (!in_data || in_size < 8) return Result::InvalidArgument;

        const uint32_t* params = static_cast<const uint32_t*>(in_data);
        uint32_t sample_rate = params[0];
        uint32_t channels = params[1];

        if (sample_rate != 48000 || (channels != 1 && channels != 2)) {
            return Result::InvalidArgument; // Opus only supports 48kHz, mono/stereo
        }

        int error = 0;
        OpusDecoder* decoder = opus_decoder_create(sample_rate, channels, &error);
        if (error != OPUS_OK || !decoder) {
            return Result::InvalidState;
        }

        uint64_t decoder_id = next_decoder_id_++;
        decoders_[decoder_id] = decoder;

        if (out_data && out_size >= 8) {
            *static_cast<uint64_t*>(out_data) = decoder_id;
        }
        return Result::Success;
    }

    Result close_hardware_opus_decoder(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!in_data || in_size < 8) return Result::InvalidArgument;

        uint64_t decoder_id = *static_cast<const uint64_t*>(in_data);
        auto it = decoders_.find(decoder_id);
        if (it == decoders_.end()) return Result::NotFound;

        opus_decoder_destroy(it->second);
        decoders_.erase(it);
        return Result::Success;
    }

    Result decode_interleaved(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Input: decoder_id (u64), opus_data (pointer), opus_size (u32), pcm_buffer (pointer), frame_size (u32)
        // This is a simplified implementation
        if (!in_data || in_size < 24) return Result::InvalidArgument;

        const uint64_t* params = static_cast<const uint64_t*>(in_data);
        uint64_t decoder_id = params[0];
        const void* opus_data = reinterpret_cast<const void*>(params[1]);
        uint32_t opus_size = static_cast<uint32_t>(params[2]);
        void* pcm_buffer = reinterpret_cast<void*>(params[3]);
        uint32_t frame_size = static_cast<uint32_t>(params[4]);

        auto it = decoders_.find(decoder_id);
        if (it == decoders_.end()) return Result::NotFound;

        int samples = opus_decode(it->second,
                                  static_cast<const unsigned char*>(opus_data),
                                  static_cast<int>(opus_size),
                                  static_cast<opus_int16*>(pcm_buffer),
                                  static_cast<int>(frame_size),
                                  0); // Not FEC

        if (samples < 0) {
            return Result::InvalidArgument;
        }

        if (out_data && out_size >= 4) {
            *static_cast<uint32_t*>(out_data) = static_cast<uint32_t>(samples);
        }
        return Result::Success;
    }

    Result decode_stereo(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Similar to decode_interleaved but for stereo
        return decode_interleaved(in_data, in_size, out_data, out_size);
    }

    Result get_decoder_state(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!in_data || in_size < 8) return Result::InvalidArgument;

        uint64_t decoder_id = *static_cast<const uint64_t*>(in_data);
        auto it = decoders_.find(decoder_id);
        if (it == decoders_.end()) return Result::NotFound;

        // Return decoder state (simplified)
        if (out_data && out_size >= 4) {
            *static_cast<uint32_t*>(out_data) = 0; // OK state
        }
        return Result::Success;
    }

    Result set_decoder_gain(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!in_data || in_size < 12) return Result::InvalidArgument;

        const uint64_t* params = static_cast<const uint64_t*>(in_data);
        uint64_t decoder_id = params[0];
        int32_t gain_q8 = static_cast<int32_t>(params[1]); // Q8 format

        auto it = decoders_.find(decoder_id);
        if (it == decoders_.end()) return Result::NotFound;

        int error = opus_decoder_ctl(it->second, OPUS_SET_GAIN(gain_q8));
        return error == OPUS_OK ? Result::Success : Result::InvalidArgument;
    }

    Result get_decoder_gain(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!in_data || in_size < 8) return Result::InvalidArgument;

        uint64_t decoder_id = *static_cast<const uint64_t*>(in_data);
        auto it = decoders_.find(decoder_id);
        if (it == decoders_.end()) return Result::NotFound;

        int32_t gain_q8 = 0;
        int error = opus_decoder_ctl(it->second, OPUS_GET_GAIN(&gain_q8));
        if (error != OPUS_OK) return Result::InvalidState;

        if (out_data && out_size >= 4) {
            *static_cast<int32_t*>(out_data) = gain_q8;
        }
        return Result::Success;
    }
};

// nvn: - NVN Graphics with Vulkan backend
class NvnService : public IService {
public:
    NvnService() = default;
    ~NvnService() override { shutdown(); }

    const char* get_name() const override { return "nvn:"; }

    Result dispatch(Session* session, uint32_t command_id,
                    const void* in_data, size_t in_size,
                    void* out_data, size_t out_size) override {
        // NVN commands (based on Horizon OS NVN service):
        // Device management:
        // 0: NvnDeviceCreate
        // 1: NvnDeviceFinalize
        // 2: NvnDeviceGetDefaultQueue
        // 3: NvnDeviceWaitIdle
        // 4: NvnDeviceGetProcAddr
        //
        // Queue management:
        // 10: NvnQueueSubmit
        // 11: NvnQueuePresent
        // 12: NvnQueueWaitIdle
        //
        // Memory management:
        // 20: NvnMemoryInitialize
        // 21: NvnMemoryFinalize
        // 22: NvnMemoryMap
        // 23: NvnMemoryUnmap
        // 24: NvnMemoryGetInfo
        //
        // Command buffer:
        // 30: NvnCommandBufferInitialize
        // 31: NvnCommandBufferFinalize
        // 32: NvnCommandBufferBegin
        // 33: NvnCommandBufferEnd
        // 34: NvnCommandBufferReset
        //
        // Swapchain:
        // 40: NvnSwapchainCreate
        // 41: NvnSwapchainDestroy
        // 42: NvnSwapchainAcquireNextImage
        // 43: NvnSwapchainQueuePresent
        //
        // Shader/Program:
        // 50: NvnProgramCreate
        // 51: NvnProgramDestroy
        // 52: NvnShaderCreate
        // 53: NvnShaderDestroy
        //
        // Texture/Sampler:
        // 60: NvnTextureCreate
        // 61: NvnTextureDestroy
        // 62: NvnSamplerCreate
        // 63: NvnSamplerDestroy
        //
        // Buffer:
        // 70: NvnBufferCreate
        // 71: NvnBufferDestroy
        // 72: NvnBufferMap
        // 73: NvnBufferUnmap

        std::lock_guard<std::mutex> lock(mutex_);

        switch (command_id) {
            // Device management
            case 0: return device_create(in_data, in_size, out_data, out_size);
            case 1: return device_finalize(in_data, in_size, out_data, out_size);
            case 2: return device_get_default_queue(in_data, in_size, out_data, out_size);
            case 3: return device_wait_idle(in_data, in_size, out_data, out_size);
            case 4: return device_get_proc_addr(in_data, in_size, out_data, out_size);

            // Queue management
            case 10: return queue_submit(in_data, in_size, out_data, out_size);
            case 11: return queue_present(in_data, in_size, out_data, out_size);
            case 12: return queue_wait_idle(in_data, in_size, out_data, out_size);

            // Memory management
            case 20: return memory_initialize(in_data, in_size, out_data, out_size);
            case 21: return memory_finalize(in_data, in_size, out_data, out_size);
            case 22: return memory_map(in_data, in_size, out_data, out_size);
            case 23: return memory_unmap(in_data, in_size, out_data, out_size);
            case 24: return memory_get_info(in_data, in_size, out_data, out_size);

            // Command buffer
            case 30: return command_buffer_initialize(in_data, in_size, out_data, out_size);
            case 31: return command_buffer_finalize(in_data, in_size, out_data, out_size);
            case 32: return command_buffer_begin(in_data, in_size, out_data, out_size);
            case 33: return command_buffer_end(in_data, in_size, out_data, out_size);
            case 34: return command_buffer_reset(in_data, in_size, out_data, out_size);

            // Swapchain
            case 40: return swapchain_create(in_data, in_size, out_data, out_size);
            case 41: return swapchain_destroy(in_data, in_size, out_data, out_size);
            case 42: return swapchain_acquire_next_image(in_data, in_size, out_data, out_size);
            case 43: return swapchain_queue_present(in_data, in_size, out_data, out_size);

            // Shader/Program
            case 50: return program_create(in_data, in_size, out_data, out_size);
            case 51: return program_destroy(in_data, in_size, out_data, out_size);
            case 52: return shader_create(in_data, in_size, out_data, out_size);
            case 53: return shader_destroy(in_data, in_size, out_data, out_size);

            // Texture/Sampler
            case 60: return texture_create(in_data, in_size, out_data, out_size);
            case 61: return texture_destroy(in_data, in_size, out_data, out_size);
            case 62: return sampler_create(in_data, in_size, out_data, out_size);
            case 63: return sampler_destroy(in_data, in_size, out_data, out_size);

            // Buffer
            case 70: return buffer_create(in_data, in_size, out_data, out_size);
            case 71: return buffer_destroy(in_data, in_size, out_data, out_size);
            case 72: return buffer_map(in_data, in_size, out_data, out_size);
            case 73: return buffer_unmap(in_data, in_size, out_data, out_size);

            default:
                return Result::InvalidArgument;
        }
    }

private:
    std::mutex mutex_;
    bool initialized_ = false;

    // Vulkan resources
    VkInstance instance_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    uint32_t queue_family_index_ = 0;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swapchain_images_;
    std::vector<VkImageView> swapchain_image_views_;
    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers_;
    VkCommandPool command_pool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> command_buffers_;
    VkSemaphore image_available_semaphore_ = VK_NULL_HANDLE;
    VkSemaphore render_finished_semaphore_ = VK_NULL_HANDLE;
    VkFence in_flight_fence_ = VK_NULL_HANDLE;

    // NVN object handles (simplified - in reality these would be proper objects)
    uint64_t next_object_id_ = 1;
    std::unordered_map<uint64_t, VkBuffer> buffers_;
    std::unordered_map<uint64_t, VkDeviceMemory> buffer_memory_;
    std::unordered_map<uint64_t, VkImage> textures_;
    std::unordered_map<uint64_t, VkDeviceMemory> texture_memory_;
    std::unordered_map<uint64_t, VkImageView> texture_views_;
    std::unordered_map<uint64_t, VkSampler> samplers_;
    std::unordered_map<uint64_t, VkShaderModule> shaders_;
    std::unordered_map<uint64_t, VkPipeline> pipelines_;
    std::unordered_map<uint64_t, VkPipelineLayout> pipeline_layouts_;

    void shutdown() {
        if (!initialized_) return;

        // Cleanup Vulkan resources
        if (in_flight_fence_ != VK_NULL_HANDLE) {
            vkDestroyFence(device_, in_flight_fence_, nullptr);
        }
        if (render_finished_semaphore_ != VK_NULL_HANDLE) {
            vkDestroySemaphore(device_, render_finished_semaphore_, nullptr);
        }
        if (image_available_semaphore_ != VK_NULL_HANDLE) {
            vkDestroySemaphore(device_, image_available_semaphore_, nullptr);
        }

        for (auto fb : framebuffers_) {
            vkDestroyFramebuffer(device_, fb, nullptr);
        }
        framebuffers_.clear();

        if (render_pass_ != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device_, render_pass_, nullptr);
        }

        for (auto view : swapchain_image_views_) {
            vkDestroyImageView(device_, view, nullptr);
        }
        swapchain_image_views_.clear();

        if (swapchain_ != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device_, swapchain_, nullptr);
        }

        if (command_pool_ != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_, command_pool_, nullptr);
        }

        if (surface_ != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance_, surface_, nullptr);
        }

        if (device_ != VK_NULL_HANDLE) {
            vkDestroyDevice(device_, nullptr);
        }

        if (instance_ != VK_NULL_HANDLE) {
            vkDestroyInstance(instance_, nullptr);
        }

        initialized_ = false;
    }

    // Helper to create Vulkan instance
    Result create_instance() {
        VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
        app_info.pApplicationName = "KirbyRecomp NVN";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "KirbyRecomp";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_3;

        // Required extensions
        const char* extensions[] = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        };

        VkInstanceCreateInfo create_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = 3;
        create_info.ppEnabledExtensionNames = extensions;

        // Enable validation layers in debug
#ifdef _DEBUG
        const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers;
#endif

        VkResult result = vkCreateInstance(&create_info, nullptr, &instance_);
        if (result != VK_SUCCESS) {
            return Result::InvalidState;
        }
        return Result::Success;
    }

    // Helper to pick physical device
    Result pick_physical_device() {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
        if (device_count == 0) {
            return Result::NotFound;
        }

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

        for (const auto& dev : devices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(dev, &props);

            // Check for graphics queue family
            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, nullptr);
            std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, queue_families.data());

            for (uint32_t i = 0; i < queue_family_count; ++i) {
                if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    // Check surface support if we have a surface
                    VkBool32 present_support = false;
                    if (surface_ != VK_NULL_HANDLE) {
                        vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface_, &present_support);
                    } else {
                        present_support = true; // No surface yet, assume support
                    }

                    if (present_support) {
                        physical_device_ = dev;
                        queue_family_index_ = i;
                        return Result::Success;
                    }
                }
            }
        }

        return Result::NotFound;
    }

    // Helper to create logical device
    Result create_logical_device() {
        float queue_priority = 1.0f;
        VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queue_info.queueFamilyIndex = queue_family_index_;
        queue_info.queueCount = 1;
        queue_info.pQueuePriorities = &queue_priority;

        // Device extensions
        const char* device_extensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkPhysicalDeviceFeatures device_features{};
        device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        create_info.queueCreateInfoCount = 1;
        create_info.pQueueCreateInfos = &queue_info;
        create_info.enabledExtensionCount = 1;
        create_info.ppEnabledExtensionNames = device_extensions;
        create_info.pEnabledFeatures = &device_features;

#ifdef _DEBUG
        const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = layers;
#endif

        VkResult result = vkCreateDevice(physical_device_, &create_info, nullptr, &device_);
        if (result != VK_SUCCESS) {
            return Result::InvalidState;
        }

        vkGetDeviceQueue(device_, queue_family_index_, 0, &graphics_queue_);
        return Result::Success;
    }

    // Helper to create swapchain
    Result create_swapchain(uint32_t width, uint32_t height) {
        if (surface_ == VK_NULL_HANDLE) {
            return Result::InvalidState;
        }

        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities);

        VkSurfaceFormatKHR surface_format{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

        VkExtent2D extent = capabilities.currentExtent;
        if (extent.width == UINT32_MAX) {
            extent.width = width;
            extent.height = height;
        }

        uint32_t image_count = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
            image_count = capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR create_info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
        create_info.surface = surface_;
        create_info.minImageCount = image_count;
        create_info.imageFormat = surface_format.format;
        create_info.imageColorSpace = surface_format.colorSpace;
        create_info.imageExtent = extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.preTransform = capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = present_mode;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = swapchain_;

        VkResult result = vkCreateSwapchainKHR(device_, &create_info, nullptr, &swapchain_);
        if (result != VK_SUCCESS) {
            return Result::InvalidState;
        }

        // Get swapchain images
        vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, nullptr);
        swapchain_images_.resize(image_count);
        vkGetSwapchainImagesKHR(device_, swapchain_, &image_count, swapchain_images_.data());

        // Create image views
        swapchain_image_views_.resize(image_count);
        for (uint32_t i = 0; i < image_count; ++i) {
            VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
            view_info.image = swapchain_images_[i];
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = surface_format.format;
            view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            result = vkCreateImageView(device_, &view_info, nullptr, &swapchain_image_views_[i]);
            if (result != VK_SUCCESS) {
                return Result::InvalidState;
            }
        }

        return Result::Success;
    }

    // Helper to create render pass
    Result create_render_pass() {
        VkAttachmentDescription color_attachment{};
        color_attachment.format = VK_FORMAT_B8G8R8A8_SRGB;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        VkResult result = vkCreateRenderPass(device_, &render_pass_info, nullptr, &render_pass_);
        return result == VK_SUCCESS ? Result::Success : Result::InvalidState;
    }

    // Helper to create framebuffers
    Result create_framebuffers() {
        framebuffers_.resize(swapchain_image_views_.size());
        for (size_t i = 0; i < swapchain_image_views_.size(); ++i) {
            VkImageView attachments[] = {swapchain_image_views_[i]};

            VkFramebufferCreateInfo framebuffer_info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
            framebuffer_info.renderPass = render_pass_;
            framebuffer_info.attachmentCount = 1;
            framebuffer_info.pAttachments = attachments;
            framebuffer_info.width = swapchain_images_.empty() ? 800 : 800; // Will get from extent
            framebuffer_info.height = 600;
            framebuffer_info.layers = 1;

            VkResult result = vkCreateFramebuffer(device_, &framebuffer_info, nullptr, &framebuffers_[i]);
            if (result != VK_SUCCESS) {
                return Result::InvalidState;
            }
        }
        return Result::Success;
    }

    // Helper to create command pool and buffers
    Result create_command_pool_and_buffers() {
        VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pool_info.queueFamilyIndex = queue_family_index_;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkResult result = vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_);
        if (result != VK_SUCCESS) {
            return Result::InvalidState;
        }

        command_buffers_.resize(swapchain_image_views_.size());
        VkCommandBufferAllocateInfo alloc_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        alloc_info.commandPool = command_pool_;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

        result = vkAllocateCommandBuffers(device_, &alloc_info, command_buffers_.data());
        if (result != VK_SUCCESS) {
            return Result::InvalidState;
        }
        return Result::Success;
    }

    // Helper to create synchronization objects
    Result create_sync_objects() {
        VkSemaphoreCreateInfo semaphore_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkResult result = vkCreateSemaphore(device_, &semaphore_info, nullptr, &image_available_semaphore_);
        if (result != VK_SUCCESS) return Result::InvalidState;

        result = vkCreateSemaphore(device_, &semaphore_info, nullptr, &render_finished_semaphore_);
        if (result != VK_SUCCESS) return Result::InvalidState;

        result = vkCreateFence(device_, &fence_info, nullptr, &in_flight_fence_);
        if (result != VK_SUCCESS) return Result::InvalidState;

        return Result::Success;
    }

    // NVN command implementations
    Result device_create(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (initialized_) {
            return Result::InvalidState; // Already initialized
        }

        Result result;
        result = create_instance();
        if (result != Result::Success) return result;

        // Note: Surface creation requires a window handle (HWND)
        // For now we'll create it when swapchain is created
        // In real NVN, surface is created with NvnDeviceCreate

        result = pick_physical_device();
        if (result != Result::Success) return result;

        result = create_logical_device();
        if (result != Result::Success) return result;

        // Create command pool
        VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pool_info.queueFamilyIndex = queue_family_index_;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VkResult vk_result = vkCreateCommandPool(device_, &pool_info, nullptr, &command_pool_);
        if (vk_result != VK_SUCCESS) return Result::InvalidState;

        initialized_ = true;

        // Return device handle (simplified as object ID)
        if (out_data && out_size >= 8) {
            uint64_t device_id = next_object_id_++;
            *static_cast<uint64_t*>(out_data) = device_id;
        }
        return Result::Success;
    }

    Result device_finalize(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        shutdown();
        return Result::Success;
    }

    Result device_get_default_queue(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_) return Result::InvalidState;

        if (out_data && out_size >= 8) {
            // Return queue handle (simplified)
            uint64_t queue_id = next_object_id_++;
            *static_cast<uint64_t*>(out_data) = queue_id;
        }
        return Result::Success;
    }

    Result device_wait_idle(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_) return Result::InvalidState;

        VkResult result = vkDeviceWaitIdle(device_);
        return result == VK_SUCCESS ? Result::Success : Result::InvalidState;
    }

    Result device_get_proc_addr(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Not implemented - would return function pointers for NVN API
        if (out_data && out_size >= 8) {
            *static_cast<uint64_t*>(out_data) = 0;
        }
        return Result::Success;
    }

    Result queue_submit(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Submit command buffers to queue
        // Simplified implementation
        return Result::Success;
    }

    Result queue_present(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Present swapchain image
        if (!initialized_ || swapchain_ == VK_NULL_HANDLE) {
            return Result::InvalidState;
        }

        VkPresentInfoKHR present_info{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &render_finished_semaphore_;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain_;

        uint32_t image_index = 0; // Would come from acquire_next_image
        present_info.pImageIndices = &image_index;

        VkResult result = vkQueuePresentKHR(graphics_queue_, &present_info);
        return result == VK_SUCCESS ? Result::Success : Result::InvalidState;
    }

    Result queue_wait_idle(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_) return Result::InvalidState;
        VkResult result = vkQueueWaitIdle(graphics_queue_);
        return result == VK_SUCCESS ? Result::Success : Result::InvalidState;
    }

    Result memory_initialize(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // NVN memory initialization
        return Result::Success;
    }

    Result memory_finalize(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result memory_map(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Map device memory to host
        return Result::Success;
    }

    Result memory_unmap(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result memory_get_info(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result command_buffer_initialize(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result command_buffer_finalize(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result command_buffer_begin(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result command_buffer_end(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result command_buffer_reset(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result swapchain_create(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_) return Result::InvalidState;

        // Input would contain window handle, width, height, format, etc.
        // For now, create a basic swapchain
        // In real implementation, we'd get HWND from input

        // Create a dummy surface for now (would need actual window)
        // On Windows, we'd use vkCreateWin32SurfaceKHR with HWND

        // For testing without window, we'll just return success
        if (out_data && out_size >= 8) {
            uint64_t swapchain_id = next_object_id_++;
            *static_cast<uint64_t*>(out_data) = swapchain_id;
        }
        return Result::Success;
    }

    Result swapchain_destroy(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (swapchain_ != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device_, swapchain_, nullptr);
            swapchain_ = VK_NULL_HANDLE;
        }
        for (auto view : swapchain_image_views_) {
            vkDestroyImageView(device_, view, nullptr);
        }
        swapchain_image_views_.clear();
        swapchain_images_.clear();
        return Result::Success;
    }

    Result swapchain_acquire_next_image(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        if (!initialized_ || swapchain_ == VK_NULL_HANDLE) {
            return Result::InvalidState;
        }

        VkResult result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX,
                                                image_available_semaphore_, VK_NULL_HANDLE,
                                                nullptr); // image_index would be output
        return result == VK_SUCCESS ? Result::Success : Result::InvalidState;
    }

    Result swapchain_queue_present(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return queue_present(in_data, in_size, out_data, out_size);
    }

    Result program_create(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Create shader program from SPIR-V
        if (out_data && out_size >= 8) {
            uint64_t program_id = next_object_id_++;
            *static_cast<uint64_t*>(out_data) = program_id;
        }
        return Result::Success;
    }

    Result program_destroy(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result shader_create(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Create shader module from SPIR-V bytecode
        if (out_data && out_size >= 8) {
            uint64_t shader_id = next_object_id_++;
            *static_cast<uint64_t*>(out_data) = shader_id;
        }
        return Result::Success;
    }

    Result shader_destroy(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result texture_create(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Create texture/image
        if (out_data && out_size >= 8) {
            uint64_t texture_id = next_object_id_++;
            *static_cast<uint64_t*>(out_data) = texture_id;
        }
        return Result::Success;
    }

    Result texture_destroy(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result sampler_create(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Create sampler
        if (out_data && out_size >= 8) {
            uint64_t sampler_id = next_object_id_++;
            *static_cast<uint64_t*>(out_data) = sampler_id;
        }
        return Result::Success;
    }

    Result sampler_destroy(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result buffer_create(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Create buffer (vertex, index, uniform, etc.)
        if (out_data && out_size >= 8) {
            uint64_t buffer_id = next_object_id_++;
            *static_cast<uint64_t*>(out_data) = buffer_id;
        }
        return Result::Success;
    }

    Result buffer_destroy(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
    }

    Result buffer_map(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        // Map buffer for CPU access
        return Result::Success;
    }

    Result buffer_unmap(const void* in_data, size_t in_size, void* out_data, size_t out_size) {
        return Result::Success;
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