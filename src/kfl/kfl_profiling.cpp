// KFL Performance Profiling Implementation

#include "kirbyrecomp/kfl/kfl_profiling.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>

namespace kirbyrecomp::kfl {

// -----------------------------------------------------------------------------
// Profiler Implementation
// -----------------------------------------------------------------------------

Profiler::Profiler()
    : config_{}, last_error_(""), session_start_time_(0), session_end_time_(0),
      current_frame_(0), frame_start_time_(0), target_fps_(60.0f),
      max_events_(100000), max_frame_history_(600),
      gpu_timer_start_(0), gpu_time_accumulator_(0), gpu_timer_active_(false),
      current_memory_usage_(0), peak_memory_usage_(0),
      frame_start_ns_(0), target_frame_time_ns_(16666667) {
}

Profiler::~Profiler() {
    shutdown();
}

bool Profiler::initialize(const ProfilerConfig& config) {
    config_ = config;
    max_events_ = config.max_events;
    if (config.target_fps > 0) {
        target_fps_ = config.target_fps;
        target_frame_time_ns_ = static_cast<uint64_t>(1e9 / target_fps_);
    }
    last_error_.clear();
    return true;
}

void Profiler::shutdown() {
    export_json(config_.output_file);
    config_.enabled = false;
}

// -----------------------------------------------------------------------------
// Frame Management
// -----------------------------------------------------------------------------

void Profiler::begin_frame(uint64_t frame_number) {
    if (!config_.enabled) return;

    current_frame_ = frame_number;
    frame_start_time_ = get_timestamp_ns();
    frame_start_ns_ = frame_start_time_;

    current_frame_stats_ = FrameStats{};
    current_frame_stats_.frame_number = frame_number;

    session_stats_.total_frames++;
}

void Profiler::end_frame() {
    if (!config_.enabled) return;

    uint64_t frame_end_time = get_timestamp_ns();
    uint64_t frame_time_ns = frame_end_time - frame_start_time_;

    current_frame_stats_.frame_number = current_frame_;
    current_frame_stats_.frame_time_ns = frame_time_ns;
    current_frame_stats_.frame_time_ms = static_cast<float>(frame_time_ns) / 1e6f;
    current_frame_stats_.fps = 1e9f / static_cast<float>(frame_time_ns);

    // Update session stats
    session_stats_.total_frames++;
    session_stats_.total_time_ns += frame_time_ns;
    session_stats_.total_time_ns = frame_end_time - session_start_time_;
    session_stats_.total_frames = current_frame_;

    if (current_frame_ > 0) {
        session_stats_.average_fps = 1e9f / (static_cast<float>(session_stats_.total_time_ns) / session_stats_.total_frames);
    }

    // Store frame stats
    previous_frame_stats_ = current_frame_stats_;
    frame_history_.push_back(current_frame_stats_);

    // Limit frame history
    if (frame_history_.size() > max_frame_history_) {
        frame_history_.erase(frame_history_.begin());
    }

    // Update peak memory
    {
        std::lock_guard<std::mutex> lock(memory_mutex_);
        if (current_memory_usage_ > peak_memory_usage_) {
            peak_memory_usage_ = current_memory_usage_;
        }
    }

    // Frame rate limiting
    limit_frame_rate();

    current_frame_++;
}

// -----------------------------------------------------------------------------
// Event Tracking
// -----------------------------------------------------------------------------

void Profiler::begin_event(std::string_view name, ProfileCategory category) {
    if (!config_.enabled) return;

    ActiveEvent event;
    event.name = name;
    event.category = category;
    event.start_time = get_timestamp_ns();
    event.thread_id = get_thread_id();

    {
        std::lock_guard<std::mutex> lock(active_events_mutex_);
        active_events_.push_back(std::move(event));
    }
}

void Profiler::end_event(std::string_view name) {
    if (!config_.enabled) return;

    uint64_t end_time = get_timestamp_ns();

    ActiveEvent* found_event = nullptr;
    {
        std::lock_guard<std::mutex> lock(active_events_mutex_);
        for (auto it = active_events_.rbegin(); it != active_events_.rend(); ++it) {
            if (it->name == name && it->thread_id == get_thread_id()) {
                found_event = &(*it);
                break;
            }
        }
    }

    if (!found_event) return;

    uint64_t duration_ns = get_timestamp_ns() - found_event->start_time;

    ProfileEvent event;
    event.timestamp = found_event->start_time;
    event.category = found_event->category;
    event.name = found_event->name;
    event.duration_ns = duration_ns;
    event.thread_id = found_event->thread_id;
    event.frame_number = current_frame_;
    event.color = static_cast<uint32_t>(found_event->category) * 0x1000000 | 0xFF000000;

    {
        std::lock_guard<std::mutex> lock(events_mutex_);
        events_.push_back(event);
        if (events_.size() > max_events_) {
            events_.erase(events_.begin());
        }
    }

    // Remove from active events
    {
        std::lock_guard<std::mutex> lock(active_events_mutex_);
        for (auto it = active_events_.rbegin(); it != active_events_.rend(); ++it) {
            if (it->name == name && it->thread_id == get_thread_id()) {
                active_events_.erase((it + 1).base());
                break;
            }
        }
    }

    // Update current frame stats
    current_frame_stats_.cpu_time_ns += duration_ns;
}

Profiler::ScopedEvent::ScopedEvent(Profiler& profiler, std::string_view name, ProfileCategory category)
    : profiler_(profiler), name_(name), category_(category) {
    profiler_.begin_event(name, category);
}

Profiler::ScopedEvent::~ScopedEvent() {
    profiler_.end_event(name_);
}

Profiler::ScopedEvent Profiler::scope(std::string_view name, ProfileCategory category) {
    return ScopedEvent(*this, name, category);
}

// -----------------------------------------------------------------------------
// Frame Statistics
// -----------------------------------------------------------------------------

void Profiler::record_frame_stats(const FrameStats& stats) {
    current_frame_stats_ = stats;
    current_frame_stats_.frame_number = current_frame_;
}

const FrameStats& Profiler::get_current_frame_stats() const {
    return current_frame_stats_;
}

const FrameStats& Profiler::get_previous_frame_stats() const {
    return previous_frame_stats_;
}

FrameStats Profiler::get_average_frame_stats(size_t count_param) const {
    FrameStats avg{};
    if (frame_history_.empty()) return avg;

    size_t count = std::min(count_param, frame_history_.size());
    if (count == 0) return avg;

    for (size_t i = frame_history_.size() - count; i < frame_history_.size(); ++i) {
        const auto& frame = frame_history_[i];
        avg.frame_time_ns += frame.frame_time_ns;
        avg.cpu_time_ns += frame.cpu_time_ns;
        avg.gpu_time_ns += frame.gpu_time_ns;
        avg.draw_calls += frame.draw_calls;
        avg.triangles_rendered += frame.triangles_rendered;
        avg.memory_allocated += frame.memory_allocated;
        avg.memory_freed += frame.memory_freed;
        avg.audio_buffers_queued += frame.audio_buffers_queued;
        avg.audio_underruns += frame.audio_underruns;
        avg.fps += frame.fps;
        avg.frame_time_ms += frame.frame_time_ms;
    }

    float count_f = static_cast<float>(count);
    avg.frame_time_ns /= count;
    avg.cpu_time_ns /= count;
    avg.gpu_time_ns /= count;
    avg.draw_calls = static_cast<uint32_t>(avg.draw_calls / count);
    avg.triangles_rendered /= count;
    avg.memory_allocated /= count;
    avg.memory_freed /= count;
    avg.audio_buffers_queued = static_cast<uint32_t>(avg.audio_buffers_queued / count);
    avg.audio_underruns = static_cast<uint32_t>(avg.audio_underruns / count);
    avg.fps /= count_f;
    avg.frame_time_ms /= count_f;
    avg.frame_number = current_frame_;

    return avg;
}

// -----------------------------------------------------------------------------
// Session Statistics
// -----------------------------------------------------------------------------

SessionStats Profiler::get_session_stats() const {
    return session_stats_;
}

void Profiler::reset_session_stats() {
    session_stats_ = SessionStats{};
    session_start_time_ = get_timestamp_ns();
    current_frame_ = 0;
    frame_history_.clear();
    {
        std::lock_guard<std::mutex> lock(events_mutex_);
        events_.clear();
    }
    {
        std::lock_guard<std::mutex> lock(memory_mutex_);
        current_memory_usage_ = 0;
        peak_memory_usage_ = 0;
    }
}

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

void Profiler::set_config(const ProfilerConfig& config) {
    config_ = config;
    max_events_ = config.max_events;
    if (config.target_fps > 0) {
        target_fps_ = config.target_fps;
        target_frame_time_ns_ = static_cast<uint64_t>(1e9 / target_fps_);
    }
}

// -----------------------------------------------------------------------------
// Export
// -----------------------------------------------------------------------------

bool Profiler::export_json(const std::string& filename) const {
    if (filename.empty()) {
        last_error_ = "No output filename specified";
        return false;
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        last_error_ = "Failed to open file for writing: " + filename;
        return false;
    }

    file << "{\n";
    file << "  \"session\": {\n";
    file << "    \"total_frames\": " << session_stats_.total_frames << ",\n";
    file << "    \"total_time_ns\": " << session_stats_.total_time_ns << ",\n";
    file << "    \"average_fps\": " << session_stats_.average_fps << ",\n";
    file << "    \"min_fps\": " << session_stats_.min_fps << ",\n";
    file << "    \"max_fps\": " << session_stats_.max_fps << ",\n";
    file << "    \"avg_frame_time_ms\": " << session_stats_.avg_frame_time_ms << ",\n";
    file << "    \"avg_cpu_time_ms\": " << session_stats_.avg_cpu_time_ms << ",\n";
    file << "    \"avg_gpu_time_ms\": " << session_stats_.avg_gpu_time_ms << ",\n";
    file << "    \"total_draw_calls\": " << session_stats_.total_draw_calls << ",\n";
    file << "    \"total_triangles\": " << session_stats_.total_triangles << ",\n";
    file << "    \"peak_memory_usage\": " << session_stats_.peak_memory_usage << ",\n";
    file << "    \"total_audio_underruns\": " << session_stats_.total_audio_underruns << "\n";
    file << "  },\n";

    file << "  \"frames\": [\n";
    for (size_t i = 0; i < frame_history_.size(); ++i) {
        const auto& frame = frame_history_[i];
        file << "    {\n";
        file << "      \"frame_number\": " << frame.frame_number << ",\n";
        file << "      \"frame_time_ns\": " << frame.frame_time_ns << ",\n";
        file << "      \"cpu_time_ns\": " << frame.cpu_time_ns << ",\n";
        file << "      \"gpu_time_ns\": " << frame.gpu_time_ns << ",\n";
        file << "      \"draw_calls\": " << frame.draw_calls << ",\n";
        file << "      \"triangles_rendered\": " << frame.triangles_rendered << ",\n";
        file << "      \"memory_allocated\": " << frame.memory_allocated << ",\n";
        file << "      \"memory_freed\": " << frame.memory_freed << ",\n";
        file << "      \"audio_buffers_queued\": " << frame.audio_buffers_queued << ",\n";
        file << "      \"audio_underruns\": " << frame.audio_underruns << ",\n";
        file << "      \"fps\": " << frame.fps << ",\n";
        file << "      \"frame_time_ms\": " << frame.frame_time_ms << "\n";
        file << "    }";
        if (i + 1 < frame_history_.size()) file << ",";
        file << "\n";
    }
    file << "  ],\n";

    file << "  \"events\": [\n";
    {
        std::lock_guard<std::mutex> lock(events_mutex_);
        for (size_t i = 0; i < events_.size(); ++i) {
            const auto& event = events_[i];
            file << "    {\n";
            file << "      \"timestamp\": " << event.timestamp << ",\n";
            file << "      \"category\": " << static_cast<int>(event.category) << ",\n";
            file << "      \"name\": \"" << event.name << "\",\n";
            file << "      \"duration_ns\": " << event.duration_ns << ",\n";
            file << "      \"thread_id\": " << event.thread_id << ",\n";
            file << "      \"frame_number\": " << event.frame_number << ",\n";
            file << "      \"color\": " << event.color << "\n";
            file << "    }";
            if (i + 1 < events_.size()) file << ",";
            file << "\n";
        }
    }
    file << "  ]\n";
    file << "}\n";

    return true;
}

bool Profiler::export_chrome_trace(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        last_error_ = "Failed to open file for writing: " + filename;
        return false;
    }

