#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <mutex>

namespace kr::hle {

// Forward declaration
struct Service;
struct Session;
struct Server;

// Result codes (matching Horizon OS)
enum class Result : uint32_t {
    Success = 0,
    InvalidArgument = 0xA01,
    NotFound = 0xC01,
    OutOfMemory = 0xD01,
    InvalidState = 0xE01,
    PermissionDenied = 0xF01,
};

// IPC command structure
struct IpcCommand {
    uint32_t command_id;
    uint32_t num_in_objects;
    uint32_t num_out_objects;
    // In/out buffers would be handled by the runtime
};

// Base service interface
class IService {
public:
    virtual ~IService() = default;
    virtual const char* get_name() const = 0;
    virtual Result dispatch(Session* session, uint32_t command_id,
                            const void* in_data, size_t in_size,
                            void* out_data, size_t out_size) = 0;
};

// Session represents a client connection to a service
struct Session {
    uint64_t session_id;
    IService* service;
    void* user_data;
    uint64_t process_id;
    uint64_t thread_id;
};

// Service manager (sm:) - manages service registration and lookup
class ServiceManager {
public:
    static ServiceManager& instance();

    Result register_service(const std::string& name, std::unique_ptr<IService> service);
    Result unregister_service(const std::string& name);
    IService* get_service(const std::string& name);
    Session* create_session(const std::string& service_name);
    void close_session(uint64_t session_id);

private:
    ServiceManager() = default;
    std::unordered_map<std::string, std::unique_ptr<IService>> services_;
    std::unordered_map<uint64_t, Session> sessions_;
    uint64_t next_session_id_ = 1;
    std::recursive_mutex mutex_;
};

// Service factory for creating built-in services
class ServiceFactory {
public:
    static Result create_builtin_services(ServiceManager& sm);
};

} // namespace kr::hle