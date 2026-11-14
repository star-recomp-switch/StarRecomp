#include "kirbyrecomp/runtime.h"

#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

namespace kr {

using sr::common::u64;

// Thread handle type
using ThreadHandle = u64;

// Thread local storage key for CPU state
static thread_local SrCpuState* tls_cpu_state = nullptr;

// Mutex for thread registry
static std::mutex g_thread_mutex;

// Thread registry: handle -> CPU state
static std::unordered_map<ThreadHandle, SrCpuState*> g_thread_registry;

// Next thread handle
static ThreadHandle g_next_thread_handle = 1;

// Set the current thread's CPU state
void sr_thread_set_cpu_state(SrCpuState* cpu) {
    tls_cpu_state = cpu;
}

// Get the current thread's CPU state
SrCpuState* sr_thread_get_cpu_state() {
    return tls_cpu_state;
}

// Create a new thread with the given entry point
ThreadHandle sr_thread_create(u64 entry_point, SrMemory* mem, void* arg) {
    std::lock_guard<std::mutex> lock(g_thread_mutex);

    ThreadHandle handle = g_next_thread_handle++;

    // Allocate CPU state for new thread
    SrCpuState* cpu = new SrCpuState();
    sr_cpu_init(cpu);
    cpu->pc = entry_point;
    cpu->x[0] = reinterpret_cast<u64>(arg); // X0 = argument

    // Store in registry
    g_thread_registry[handle] = cpu;

    // Launch thread
    std::thread([handle, cpu, mem]() {
        sr_thread_set_cpu_state(cpu);
        // In a real implementation, this would execute generated code
        // For now, just a placeholder
        (void)mem;
        sr_cpu_execute(cpu, mem, cpu->pc);

        // Cleanup on thread exit
        std::lock_guard<std::mutex> cleanup_lock(g_thread_mutex);
        auto it = g_thread_registry.find(handle);
        if (it != g_thread_registry.end()) {
            delete it->second;
            g_thread_registry.erase(it);
        }
    }).detach();

    return handle;
}

// Wait for a thread to complete
void sr_thread_join(ThreadHandle handle) {
    // In a real implementation, this would use condition variables
    // For now, just a placeholder
    (void)handle;
}

// Get the current thread's handle
ThreadHandle sr_thread_self() {
    // Find handle for current CPU state
    SrCpuState* cpu = sr_thread_get_cpu_state();
    if (!cpu) return 0;

    std::lock_guard<std::mutex> lock(g_thread_mutex);
    for (const auto& [handle, state] : g_thread_registry) {
        if (state == cpu) {
            return handle;
        }
    }
    return 0;
}

// Get CPU state for a thread handle
SrCpuState* sr_thread_get_cpu(ThreadHandle handle) {
    std::lock_guard<std::mutex> lock(g_thread_mutex);
    auto it = g_thread_registry.find(handle);
    return (it != g_thread_registry.end()) ? it->second : nullptr;
}

} // namespace kr