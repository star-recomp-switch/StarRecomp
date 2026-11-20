#pragma once

// NVN Command Buffer Translator
// Translates parsed Maxwell FIFO commands to Vulkan command buffer recording

#include "kirbyrecomp/nvn/command_buffer.h"
#include "kirbyrecomp/nvn/resource_manager.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <optional>

namespace kirbyrecomp::nvn {

// ============================================================================
// Pipeline State Cache
// ============================================================================

struct PipelineState {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;

    // Bound resources for this pipeline
    std::vector<VkBuffer> vertex_buffers;
    std::vector<VkDeviceSize> vertex_buffer_offsets;
    VkBuffer index_buffer = VK_NULL_HANDLE;
    VkIndexType index_type = VK_INDEX_TYPE_UINT16;
    std::vector<VkDescriptorSet> descriptor_sets;

    // Dynamic state
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    float blend_constants[4] = {0, 0, 0, 0};
    uint32_t sample_mask = 0xFFFFFFFF;
    float line_width = 1.0f;
    float depth_bias_constant = 0.0f;
    float depth_bias_clamp = 0.0f;
    float depth_bias_slope = 0.0f;
    VkStencilFaceFlags stencil_face = VK_STENCIL_FACE_FRONT_BIT;
    uint32_t stencil_reference = 0;

    // Shader stages bound
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
};

// ============================================================================
// Command Buffer Translator
// ============================================================================

class CommandBufferTranslator {
public:
    CommandBufferTranslator(VkDevice device, VkCommandBuffer cmd_buffer,
                            ResourceManager& resource_manager);
    ~CommandBufferTranslator() = default;

    // Non-copyable, movable
    CommandBufferTranslator(const CommandBufferTranslator&) = delete;
    CommandBufferTranslator& operator=(const CommandBufferTranslator&) = delete;
    CommandBufferTranslator(CommandBufferTranslator&&) = default;
    CommandBufferTranslator& operator=(CommandBufferTranslator&&) = default;

    // Translate and record a parsed command
    bool translate_command(const ParsedCommand& cmd);

    // Translate a full command buffer
    bool translate_command_buffer(const std::vector<ParsedCommand>& commands);

    // Begin/end command buffer recording
    bool begin_command_buffer(VkCommandBufferUsageFlags flags = 0);
    bool end_command_buffer();

    // Set current pipeline (graphics or compute)
    void set_graphics_pipeline(uint64_t pipeline_handle);
    void set_compute_pipeline(uint64_t pipeline_handle);

    // Get current pipeline state
    const PipelineState& get_pipeline_state() const { return pipeline_state_; }

    // Get last error
    const std::string& get_last_error() const { return last_error_; }

private:
    VkDevice device_;
    VkCommandBuffer cmd_buffer_;
    ResourceManager& resource_manager_;
    std::string last_error_;

    // Current pipeline state
    PipelineState pipeline_state_;
    bool is_compute_pipeline_ = false;

    // Descriptor set update tracking
    struct DescriptorWrite {
        uint64_t set_handle;
        std::vector<VkWriteDescriptorSet> writes;
    };
    std::vector<DescriptorWrite> pending_descriptor_writes_;

    // Descriptor set management
    VkDescriptorSet get_or_create_descriptor_set(VkShaderStageFlags stage_flags);

    // ============================================================================
    // Draw Commands
    // ============================================================================
    bool translate_draw_arrays(const ParsedCommand& cmd);
    bool translate_draw_elements(const ParsedCommand& cmd);
    bool translate_draw_arrays_instanced(const ParsedCommand& cmd);
    bool translate_draw_elements_instanced(const ParsedCommand& cmd);
    bool translate_draw_transform_feedback(const ParsedCommand& cmd);
    bool translate_draw_transform_feedback_instanced(const ParsedCommand& cmd);
    bool translate_draw_auto(const ParsedCommand& cmd);
    bool translate_multi_draw_arrays(const ParsedCommand& cmd);
    bool translate_multi_draw_elements(const ParsedCommand& cmd);

    // ============================================================================
    // Compute Dispatch
    // ============================================================================
    bool translate_dispatch(const ParsedCommand& cmd);
    bool translate_dispatch_indirect(const ParsedCommand& cmd);

