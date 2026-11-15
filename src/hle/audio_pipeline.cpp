// Audio Pipeline Implementation

#include "kirbyrecomp/hle/audio_pipeline.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <opus/opus.h>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace kirbyrecomp::hle {

// ============================================================================
// OpusDecoderWrapper Implementation
// ============================================================================

OpusDecoderWrapper::OpusDecoderWrapper() : decoder_(nullptr), error_(OPUS_OK) {
}

OpusDecoderWrapper::~OpusDecoderWrapper() {
    close();
}

bool OpusDecoderWrapper::open(uint32_t sample_rate, uint32_t channels) {
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

void OpusDecoderWrapper::close() {
    if (decoder_) {
        opus_decoder_destroy(decoder_);
        decoder_ = nullptr;
    }
}

int OpusDecoderWrapper::decode(const uint8_t* opus_data, size_t opus_size,
                               int16_t* pcm_buffer, int frame_size, bool fec) {
    if (!decoder_) return OPUS_BAD_ARG;

    int samples = opus_decode(decoder_,
                              opus_data, static_cast<int>(opus_size),
                              pcm_buffer, frame_size, fec ? 1 : 0);
    return samples;
}

int OpusDecoderWrapper::decode_float(const uint8_t* opus_data, size_t opus_size,
                                     float* pcm_buffer, int frame_size, bool fec) {
    if (!decoder_) return OPUS_BAD_ARG;

    int samples = opus_decode_float(decoder_,
                                    opus_data, static_cast<int>(opus_size),
                                    pcm_buffer, frame_size, fec ? 1 : 0);
    return samples;
}

bool OpusDecoderWrapper::set_gain(int32_t gain_q8) {
    if (!decoder_) return false;
    int error = opus_decoder_ctl(decoder_, OPUS_SET_GAIN(gain_q8));
    return error == OPUS_OK;
}

bool OpusDecoderWrapper::get_gain(int32_t& gain_q8) {
    if (!decoder_) return false;
    int gain = 0;
    int error = opus_decoder_ctl(decoder_, OPUS_GET_GAIN(&gain));
    if (error == OPUS_OK) {
        gain_q8 = gain;
        return true;
    }
    return false;
}

bool OpusDecoderWrapper::is_open() const {
    return decoder_ != nullptr;
}

uint32_t OpusDecoderWrapper::sample_rate() const {
    return sample_rate_;
}

uint32_t OpusDecoderWrapper::channels() const {
    return channels_;
}

int OpusDecoderWrapper::last_error() const {
    return error_;
}

// ============================================================================
// AudioResampler Implementation (simple linear interpolation)
// ============================================================================

bool AudioResampler::initialize(const AudioFormat& src_format, const AudioFormat& dst_format) {
    if (src_format.format != dst_format.format ||
        src_format.channels != dst_format.channels) {
        return false; // Only support same format/channel conversion for now
    }

    if (src_format.sample_rate == dst_format.sample_rate) {
        initialized_ = true;
        return true;
    }

    src_sample_rate_ = src_format.sample_rate;
    dst_sample_rate_ = dst_format.sample_rate;
    channels_ = src_format.channels;
    ratio_ = static_cast<double>(src_sample_rate_) / dst_sample_rate_;
    initialized_ = true;
    return true;
}

int AudioResampler::resample(const int16_t* src, size_t src_frames,
                             int16_t* dst, size_t max_dst_frames) {
    if (!initialized_) return -1;

    if (src_sample_rate_ == dst_sample_rate_) {
        // No resampling needed
        size_t frames_to_copy = std::min(src_frames, max_dst_frames);
        std::copy(src, src + frames_to_copy * channels_, dst);
        return static_cast<int>(frames_to_copy);
    }

    // Simple linear interpolation resampling
    double ratio = ratio_;
    size_t output_frames = 0;

    for (size_t i = 0; i < max_dst_frames && output_frames < max_dst_frames; ++i) {
        double src_pos = i * ratio_;
        size_t idx = static_cast<size_t>(src_pos);
        double frac = src_pos - idx;

        if (idx + 1 >= src_frames) {
            // At end of input, just copy last sample
            for (uint32_t c = 0; c < channels_; ++c) {
                dst[output_frames * channels_ + c] = src[(src_frames - 1) * channels_ + c];
            }
        } else {
            for (uint32_t c = 0; c < channels_; ++c) {
                int16_t s0 = src[idx * channels_ + c];
                int16_t s1 = src[(idx + 1) * channels_ + c];
                double interp = s0 + (s1 - s0) * frac;
                dst[output_frames * channels_ + c] = static_cast<int16_t>(interp);
            }
        }
        output_frames++;
    }

    return static_cast<int>(output_frames);
}

size_t AudioResampler::get_output_delay() const {
    return 0; // Simple linear interpolation has no delay
}

void AudioResampler::reset() {
    // Nothing to reset for linear interpolation
}

// Private members
uint32_t AudioResampler::src_sample_rate_ = 48000;
uint32_t AudioResampler::dst_sample_rate_ = 48000;
uint32_t AudioResampler::channels_ = 2;
double AudioResampler::ratio_ = 1.0;

// ============================================================================
// AudioPipeline::Session Implementation
// ============================================================================

AudioPipeline::Session::Session(uint64_t id, const AudioSessionConfig& config)
    : id(id), config(config), active(false) {
}

AudioPipeline::Session::~Session() {
    if (active) {
        active = false;
        if (audio_device_ != 0) {
            SDL_CloseAudioDevice(audio_device_);
            audio_device_ = 0;
        }
    }
}

// ============================================================================
// AudioPipeline Implementation
// ============================================================================

AudioPipeline::AudioPipeline()
    : next_session_id_(1), next_buffer_id_(1),
      total_buffers_played_(0), total_opus_frames_decoded_(0),
      total_bytes_written_(0), total_buffer_underruns_(0),
      devices_cached_(false) {
}

AudioPipeline::~AudioPipeline() {
    shutdown();
}

bool AudioPipeline::initialize(const AudioSessionConfig& config) {
    // Initialize SDL audio subsystem
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        return false;
    }

    // Cache audio devices
    enumerate_devices();

    return true;
}

