#pragma once

// Audio Pipeline Integration
// Connects audren, audout, and hwopus into a unified audio pipeline

#include "kirbyrecomp/hle/service.h"
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <opus/opus.h>

namespace kirbyrecomp::hle {

// ============================================================================
// Audio Pipeline Types
// ============================================================================

// Audio format specification
struct AudioFormat {
    enum class SampleFormat : uint32_t {
        S16 = 0,
        F32 = 1,
        S32 = 2,
    };

    SampleFormat format = SampleFormat::S16;
    uint32_t sample_rate = 48000;
    uint32_t channels = 2;
    uint32_t frame_size = 960;  // Opus frame size at 48kHz

    size_t bytes_per_frame() const {
        size_t bytes_per_sample = 0;
        switch (format) {
            case SampleFormat::S16: bytes_per_sample = 2; break;
            case SampleFormat::F32: bytes_per_sample = 4; break;
            case SampleFormat::S32: bytes_per_sample = 4; break;
        }
        return bytes_per_sample * channels;
    }

    size_t bytes_per_second() const {
        return sample_rate * bytes_per_frame();
    }
};

// Audio buffer with metadata
struct AudioBuffer {
    uint64_t id = 0;
    std::vector<uint8_t> data;
    AudioFormat format;
    uint64_t timestamp = 0;  // Presentation timestamp in microseconds
    uint32_t sequence = 0;   // Sequence number for ordering
};

// Audio session configuration
struct AudioSessionConfig {
    AudioFormat output_format;
    uint32_t buffer_count = 3;
    size_t buffer_size = 48000 * 2 * 2 * 2 / 10;  // ~200ms at 48kHz stereo S16
    bool enable_resampling = true;
    float master_volume = 1.0f;
};

// Audio device info
struct AudioDeviceInfo {
    uint32_t id = 0;
    std::string name;
    bool is_default = false;
    bool is_input = false;
    AudioFormat preferred_format;
    std::vector<AudioFormat> supported_formats;
};

// ============================================================================
// Opus Decoder Wrapper
// ============================================================================

class OpusDecoderWrapper {
public:
    OpusDecoderWrapper() : decoder_(nullptr) {}
    ~OpusDecoderWrapper() { close(); }

    bool open(uint32_t sample_rate, uint32_t channels) {
        if (decoder_) return false;

        int error = 0;
        decoder_ = opus_decoder_create(sample_rate, channels, &error_);
        if (error_ != OPUS_OK || !decoder_) {
            error_ = error_;
            return false;
        }
        sample_rate_ = sample_rate;
        channels_ = channels;
        return true;
    }

    void close() {
        if (decoder_) {
            opus_decoder_destroy(decoder_);
            decoder_ = nullptr;
        }
    }

    // Decode Opus data to PCM (interleaved)
    // Returns number of samples decoded per channel, or negative error code
    int decode(const uint8_t* opus_data, size_t opus_size,
               int16_t* pcm_buffer, int frame_size, bool fec = false) {
        if (!decoder_) return OPUS_BAD_ARG;

        int samples = opus_decode(decoder_,
                                  opus_data, static_cast<int>(opus_size),
                                  pcm_buffer, frame_size, 0);
        return samples;
    }

    // Decode to float output
    int decode_float(const uint8_t* opus_data, size_t opus_size,
                     float* pcm_buffer, int frame_size, bool fec = false) {
        if (!decoder_) return OPUS_BAD_ARG;

        int samples = opus_decode_float(decoder_,
                                        opus_data, static_cast<int>(opus_size),
                                        pcm_buffer, frame_size, 0);
        return samples;
    }

    // Set decoder gain (Q8 format)
    bool set_gain(int32_t gain_q8) {
        if (!decoder_) return false;
        int error = opus_decoder_ctl(decoder_, OPUS_SET_GAIN(gain_q8));
        return error == OPUS_OK;
    }

    // Get decoder gain (Q8 format)
    bool get_gain(int32_t& gain_q8) {
        if (!decoder_) return false;
        int gain = 0;
        int error = opus_decoder_ctl(decoder_, OPUS_GET_GAIN(&gain));
        if (error == OPUS_OK) {
            gain_q8 = gain;
            return true;
        }
        return false;
    }

