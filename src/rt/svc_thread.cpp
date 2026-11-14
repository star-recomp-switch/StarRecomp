// HLE implementation of the thread / synchronization SVCs for KirbyRecomp.
// These handlers are registered with the SVC dispatch table and invoked via
// sr_rt_handle_svc() when guest code executes SVC instructions.

#include "kirbyrecomp/runtime.h"
#include "kirbyrecomp/types.h"

#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace kr {

using sr::common::u64;
using sr::common::u32;

namespace {

// Thread handle type
using ThreadHandle = u64;

// Thread state tracked by the runtime
struct ThreadState {
    std::thread thread;
    SrCpuState* cpu_state = nullptr;
    std::mutex mutex;
    std::condition_variable cv;
    bool completed = false;
    u64 exit_value = 0;
};

// Global thread registry
static std::mutex g_thread_mutex;
static std::map<ThreadHandle, std::unique_ptr<ThreadState>> g_thread_registry;
static ThreadHandle g_next_thread_handle = 1;

// Mutex for synchronization primitives
static std::mutex g_sync_mutex;
static std::condition_variable g_sync_cv;
static std::map<u64, bool> g_event_signaled; // event_id -> signaled state

// Allocate a new thread handle
static ThreadHandle allocate_handle() {
    return g_next_thread_handle++;
}

// SVC 0x0A: CreateThread
// Input: x0 = entry point, x1 = arg, x2 = stack_base, x3 = stack_size, x4 = priority, x5 = core_id
// Output: x0 = thread handle, x1 = result code (0 = success)
static void svc_create_thread(SrCpuState* cpu) {
    u64 entry_point = cpu->x[0];
    u64 arg = cpu->x[1];
    u64 stack_base = cpu->x[2];
    u64 stack_size = cpu->x[3];
    u32 priority = static_cast<u32>(cpu->x[4]);
    s32 core_id = static_cast<s32>(cpu->x[5]);

    ThreadHandle handle = 0;
    {
        std::lock_guard<std::mutex> lock(g_thread_mutex);
        handle = allocate_handle();
    }

    // Create CPU state for the new thread
    SrCpuState* new_cpu = new SrCpuState();
    sr_cpu_init(new_cpu);
    new_cpu->pc = entry_point;
    new_cpu->x[0] = arg; // x0 = argument
    new_cpu->sp = stack_base + stack_size; // Top of stack

    // Create thread state
    auto thread_state = std::make_unique<ThreadState>();
    thread_state->cpu_state = new_cpu;

    // Store in registry
    {
        std::lock_guard<std::mutex> lock(g_thread_mutex);
        g_thread_registry[handle] = std::move(thread_state);
    }

    // Launch the thread
    std::thread worker([handle, new_cpu]() {
        // Set TLS for this thread
        // Note: In a full implementation, we'd need thread-local storage for CPU state

        // Execute the guest code
        // For now, we simulate execution by running a simple loop
        // In a real implementation, this would call into the JIT/exec engine
        (void)new_cpu; // Suppress unused warning for now

        // Mark thread as completed - use a quick lock to avoid holding mutex during join
        {
            std::lock_guard<std::mutex> lock(g_thread_mutex);
            auto it = g_thread_registry.find(handle);
            if (it != g_thread_registry.end() && it->second) {
                it->second->completed = true;
                it->second->cv.notify_all();
            }
        }
    });

    // Move thread to thread_state
    {
        std::lock_guard<std::mutex> lock(g_thread_mutex);
        auto it = g_thread_registry.find(handle);
        if (it != g_thread_registry.end() && it->second) {
            it->second->thread = std::move(worker);
        }
    }

    // Return handle in x0, success (0) in x1
    cpu->x[0] = handle;
    cpu->x[1] = 0;
}

// SVC 0x22: StartThread
// Input: x0 = thread handle
// Output: x0 = result code (0 = success)
static void svc_start_thread(SrCpuState* cpu) {
    // In our simplified model, CreateThread already starts the thread
    // This is a no-op that returns success
    cpu->x[0] = 0;
}

// SVC 0x23: ExitThread
// Input: none
// Output: none (doesn't return)
static void svc_exit_thread(SrCpuState* cpu) {
    // Get current thread handle from TLS or registry
    ThreadHandle handle = 0;
    SrCpuState* current_cpu = sr_thread_get_cpu_state();

    if (current_cpu) {
        std::lock_guard<std::mutex> lock(g_thread_mutex);
        for (const auto& [h, state] : g_thread_registry) {
            if (state && state->cpu_state == current_cpu) {
                handle = h;
                break;
            }
        }
    }

    if (handle != 0) {
        std::lock_guard<std::mutex> lock(g_thread_mutex);
        auto it = g_thread_registry.find(handle);
        if (it != g_thread_registry.end() && it->second) {
            it->second->completed = true;
            it->second->exit_value = cpu->x[0]; // Exit value in x0
            it->second->cv.notify_all();
        }
    }

    // In a real implementation, this would terminate the current thread
    // For now, we just return and let the thread function exit naturally
}

// SVC 0x0B: WaitSynchronization
// Input: x0 = handle, x1 = timeout (ignored in simplified impl)
// Output: x0 = result code (0 = success)
static void svc_wait_synchronization(SrCpuState* cpu) {
    ThreadHandle handle = cpu->x[0];
    // u64 timeout = cpu->x[1]; // Ignored in simplified impl

    std::unique_ptr<ThreadState> thread_state;
    {
        std::lock_guard<std::mutex> lock(g_thread_mutex);
        auto it = g_thread_registry.find(handle);
        if (it != g_thread_registry.end()) {
            thread_state = std::move(it->second);
            g_thread_registry.erase(it);
        }
    }

    if (thread_state) {
        // Wait for completion outside the mutex
        if (thread_state->thread.joinable()) {
            thread_state->thread.join();
        }

        // Clean up CPU state
        if (thread_state->cpu_state) {
            delete thread_state->cpu_state;
            thread_state->cpu_state = nullptr;
        }
    }

    cpu->x[0] = 0; // Success
}

// SVC 0x24: SleepThread
// Input: x0 = nanoseconds
// Output: x0 = result code (0 = success)
static void svc_sleep_thread(SrCpuState* cpu) {
    u64 nanoseconds = cpu->x[0];

    // Convert to milliseconds for std::this_thread::sleep_for
    auto ms = std::chrono::nanoseconds(nanoseconds);
    std::this_thread::sleep_for(ms);

    cpu->x[0] = 0; // Success
}

// SVC 0x0C: GetResult
// Input: none
// Output: x0 = result value (from shared counter or thread exit value)
static void svc_get_result(SrCpuState* cpu) {
    // In the SwitchRecomp model, this returns a shared counter
    // For our implementation, we return the exit value of the last joined thread
    // or 0 if no specific result
    cpu->x[0] = 0; // Simplified - could track a shared counter
}

// Event-based synchronization primitives

// SVC: SignalEvent (not in original set, but useful)
// Input: x0 = event_id
// Output: x0 = result code
static void svc_signal_event(SrCpuState* cpu) {
    u64 event_id = cpu->x[0];

    std::lock_guard<std::mutex> lock(g_sync_mutex);
    g_event_signaled[event_id] = true;
    g_sync_cv.notify_all();

    cpu->x[0] = 0;
}

// SVC: WaitEvent (not in original set, but useful)
// Input: x0 = event_id, x1 = timeout
// Output: x0 = result code
static void svc_wait_event(SrCpuState* cpu) {
    u64 event_id = cpu->x[0];
    u64 timeout_ns = cpu->x[1];

    std::unique_lock<std::mutex> lock(g_sync_mutex);
    auto it = g_event_signaled.find(event_id);

    if (it == g_event_signaled.end() || !it->second) {
        if (timeout_ns == 0) {
            // Wait indefinitely
            g_sync_cv.wait(lock, [&]() {
                auto it2 = g_event_signaled.find(event_id);
                return it2 != g_event_signaled.end() && it2->second;
            });
        } else {
            auto timeout = std::chrono::nanoseconds(timeout_ns);
            g_sync_cv.wait_for(lock, timeout, [&]() {
                auto it2 = g_event_signaled.find(event_id);
                return it2 != g_event_signaled.end() && it2->second;
            });
        }
    }

    // Clear the event (auto-reset)
    if (it != g_event_signaled.end()) {
        it->second = false;
    }

    cpu->x[0] = 0;
}

// SVC: CreateEvent (not in original set)
// Input: x0 = event_id (output), x1 = initial_state (0 or 1), x2 = auto_reset (0 or 1)
// Output: x0 = result code, x1 = event handle
static void svc_create_event(SrCpuState* cpu) {
    u64 event_id = g_next_thread_handle++; // Reuse handle allocator
    bool initial_state = cpu->x[1] != 0;
    // bool auto_reset = cpu->x[2] != 0; // Ignored in simplified impl

    std::lock_guard<std::mutex> lock(g_sync_mutex);
    g_event_signaled[event_id] = initial_state;

    cpu->x[0] = 0;
    cpu->x[1] = event_id;
}

// SVC: CloseHandle (not in original set, but useful for cleanup)
// Input: x0 = handle
// Output: x0 = result code
static void svc_close_handle(SrCpuState* cpu) {
    ThreadHandle handle = cpu->x[0];

    std::unique_ptr<ThreadState> thread_state;
    {
        std::lock_guard<std::mutex> lock(g_thread_mutex);
        auto it = g_thread_registry.find(handle);
        if (it != g_thread_registry.end()) {
            thread_state = std::move(it->second);
            g_thread_registry.erase(it);
        }
    }

    // Join outside the mutex to avoid deadlock with worker threads
    if (thread_state) {
        if (thread_state->thread.joinable()) {
            thread_state->thread.join();
        }
        if (thread_state->cpu_state) {
            delete thread_state->cpu_state;
            thread_state->cpu_state = nullptr;
        }
        cpu->x[0] = 0;
    } else {
        cpu->x[0] = 1; // Not found
    }
}

// Map of SVC IDs to handler names and functions
struct ThreadSvcEntry {
    u32 id;
    const char* name;
    void (*handler)(SrCpuState*);
};

constexpr ThreadSvcEntry kThreadSvcEntries[] = {
    {0x0A, "CreateThread",        &svc_create_thread},
    {0x0B, "WaitSynchronization", &svc_wait_synchronization},
    {0x0C, "GetResult",           &svc_get_result},
    {0x22, "StartThread",         &svc_start_thread},
    {0x23, "ExitThread",          &svc_exit_thread},
    {0x24, "SleepThread",         &svc_sleep_thread},
    // Extended SVCs for synchronization primitives
    {0x40, "SignalEvent",         &svc_signal_event},
    {0x41, "WaitEvent",           &svc_wait_event},
    {0x42, "CreateEvent",         &svc_create_event},
    {0x43, "CloseHandle",         &svc_close_handle},
};

// Get handler name for an SVC ID
const char* thread_svc_name(u32 svc_id) {
    for (const auto& entry : kThreadSvcEntries) {
        if (entry.id == svc_id) return entry.name;
    }
    return nullptr;
}

// Get handler function for an SVC ID
void (*thread_svc_handler(u32 svc_id))(SrCpuState*) {
    for (const auto& entry : kThreadSvcEntries) {
        if (entry.id == svc_id) return entry.handler;
    }
    return nullptr;
}

} // namespace

// Dispatch a thread-related SVC
// Returns true if handled, false if not a thread SVC
bool dispatch_thread_svc(SrCpuState* cpu, u32 svc_id) {
    auto handler = thread_svc_handler(svc_id);
    if (handler) {
        handler(cpu);
        return true;
    }
    return false;
}

} // namespace kr