void AudioPipeline::shutdown() {
    // Destroy all sessions
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [id, session] : sessions_) {
        session->active = false;
    }
    sessions_.clear();

    // Quit SDL audio
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

std::vector<AudioDeviceInfo> AudioPipeline::enumerate_devices() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!devices_cached_) {
        int count = SDL_GetNumAudioDevices(SDL_FALSE);
        cached_devices_.clear();
        cached_devices_.reserve(count);

        for (int i = 0; i < count; ++i) {
            const char* name = SDL_GetAudioDeviceName(i, SDL_FALSE);
            AudioDeviceInfo info;
            info.id = static_cast<uint32_t>(i);
            info.name = name ? name : "Unknown";
            info.is_default = (i == 0);
            info.is_input = false;
            info.preferred_format = AudioFormat{};
            info.supported_formats.push_back(AudioFormat{
                AudioFormat::SampleFormat::S16, 48000, 2
            });
            cached_devices_.push_back(info);
        }
        devices_cached_ = true;
    }

    return cached_devices_;
}

uint64_t AudioPipeline::create_session(const AudioSessionConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);

    uint64_t session_id = next_session_id_++;
    auto session = std::make_unique<Session>(next_session_id_++, config);
    session->id = session_id;

    // Open audio device
    SDL_AudioSpec spec{};
    spec.format = SDL_AUDIO_S16;
    spec.channels = static_cast<int>(config.output_format.channels);
    spec.freq = static_cast<int>(config.output_format.sample_rate);

    session->audio_spec_ = spec;
    size_t buffer_size = config.buffer_size > 0 ? config.buffer_size :
                        (config.output_format.sample_rate * config.output_format.channels * 2 / 10);
    session->audio_buffer_.resize(buffer_size);

    // Open audio device
    session->audio_device_ = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (session->audio_device_ == 0) {
        return 0; // Failed to open device
    }

    // Initialize Opus decoder for 48kHz stereo
    session->opus_decoder_ = std::make_unique<OpusDecoderWrapper>();
    if (!session->opus_decoder_->open(48000, 2)) {
        SDL_CloseAudioDevice(session->audio_device_);
        return 0;
    }

    // Initialize resampler
    AudioFormat src_format{AudioFormat::SampleFormat::S16, 48000, 2};
    session->resampler_ = std::make_unique<AudioResampler>();
    session->resampler_->initialize(AudioFormat{AudioFormat::SampleFormat::S16, 48000, 2}, config.output_format);

    // Start audio device
    session->running_ = true;
    session->active = true;

    // Set audio callback
    // Note: In SDL3, we'd use SDL_AudioStream with callback
    // For simplicity, we'll use the legacy approach
    SDL_ResumeAudioDevice(session->audio_device_);

    sessions_[session_id] = std::move(session);
    return session_id;
}