    file << "{\n";
    file << "  \"traceEvents\": [\n";

    std::lock_guard<std::mutex> lock(events_mutex_);
    for (size_t i = 0; i < events_.size(); ++i) {
        const auto& event = events_[i];
        file << "    {\n";
        file << "      \"name\": \"" << event.name << "\",\n";
        file << "      \"cat\": \"" << static_cast<int>(event.category) << "\",\n";
        file << "      \"ph\": \"X\",\n";
        file << "      \"ts\": " << (event.timestamp / 1000) << ",\n";  // microseconds
        file << "      \"dur\": " << (event.duration_ns / 1000) << ",\n";
        file << "      \"pid\": 1,\n";
        file << "      \"tid\": " << event.thread_id << ",\n";
        file << "      \"args\": {\n";
        file << "        \"frame\": " << event.frame_number << ",\n";
        file << "        \"category\": " << static_cast<int>(event.category) << "\n";
        file << "      }\n";
        file << "    }";
        if (i + 1 < events_.size()) file << ",";
        file << "\n";
    }

    file << "  ],\n";
    file << "  \"displayTimeUnit\": \"ms\"\n";
    file << "}\n";

    return true;
}

// -----------------------------------------------------------------------------
// GPU Timing
// -----------------------------------------------------------------------------