    // ============================================================================
    // Resource Binding
    // ============================================================================
    bool translate_bind_vertex_buffer(const ParsedCommand& cmd);
    bool translate_bind_index_buffer(const ParsedCommand& cmd);
    bool translate_bind_constant_buffer(const ParsedCommand& cmd);
    bool translate_bind_shader_resource(const ParsedCommand& cmd);
    bool translate_bind_unordered_access(const ParsedCommand& cmd);
    bool translate_bind_sampler(const ParsedCommand& cmd);
    bool translate_bind_shader(const ParsedCommand& cmd, VkShaderStageFlagBits stage);

    // ============================================================================
    // Pipeline State
    // ============================================================================
    bool translate_set_viewport(const ParsedCommand& cmd);
    bool translate_set_scissor(const ParsedCommand& cmd);
    bool translate_set_blend_state(const ParsedCommand& cmd);
    bool translate_set_depth_stencil_state(const ParsedCommand& cmd);
    bool translate_set_rasterizer_state(const ParsedCommand& cmd);
    bool translate_set_sample_mask(const ParsedCommand& cmd);
    bool translate_set_primitive_topology(const ParsedCommand& cmd);
    bool translate_set_vertex_layout(const ParsedCommand& cmd);

    // ============================================================================
    // Texture/Sampler
    // ============================================================================
    bool translate_set_texture(const ParsedCommand& cmd);
    bool translate_set_sampler(const ParsedCommand& cmd);
    bool translate_set_texture_view(const ParsedCommand& cmd);

    // ============================================================================
    // Buffer/Texture Copy
    // ============================================================================
    bool translate_copy_buffer(const ParsedCommand& cmd);
    bool translate_copy_buffer_region(const ParsedCommand& cmd);
    bool translate_copy_texture(const ParsedCommand& cmd);
    bool translate_copy_texture_region(const ParsedCommand& cmd);
    bool translate_copy_buffer_to_texture(const ParsedCommand& cmd);
    bool translate_copy_texture_to_buffer(const ParsedCommand& cmd);
    bool translate_update_buffer(const ParsedCommand& cmd);
    bool translate_update_texture(const ParsedCommand& cmd);
    bool translate_fill_buffer(const ParsedCommand& cmd);
    bool translate_clear_texture(const ParsedCommand& cmd);

    // ============================================================================
    // Synchronization
    // ============================================================================
    bool translate_barrier(const ParsedCommand& cmd);
    bool translate_event_signal(const ParsedCommand& cmd);
    bool translate_event_wait(const ParsedCommand& cmd);
    bool translate_query_begin(const ParsedCommand& cmd);
    bool translate_query_end(const ParsedCommand& cmd);
    bool translate_query_reset(const ParsedCommand& cmd);
    bool translate_wait_idle(const ParsedCommand& cmd);

    // ============================================================================
    // Memory Management
    // ============================================================================
    bool translate_allocate_memory(const ParsedCommand& cmd);
    bool translate_free_memory(const ParsedCommand& cmd);
    bool translate_map_memory(const ParsedCommand& cmd);
    bool translate_unmap_memory(const ParsedCommand& cmd);

    // ============================================================================
    // Helpers
    // ============================================================================
    void flush_descriptor_writes();
    void apply_dynamic_state();
    void apply_pipeline_state();

    // Resource lookup helpers
    VkBuffer get_buffer(uint64_t resource_id) const;
    VkImage get_image(uint64_t resource_id) const;
    VkImageView get_image_view(uint64_t resource_id) const;
    VkSampler get_sampler(uint64_t resource_id) const;
    VkPipeline get_pipeline(uint64_t pipeline_id) const;
    VkPipelineLayout get_pipeline_layout(uint64_t pipeline_id) const;
    VkDescriptorSet get_descriptor_set(uint64_t set_handle) const;
    VkDescriptorPool get_descriptor_pool(uint64_t pool_handle) const;

    // Texture format helpers
    bool is_astc_format(uint32_t format);
    uint32_t calculate_row_pitch(uint32_t format, uint32_t width);
    uint32_t get_format_block_size(TextureFormat format);
    VkImageAspectFlags get_format_aspect(VkFormat format);
};

} // namespace kirbyrecomp::nvn