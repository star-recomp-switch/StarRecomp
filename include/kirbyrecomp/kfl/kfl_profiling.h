#pragma once

// KFL Performance Profiling System
// Provides frame timing, GPU/CPU profiling, and performance metrics

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <array>
#include <string_view>

namespace kirbyrecomp::kfl {

// -----------------------------------------------------------------------------
// Profiling Types
// -----------------------------------------------------------------------------

enum class ProfileCategory : uint8_t {
    Frame = 0,           // Frame timing
    CPU = 1,             // CPU work
    GPU = 2,             // GPU work
    Audio = 3,           // Audio processing
    Input = 4,           // Input handling
    Physics = 5,         // Physics simulation
    Render = 6,          // Rendering
    Memory = 7,          // Memory operations
    IO = 8,              // I/O operations
    Custom = 9,          // Custom categories
    Count = 10
};

// Profile event
struct ProfileEvent {
    uint64_t timestamp;           // Timestamp in nanoseconds
    ProfileCategory category;     // Category
    std::string name;             // Event name
    uint64_t duration_ns;         // Duration in nanoseconds
    uint32_t thread_id;           // Thread ID
    uint64_t frame_number;        // Frame number
    uint32_t color;               // Color for visualization (ARGB)
};

// Frame statistics
struct FrameStats {
    uint64_t frame_number;
    uint64_t frame_time_ns;           // Total frame time
    uint64_t cpu_time_ns;             // CPU time
    uint64_t gpu_time_ns;             // GPU time
    uint32_t draw_calls;              // Number of draw calls
    uint32_t triangles_rendered;      // Triangles rendered
    size_t memory_allocated;          // Memory allocated this frame
    size_t memory_freed;              // Memory freed this frame
    uint32_t audio_buffers_queued;    // Audio buffers queued
    uint32_t audio_underruns;         // Audio underruns
    float fps;                        // Current FPS
    float frame_time_ms;              // Frame time in milliseconds
};

// Profile session statistics
struct SessionStats {
    uint64_t session_start_time;
    uint64_t session_end_time;
    uint64_t total_frames;
    uint64_t total_time_ns;
    float average_fps;
    float min_fps;
    float max_fps;
    float avg_frame_time_ms;
    float avg_cpu_time_ms;
    float avg_gpu_time_ms;
    size_t total_draw_calls;
    uint64_t total_triangles;
    size_t peak_memory_usage;
    size_t total_audio_underruns;
};

// Profiling configuration
struct ProfilerConfig {
    bool enabled = true;
    bool track_gpu = true;
    bool track_cpu = true;
    bool track_memory = true;
    bool track_audio = true;
    size_t max_events = 100000;       // Max events to store
    bool auto_flush = true;           // Auto-flush when full
    std::string output_file;          // Output file for profiling data
    bool output_json = true;          // Output JSON format
    float target_fps = 60.0f;         // Target frame rate for frame limiting
};

// -----------------------------------------------------------------------------
// Profiler
// -----------------------------------------------------------------------------

class Profiler {
public:
    Profiler();
    ~Profiler();

    // Initialize profiler
    bool initialize(const ProfilerConfig& config = {});

    // Shutdown profiler
    void shutdown();

    // Frame management
    void begin_frame(uint64_t frame_number);
    void end_frame();

    // Event tracking
    void begin_event(std::string_view name, ProfileCategory category = ProfileCategory::Custom);
    void end_event(std::string_view name);

    // Scoped event helper
    class ScopedEvent {
    public:
        ScopedEvent(Profiler& profiler, std::string_view name, ProfileCategory category);
        ~ScopedEvent();
    private:
        Profiler& profiler_;
        std::string name_;
        ProfileCategory category_;
    };

    // Scoped event factory
    ScopedEvent scope(std::string_view name, ProfileCategory category = ProfileCategory::Custom);

    // Frame statistics
    void record_frame_stats(const FrameStats& stats);
    const FrameStats& get_current_frame_stats() const;
    const FrameStats& get_previous_frame_stats() const;