void Profiler::begin_gpu_timer(const char* name) {
    if (!config_.enabled || !config_.track_gpu) return;

    gpu_timer_start_ = get_timestamp_ns();
    gpu_timer_active_ = true;
    begin_event("GPU: " + std::string(name), ProfileCategory::GPU);
}

void Profiler::end_gpu_timer() {
    if (!config_.enabled || !config_.track_gpu || !gpu_timer_active_) return;

    uint64_t end_time = get_timestamp_ns();
    uint64_t gpu_time = end_time - gpu_timer_start_;
    gpu_time_accumulator_ += gpu_time;
    gpu_timer_active_ = false;

    current_frame_stats_.gpu_time_ns += gpu_time;
    end_event("GPU Timer");
}

uint64_t Profiler::get_gpu_time_ns() const {
    return gpu_time_accumulator_;
}

// -----------------------------------------------------------------------------
// Memory Tracking
// -----------------------------------------------------------------------------

void Profiler::record_allocation(size_t size) {
    std::lock_guard<std::mutex> lock(memory_mutex_);
    current_memory_usage_ += size;
    if (current_memory_usage_ > peak_memory_usage_) {
        peak_memory_usage_ = current_memory_usage_;
    }
    current_frame_stats_.memory_allocated += size;
}

void Profiler::record_deallocation(size_t size) {
    std::lock_guard<std::mutex> lock(memory_mutex_);
    if (current_memory_usage_ >= size) {
        current_memory_usage_ -= size;
    }
    current_frame_stats_.memory_freed += size;
}