    bool is_open() const { return decoder_ != nullptr; }
    uint32_t sample_rate() const { return sample_rate_; }
    uint32_t channels() const { return channels_; }
    int last_error() const { return error_; }

private:
    OpusDecoder* decoder_ = nullptr;
    uint32_t sample_rate_ = 48000;
    uint32_t channels_ = 2;
    int error_ = OPUS_OK;
};

// ============================================================================
// Audio Resampler
// ============================================================================

class AudioResampler {
public:
    AudioResampler() = default;
    ~AudioResampler() = default;

    // Initialize resampler
    // Returns false if format conversion not supported
    bool initialize(const AudioFormat& src_format, const AudioFormat& dst_format);

    // Resample audio buffer
    // Returns number of output frames, or negative error
    int resample(const int16_t* src, size_t src_frames,
                 int16_t* dst, size_t max_dst_frames);

    // Get output delay in frames
    size_t get_output_delay() const;

    // Reset internal state
    void reset();

    bool is_initialized() const { return initialized_; }

private:
    bool initialized_ = false;
    // Would use a proper resampling library like libsamplerate or soxr
    // For now, implement simple linear interpolation
};

// ============================================================================
// Audio Pipeline Manager
// ============================================================================

class AudioPipeline {
public:
    AudioPipeline();
    ~AudioPipeline();

    // Initialize the audio pipeline
    bool initialize(const AudioSessionConfig& config);

    // Shutdown and cleanup
    void shutdown();

    // Get available audio devices
    std::vector<AudioDeviceInfo> enumerate_devices();

    // Create audio session
    uint64_t create_session(const AudioSessionConfig& config = {});

    // Destroy audio session
    void destroy_session(uint64_t session_id);

    // Submit audio data for playback (PCM)
    // Returns buffer ID for event tracking
    uint64_t submit_audio(uint64_t session_id, const void* data, size_t size,
                          const AudioFormat& format = {});

    // Submit Opus data for decoding and playback
    // Returns buffer ID for event tracking
    uint64_t submit_opus(uint64_t session_id, const uint8_t* opus_data, size_t opus_size);

    // Register event for buffer completion
    bool register_buffer_event(uint64_t buffer_id, uint64_t event_handle);

    // Get released buffer (buffer that has finished playing)
    // Returns buffer ID or 0 if none
    uint64_t get_released_buffer(uint64_t session_id);

    // Volume control (0.0 to 1.0)
    void set_master_volume(float volume);
    float get_master_volume() const;

    // Get pipeline statistics
    struct Stats {
        size_t buffers_queued = 0;
        size_t buffers_played = 0;
        size_t opus_frames_decoded = 0;
        size_t bytes_written = 0;
        size_t buffer_underruns = 0;
    };
    Stats get_stats(uint64_t session_id) const;

private:
    struct Session {
        uint64_t id;
        AudioSessionConfig config;
        bool active = false;

        // Audio renderer (audren)
        SDL_AudioDeviceID audio_device_ = 0;
        SDL_AudioSpec audio_spec_;
        std::vector<uint8_t> audio_buffer_;
        size_t buffer_offset_ = 0;
        std::mutex buffer_mutex_;
        bool running_ = false;

        // Opus decoder
        std::unique_ptr<OpusDecoderWrapper> opus_decoder_;
        std::unique_ptr<AudioResampler> resampler_;

        // Buffer tracking for event system
        struct BufferEntry {
            uint64_t id;
            size_t offset;
            size_t size;
            uint64_t event_handle;
            bool event_registered = false;
        };
        std::vector<BufferEntry> buffer_entries_;
        uint64_t next_buffer_id_ = 1;
        std::mutex buffer_entries_mutex_;
        std::unordered_map<uint64_t, uint64_t> event_to_buffer_;

        // Audio callback
        static void audio_callback(void* userdata, SDL_AudioStream* stream,
                                   int additional_amount, int total_amount);
    };

    std::mutex mutex_;
    uint64_t next_session_id_ = 1;
    std::unordered_map<uint64_t, std::unique_ptr<Session>> sessions_;
    uint64_t next_buffer_id_ = 1;

    // Global stats
    size_t total_buffers_played_ = 0;
    size_t total_opus_frames_decoded_ = 0;
    size_t total_bytes_written_ = 0;
    size_t total_buffer_underruns_ = 0;

    // Audio device enumeration
    std::vector<AudioDeviceInfo> cached_devices_;
    bool devices_cached_ = false;

    // SDL audio callback static wrapper
    static void sdl_audio_callback(void* userdata, SDL_AudioStream* stream,
                                   int additional_amount, int total_amount);
};

} // namespace kirbyrecomp::hle