    // Get current frame number
    uint64_t get_current_frame() const { return current_frame_; }

    // Get events
    const std::vector<ProfileEvent>& get_events() const { return events_; }
    std::vector<ProfileEvent> get_events_since(uint64_t timestamp) const;

    // Get frame statistics
    const std::vector<FrameStats>& get_frame_history() const { return frame_history_; }
    FrameStats get_average_frame_stats(size_t count = 60) const;

    // Session statistics
    SessionStats get_session_stats() const;
    void reset_session_stats();

    // Configuration
    void set_config(const ProfilerConfig& config);
    const ProfilerConfig& get_config() const { return config_; }

    // Export profiling data
    bool export_json(const std::string& filename) const;
    bool export_chrome_trace(const std::string& filename) const;  // Chrome trace format

    // Get last error
    std::string get_last_error() const { return last_error_; }

    // Get profiler status
    bool is_enabled() const { return config_.enabled; }
    void set_enabled(bool enabled) { config_.enabled = enabled; }

    // GPU timing (if available)
    void begin_gpu_timer(const char* name);
    void end_gpu_timer();
    uint64_t get_gpu_time_ns() const;

    // Memory tracking
    void record_allocation(size_t size);
    void record_deallocation(size_t size);
    size_t get_current_memory_usage() const;
    size_t get_peak_memory_usage() const;

    // Frame rate limiting
    void set_target_fps(float fps);
    float get_target_fps() const { return target_fps_; }
    void limit_frame_rate();

private:
    ProfilerConfig config_;
    mutable std::string last_error_;

    // Session state
    uint64_t session_start_time_ = 0;
    uint64_t session_end_time_ = 0;
    uint64_t current_frame_ = 0;
    uint64_t frame_start_time_ = 0;
    float target_fps_ = 60.0f;

    // Event storage
    std::vector<ProfileEvent> events_;
    mutable std::mutex events_mutex_;
    size_t max_events_ = 100000;

    // Frame history
    std::vector<FrameStats> frame_history_;
    size_t max_frame_history_ = 600;  // 10 minutes at 60fps
    FrameStats current_frame_stats_;
    FrameStats previous_frame_stats_;

    // Session statistics
    SessionStats session_stats_;

    // Active events stack
    struct ActiveEvent {
        std::string name;
        ProfileCategory category;
        uint64_t start_time;
        uint32_t thread_id;
    };
    std::vector<ActiveEvent> active_events_;
    mutable std::mutex active_events_mutex_;

    // GPU timing
    uint64_t gpu_timer_start_ = 0;
    uint64_t gpu_time_accumulator_ = 0;
    bool gpu_timer_active_ = false;

    // Memory tracking
    size_t current_memory_usage_ = 0;
    size_t peak_memory_usage_ = 0;
    mutable std::mutex memory_mutex_;

    // Frame rate limiting
    uint64_t frame_start_ns_ = 0;
    uint64_t target_frame_time_ns_ = 16666667;  // ~60 FPS

    // Thread ID helper
    static uint32_t get_thread_id();

    // Timestamp helper
    static uint64_t get_timestamp_ns();

    // Helper: flush events if needed
    void maybe_flush_events();

    // Helper: export frame data to JSON
    std::string frame_to_json(const FrameStats& frame) const;

    // Helper: export event to JSON
    std::string event_to_json(const ProfileEvent& event) const;
};

// Global profiler instance
Profiler& get_global_profiler();

// Convenience macros
#define PROFILE_SCOPE(name, category) \
    auto _profile_scope = ::kirbyrecomp::kfl::get_global_profiler().scope(#name, ::kirbyrecomp::kfl::ProfileCategory::category)

#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCTION__, CPU)

#define PROFILE_GPU_SCOPE(name) \
    do { \
        auto& profiler = ::kirbyrecomp::kfl::get_global_profiler(); \
        profiler.begin_gpu_timer(#name); \
        auto _gpu_scope_guard = [&]() { profiler.end_gpu_timer(); }; \
        (void)_gpu_scope_guard; \
    } while(0)

} // namespace kirbyrecomp::kfl