void AudioPipeline::destroy_session(uint64_t session_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) return;

    it->second->active = false;
    if (it->second->audio_device_ != 0) {
        SDL_CloseAudioDevice(it->second->audio_device_);
    }
    sessions_.erase(it);
}

uint64_t AudioPipeline::submit_audio(uint64_t session_id, const void* data, size_t size,
                                     const AudioFormat& format) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = sessions_.find(session_id);
    if (it == sessions_.end() || !it->second->active) return 0;

    Session& session = it->second;
    std::lock_guard<std::mutex> lock(session.buffer_mutex_);
    std::lock_guard<std::mutex> entries_lock(session.buffer_entries_mutex_);

    const uint8_t* src = static_cast<const uint8_t*>(data);
    size_t current_offset = session.audio_buffer_.size();

    // Resize buffer if needed
    if (session.audio_buffer_.size() + size > session.audio_buffer_.capacity()) {
        session.audio_buffer_.reserve(session.audio_buffer_.size() + size);
    }

    session.audio_buffer_.insert(session.audio_buffer_.end(),
                                static_cast<const uint8_t*>(data),
                                static_cast<const uint8_t*>(data) + size);

    // Create buffer entry
    Session::BufferEntry entry;
    entry.id = session.next_buffer_id_++;
    entry.offset = current_offset;
    entry.size = size;
    entry.event_handle = 0;
    entry.event_registered = false;
    session.buffer_entries_.push_back(entry);

    // Update global stats
    total_bytes_written_ += size;

    return session.buffer_entries_.back().id;
}

uint64_t AudioPipeline::submit_opus(uint64_t session_id, const uint8_t* opus_data, size_t opus_size) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = sessions_.find(session_id);
    if (it == sessions_.end() || !it->second->active) return 0;

    Session& session = it->second;
    if (!session.opus_decoder_ || !session->opus_decoder_->is_open()) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(it->second.buffer_mutex_);
    std::lock_guard<std::mutex> entries_lock(it->second.buffer_entries_mutex_);

    // Decode Opus to PCM
    const int frame_size = 960; // 20ms at 48kHz
    std::vector<int16_t> pcm_buffer(frame_size * 2); // stereo

    int samples = session.opus_decoder_->decode(opus_data, opus_size,
                                                it->second.pcm_buffer_.data(), frame_size);
    if (samples <= 0) {
        return 0;
    }

    total_opus_frames_decoded_ += samples;

    // Convert to bytes
    size_t pcm_size = samples * 2 * 2; // samples * channels * 2 bytes per sample

    // Resample if needed
    if (it->second.resampler_ && it->second.resampler_->is_initialized()) {
        // Would resample here
    }

    size_t current_offset = it->second.audio_buffer_.size();
    it->second.audio_buffer_.insert(it->second.audio_buffer_.end(),
                                    reinterpret_cast<const uint8_t*>(it->second.pcm_buffer_.data()),
                                    reinterpret_cast<const uint8_t*>(it->second.pcm_buffer_.data()) + pcm_size);

    // Create buffer entry
    Session::BufferEntry entry;
    entry.id = it->second.next_buffer_id_++;
    entry.offset = current_offset;
    entry.size = pcm_size;
    entry.event_handle = 0;
    entry.event_registered = false;
    it->second.buffer_entries_.push_back(entry);

    total_bytes_written_ += pcm_size;

    return it->second.buffer_entries_.back().id;
}