size_t Profiler::get_current_memory_usage() const {
    std::lock_guard<std::mutex> lock(memory_mutex_);
    return current_memory_usage_;
}

size_t Profiler::get_peak_memory_usage() const {
    std::lock_guard<std::mutex> lock(memory_mutex_);
    return peak_memory_usage_;
}

// -----------------------------------------------------------------------------
// Frame Rate Limiting
// -----------------------------------------------------------------------------

void Profiler::set_target_fps(float fps) {
    if (fps > 0) {
        target_fps_ = fps;
        target_frame_time_ns_ = static_cast<uint64_t>(1e9 / fps);
    }
}

void Profiler::limit_frame_rate() {
    if (target_fps_ <= 0) return;

    uint64_t elapsed = get_timestamp_ns() - frame_start_ns_;
    if (elapsed < target_frame_time_ns_) {
        uint64_t sleep_ns = target_frame_time_ns_ - elapsed;
        std::this_thread::sleep_for(std::chrono::nanoseconds(sleep_ns));
    }
}

std::vector<ProfileEvent> Profiler::get_events_since(uint64_t timestamp) const {
    std::lock_guard<std::mutex> lock(events_mutex_);
    std::vector<ProfileEvent> result;
    for (const auto& event : events_) {
        if (event.timestamp >= timestamp) {
            result.push_back(event);
        }
    }
    return result;
}

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

uint32_t Profiler::get_thread_id() {
    static thread_local uint32_t thread_id = []() {
        std::hash<std::thread::id> hasher;
        return static_cast<uint32_t>(hasher(std::this_thread::get_id()));
    }();
    return thread_id;
}

uint64_t Profiler::get_timestamp_ns() {
    using namespace std::chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

void Profiler::maybe_flush_events() {
    if (events_.size() > max_events_) {
        std::lock_guard<std::mutex> lock(events_mutex_);
        if (events_.size() > max_events_) {
            events_.erase(events_.begin(), events_.begin() + (events_.size() - max_events_));
        }
    }
}

// -----------------------------------------------------------------------------
// Global Profiler
// -----------------------------------------------------------------------------

Profiler& get_global_profiler() {
    static Profiler instance;
    return instance;
}

} // namespace kirbyrecomp::kfl