bool AudioPipeline::register_buffer_event(uint64_t buffer_id, uint64_t event_handle) {
    // Find session containing this buffer
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto& [session_id, session] : sessions_) {
        if (!session->active) continue;

        std::lock_guard<std::mutex> entries_lock(session->buffer_entries_mutex_);

        auto it = std::find_if(session->buffer_entries_.begin(), session->buffer_entries_.end(),
                               [buffer_id](const auto& e) { return e.id == buffer_id; });

        if (it != session->buffer_entries_.end()) {
            it->event_handle = event_handle;
            it->event_registered = true;
            return true;
        }
    }
    return false;
}

uint64_t AudioPipeline::get_released_buffer(uint64_t session_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = sessions_.find(session_id);
    if (it == sessions_.end() || !it->second->active) return 0;

    Session& session = it->second;
    std::lock_guard<std::mutex> entries_lock(session.buffer_entries_mutex_);

    // Check for fully played buffers with registered events
    for (auto it = session.buffer_entries_.begin(); it != session.buffer_entries_.end(); ++it) {
        if (it->event_registered && session.buffer_offset_ >= it->offset + it->size) {
            uint64_t buffer_id = it->id;

            // Remove from tracking
            session.buffer_entries_.erase(it);
            total_buffers_played_++;

            return buffer_id;
        }
    }

    return 0;
}

void AudioPipeline::set_master_volume(float volume) {
    std::lock_guard<std::mutex> lock(mutex_);
    volume = std::clamp(volume, 0.0f, 1.0f);

    for (auto& [id, session] : sessions_) {
        if (session->active && session->audio_device_ != 0) {
            SDL_SetAudioDeviceGain(session->audio_device_, volume);
        }
    }
}

float AudioPipeline::get_master_volume() const {
    // Return volume of first active session
    for (const auto& [id, session] : sessions_) {
        if (session->active) return 1.0f; // Would track actual volume
    }
    return 1.0f;
}

AudioPipeline::Stats AudioPipeline::get_stats(uint64_t session_id) const {
    Stats stats;
    stats.buffers_queued = 0;
    stats.buffers_played = total_buffers_played_;
    stats.opus_frames_decoded = total_opus_frames_decoded_;
    stats.bytes_written = total_bytes_written_;
    stats.buffer_underruns = total_buffer_underruns_;

    if (session_id != 0) {
        auto it = sessions_.find(session_id);
        if (it != sessions_.end()) {
            stats.buffers_queued = it->second->buffer_entries_.size();
        }
    }
    return stats;
}

// Static SDL audio callback
void AudioPipeline::sdl_audio_callback(void* userdata, SDL_AudioStream* stream,
                                       int additional_amount, int total_amount) {
    // This would be set up per-session
    // For now, the callback is set per-session in Session
}

// ============================================================================
// Session Audio Callback
// ============================================================================

void AudioPipeline::Session::audio_callback(void* userdata, SDL_AudioStream* stream,
                                            int additional_amount, int total_amount) {
    auto* self = static_cast<Session*>(userdata);
    if (!self || !self->running_) return;

    std::lock_guard<std::mutex> buffer_lock(self->buffer_mutex_);
    std::lock_guard<std::mutex> entries_lock(self->buffer_entries_mutex_);

    if (self->buffer_offset_ < self->audio_buffer_.size()) {
        size_t to_copy = std::min<size_t>(additional_amount, self->audio_buffer_.size() - self->buffer_offset_);
        SDL_PutAudioStreamData(stream, self->audio_buffer_.data() + self->buffer_offset_, static_cast<int>(to_copy));

        size_t copied = to_copy;
        self->buffer_offset_ += copied;

        // Check if any buffer entries have been fully played
        for (auto it = self->buffer_entries_.begin(); it != self->buffer_entries_.end(); ) {
            if (it->event_registered && self->buffer_offset_ >= it->offset + it->size) {
                // Buffer fully played - signal the event
                it->event_registered = false;
                it->event_handle = 0;
                it = self->buffer_entries_.erase(it);
            } else {
                ++it;
            }
        }
    }
}

} // namespace kirbyrecomp::hle