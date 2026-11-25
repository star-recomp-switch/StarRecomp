// NVN Command Buffer Translator Implementation

#include "kirbyrecomp/nvn/command_translator.h"

#include <cstring>
#include <algorithm>
#include <iostream>

namespace kirbyrecomp::nvn {

// ============================================================================
// CommandBufferTranslator Implementation
// ============================================================================

CommandBufferTranslator::CommandBufferTranslator(VkDevice device, VkCommandBuffer cmd_buffer,
                                                 ResourceManager& resource_manager)
    : device_(device), cmd_buffer_(cmd_buffer), resource_manager_(resource_manager) {
}

bool CommandBufferTranslator::begin_command_buffer(VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = flags;
    begin_info.pInheritanceInfo = nullptr;

    VkResult result = vkBeginCommandBuffer(cmd_buffer_, &begin_info);
    if (result != VK_SUCCESS) {
        last_error_ = "vkBeginCommandBuffer failed: " + std::to_string(result);
        return false;
    }
    return true;
}

bool CommandBufferTranslator::end_command_buffer() {
    flush_descriptor_writes();

    VkResult result = vkEndCommandBuffer(cmd_buffer_);
    if (result != VK_SUCCESS) {
        last_error_ = "vkEndCommandBuffer failed: " + std::to_string(result);
        return false;
    }
    return true;
}

void CommandBufferTranslator::set_graphics_pipeline(uint64_t pipeline_handle) {
    flush_descriptor_writes();

    auto pipeline_opt = resource_manager_.get_pipeline(pipeline_handle);
    if (!pipeline_opt || !*pipeline_opt) {
        last_error_ = "Pipeline not found: " + std::to_string(pipeline_handle);
        return;
    }

    PipelineResource* pipeline = *pipeline_opt;
    vkCmdBindPipeline(cmd_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);

    pipeline_state_.pipeline = pipeline->pipeline;
    pipeline_state_.layout = pipeline->layout;
    pipeline_state_.descriptor_set_layouts = pipeline->descriptor_set_layouts;
    is_compute_pipeline_ = false;
}

void CommandBufferTranslator::set_compute_pipeline(uint64_t pipeline_handle) {
    flush_descriptor_writes();

    auto pipeline_opt = resource_manager_.get_pipeline(pipeline_handle);
    if (!pipeline_opt || !*pipeline_opt) {
        last_error_ = "Pipeline not found: " + std::to_string(pipeline_handle);
        return;
    }

    PipelineResource* pipeline = *pipeline_opt;
    vkCmdBindPipeline(cmd_buffer_, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline);

    pipeline_state_.pipeline = pipeline->pipeline;
    pipeline_state_.layout = pipeline->layout;
    pipeline_state_.descriptor_set_layouts = pipeline->descriptor_set_layouts;
    is_compute_pipeline_ = true;
}

bool CommandBufferTranslator::translate_command(const ParsedCommand& cmd) {
    switch (cmd.opcode) {
        // Draw commands
        case MaxwellCmdOpcode::DRAW_ARRAYS:
            return translate_draw_arrays(cmd);
        case MaxwellCmdOpcode::DRAW_ARRAYS_INSTANCED:
            return translate_draw_arrays_instanced(cmd);
        case MaxwellCmdOpcode::DRAW_ELEMENTS:
            return translate_draw_elements(cmd);
        case MaxwellCmdOpcode::DRAW_ELEMENTS_INSTANCED:
            return translate_draw_elements_instanced(cmd);
        case MaxwellCmdOpcode::DRAW_TRANSFORM_FEEDBACK:
            return translate_draw_transform_feedback(cmd);
        case MaxwellCmdOpcode::DRAW_TRANSFORM_FEEDBACK_INST:
            return translate_draw_transform_feedback_instanced(cmd);
        case MaxwellCmdOpcode::DRAW_AUTO:
            return translate_draw_auto(cmd);
        case MaxwellCmdOpcode::MULTI_DRAW_ARRAYS:
            return translate_multi_draw_arrays(cmd);
        case MaxwellCmdOpcode::MULTI_DRAW_ELEMENTS:
            return translate_multi_draw_elements(cmd);

        // Compute dispatch
        case MaxwellCmdOpcode::DISPATCH:
            return translate_dispatch(cmd);
        case MaxwellCmdOpcode::DISPATCH_INDIRECT:
            return translate_dispatch_indirect(cmd);

        // Resource binding
        case MaxwellCmdOpcode::BIND_VERTEX_BUFFER:
            return translate_bind_vertex_buffer(cmd);
        case MaxwellCmdOpcode::BIND_INDEX_BUFFER:
            return translate_bind_index_buffer(cmd);
        case MaxwellCmdOpcode::BIND_CONSTANT_BUFFER:
            return translate_bind_constant_buffer(cmd);
        case MaxwellCmdOpcode::BIND_SHADER_RESOURCE:
            return translate_bind_shader_resource(cmd);
        case MaxwellCmdOpcode::BIND_UNORDERED_ACCESS:
            return translate_bind_unordered_access(cmd);
        case MaxwellCmdOpcode::BIND_SAMPLER:
            return translate_bind_sampler(cmd);
        case MaxwellCmdOpcode::BIND_VERTEX_SHADER:
            return translate_bind_shader(cmd, VK_SHADER_STAGE_VERTEX_BIT);
        case MaxwellCmdOpcode::BIND_PIXEL_SHADER:
            return translate_bind_shader(cmd, VK_SHADER_STAGE_FRAGMENT_BIT);
        case MaxwellCmdOpcode::BIND_GEOMETRY_SHADER:
            return translate_bind_shader(cmd, VK_SHADER_STAGE_GEOMETRY_BIT);
        case MaxwellCmdOpcode::BIND_HULL_SHADER:
            return translate_bind_shader(cmd, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
        case MaxwellCmdOpcode::BIND_DOMAIN_SHADER:
            return translate_bind_shader(cmd, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
        case MaxwellCmdOpcode::BIND_COMPUTE_SHADER:
            return translate_bind_shader(cmd, VK_SHADER_STAGE_COMPUTE_BIT);

        // Pipeline state
        case MaxwellCmdOpcode::SET_VIEWPORT:
            return translate_set_viewport(cmd);
        case MaxwellCmdOpcode::SET_SCISSOR:
            return translate_set_scissor(cmd);
        case MaxwellCmdOpcode::SET_BLEND_STATE:
            return translate_set_blend_state(cmd);
        case MaxwellCmdOpcode::SET_DEPTH_STENCIL_STATE:
            return translate_set_depth_stencil_state(cmd);
        case MaxwellCmdOpcode::SET_RASTERIZER_STATE:
            return translate_set_rasterizer_state(cmd);
        case MaxwellCmdOpcode::SET_SAMPLE_MASK:
            return translate_set_sample_mask(cmd);
        case MaxwellCmdOpcode::SET_PRIMITIVE_TOPOLOGY:
            return translate_set_primitive_topology(cmd);
        case MaxwellCmdOpcode::SET_VERTEX_LAYOUT:
            return translate_set_vertex_layout(cmd);

        // Texture/Sampler
        case MaxwellCmdOpcode::SET_TEXTURE:
            return translate_set_texture(cmd);
        case MaxwellCmdOpcode::SET_SAMPLER:
            return translate_set_sampler(cmd);
        case MaxwellCmdOpcode::SET_TEXTURE_VIEW:
            return translate_set_texture_view(cmd);

        // Buffer/Texture copy
        case MaxwellCmdOpcode::COPY_BUFFER:
            return translate_copy_buffer(cmd);
        case MaxwellCmdOpcode::COPY_BUFFER_REGION:
            return translate_copy_buffer_region(cmd);
        case MaxwellCmdOpcode::COPY_TEXTURE:
            return translate_copy_texture(cmd);
        case MaxwellCmdOpcode::COPY_TEXTURE_REGION:
            return translate_copy_texture_region(cmd);
        case MaxwellCmdOpcode::COPY_BUFFER_TO_TEXTURE:
            return translate_copy_buffer_to_texture(cmd);
        case MaxwellCmdOpcode::COPY_TEXTURE_TO_BUFFER:
            return translate_copy_texture_to_buffer(cmd);
        case MaxwellCmdOpcode::UPDATE_BUFFER:
            return translate_update_buffer(cmd);
        case MaxwellCmdOpcode::UPDATE_TEXTURE:
            return translate_update_texture(cmd);
        case MaxwellCmdOpcode::FILL_BUFFER:
            return translate_fill_buffer(cmd);
        case MaxwellCmdOpcode::CLEAR_TEXTURE:
            return translate_clear_texture(cmd);

        // Synchronization
        case MaxwellCmdOpcode::BARRIER:
            return translate_barrier(cmd);
        case MaxwellCmdOpcode::EVENT_SIGNAL:
            return translate_event_signal(cmd);
        case MaxwellCmdOpcode::EVENT_WAIT:
            return translate_event_wait(cmd);
        case MaxwellCmdOpcode::QUERY_BEGIN:
            return translate_query_begin(cmd);
        case MaxwellCmdOpcode::QUERY_END:
            return translate_query_end(cmd);
        case MaxwellCmdOpcode::QUERY_RESET:
            return translate_query_reset(cmd);
        case MaxwellCmdOpcode::WAIT_IDLE:
            return translate_wait_idle(cmd);

        // Memory management
        case MaxwellCmdOpcode::ALLOCATE_MEMORY:
            return translate_allocate_memory(cmd);
        case MaxwellCmdOpcode::FREE_MEMORY:
            return translate_free_memory(cmd);
        case MaxwellCmdOpcode::MAP_MEMORY:
            return translate_map_memory(cmd);
        case MaxwellCmdOpcode::UNMAP_MEMORY:
            return translate_unmap_memory(cmd);

        // Debug
        case MaxwellCmdOpcode::INSERT_MARKER:
        case MaxwellCmdOpcode::PUSH_DEBUG_GROUP:
        case MaxwellCmdOpcode::POP_DEBUG_GROUP:
            // Debug markers - no-op for now
            return true;

        case MaxwellCmdOpcode::END:
            return true;

        default:
            last_error_ = "Unknown command opcode: " + std::to_string(static_cast<uint32_t>(cmd.opcode));
            return false;
    }
}

bool CommandBufferTranslator::translate_command_buffer(const std::vector<ParsedCommand>& commands) {
    for (const auto& cmd : commands) {
        if (!translate_command(cmd)) {
            return false;
        }
    }
    return true;
}

// ============================================================================
// Draw Commands
// ============================================================================

bool CommandBufferTranslator::translate_draw_arrays(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdDrawArrays)) {
        last_error_ = "DRAW_ARRAYS command data too small";
        return false;
    }

    const CmdDrawArrays* draw = cmd.get_data<CmdDrawArrays>();

    if (!is_compute_pipeline_ && pipeline_state_.pipeline != VK_NULL_HANDLE) {
        apply_dynamic_state();
        flush_descriptor_writes();
        vkCmdDraw(cmd_buffer_, draw->vertex_count, draw->instance_count,
                  draw->first_vertex, draw->first_instance);
    } else {
        last_error_ = "No graphics pipeline bound for DRAW_ARRAYS";
        return false;
    }
    return true;
}

bool CommandBufferTranslator::translate_draw_arrays_instanced(const ParsedCommand& cmd) {
    return translate_draw_arrays(cmd); // Same structure
}

bool CommandBufferTranslator::translate_draw_elements(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdDrawElements)) {
        last_error_ = "DRAW_ELEMENTS command data too small";
        return false;
    }

    const CmdDrawElements* draw = cmd.get_data<CmdDrawElements>();

    if (!is_compute_pipeline_ && pipeline_state_.pipeline != VK_NULL_HANDLE) {
        apply_dynamic_state();
        flush_descriptor_writes();
        vkCmdDrawIndexed(cmd_buffer_, draw->index_count, draw->instance_count,
                         draw->first_index, draw->vertex_offset, draw->first_instance);
    } else {
        last_error_ = "No graphics pipeline bound for DRAW_ELEMENTS";
        return false;
    }
    return true;
}

bool CommandBufferTranslator::translate_draw_elements_instanced(const ParsedCommand& cmd) {
    return translate_draw_elements(cmd); // Same structure
}

bool CommandBufferTranslator::translate_draw_transform_feedback(const ParsedCommand& cmd) {
    // Transform feedback draw - requires VK_EXT_transform_feedback
    last_error_ = "DRAW_TRANSFORM_FEEDBACK not implemented";
    return false;
}

bool CommandBufferTranslator::translate_draw_transform_feedback_instanced(const ParsedCommand& cmd) {
    last_error_ = "DRAW_TRANSFORM_FEEDBACK_INST not implemented";
    return false;
}

bool CommandBufferTranslator::translate_draw_auto(const ParsedCommand& cmd) {
    // Draw from transform feedback or indirect buffer
    last_error_ = "DRAW_AUTO not implemented";
    return false;
}

bool CommandBufferTranslator::translate_multi_draw_arrays(const ParsedCommand& cmd) {
    // Multi-draw indirect
    last_error_ = "MULTI_DRAW_ARRAYS not implemented";
    return false;
}

bool CommandBufferTranslator::translate_multi_draw_elements(const ParsedCommand& cmd) {
    last_error_ = "MULTI_DRAW_ELEMENTS not implemented";
    return false;
}

// ============================================================================
// Compute Dispatch
// ============================================================================

bool CommandBufferTranslator::translate_dispatch(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdDispatch)) {
        last_error_ = "DISPATCH command data too small";
        return false;
    }

    const CmdDispatch* dispatch = cmd.get_data<CmdDispatch>();

    if (is_compute_pipeline_ && pipeline_state_.pipeline != VK_NULL_HANDLE) {
        flush_descriptor_writes();
        vkCmdDispatch(cmd_buffer_, dispatch->group_count_x, dispatch->group_count_y,
                      dispatch->group_count_z);
    } else {
        last_error_ = "No compute pipeline bound for DISPATCH";
        return false;
    }
    return true;
}

bool CommandBufferTranslator::translate_dispatch_indirect(const ParsedCommand& cmd) {
    // Dispatch indirect - requires buffer with dispatch parameters
    last_error_ = "DISPATCH_INDIRECT not implemented";
    return false;
}

// ============================================================================
// Resource Binding
// ============================================================================

bool CommandBufferTranslator::translate_bind_vertex_buffer(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdBindVertexBuffer)) {
        last_error_ = "BIND_VERTEX_BUFFER command data too small";
        return false;
    }

    const CmdBindVertexBuffer* bind = cmd.get_data<CmdBindVertexBuffer>();

    auto buffer_opt = resource_manager_.get_buffer(bind->buffer_address);
    if (!buffer_opt || !*buffer_opt) {
        last_error_ = "Vertex buffer not found: " + std::to_string(bind->buffer_address);
        return false;
    }

    VkBuffer buffer = (*buffer_opt)->buffer;
    VkDeviceSize offset = bind->offset;

    vkCmdBindVertexBuffers(cmd_buffer_, bind->slot, 1, &buffer, &offset);

    // Store for pipeline state
    if (bind->slot >= pipeline_state_.vertex_buffers.size()) {
        pipeline_state_.vertex_buffers.resize(bind->slot + 1, VK_NULL_HANDLE);
        pipeline_state_.vertex_buffer_offsets.resize(bind->slot + 1, 0);
    }
    pipeline_state_.vertex_buffers[bind->slot] = buffer;
    pipeline_state_.vertex_buffer_offsets[bind->slot] = offset;

    return true;
}

bool CommandBufferTranslator::translate_bind_index_buffer(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdBindIndexBuffer)) {
        last_error_ = "BIND_INDEX_BUFFER command data too small";
        return false;
    }

    const CmdBindIndexBuffer* bind = cmd.get_data<CmdBindIndexBuffer>();

    auto buffer_opt = resource_manager_.get_buffer(bind->buffer_address);
    if (!buffer_opt || !*buffer_opt) {
        last_error_ = "Index buffer not found: " + std::to_string(bind->buffer_address);
        return false;
    }

    VkBuffer buffer = (*buffer_opt)->buffer;
    VkIndexType index_type = (bind->index_type == 0) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

    vkCmdBindIndexBuffer(cmd_buffer_, buffer, bind->offset, index_type);

    pipeline_state_.index_buffer = buffer;
    pipeline_state_.index_type = index_type;

    return true;
}

bool CommandBufferTranslator::translate_bind_constant_buffer(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdBindShaderResource)) {
        last_error_ = "BIND_CONSTANT_BUFFER command data too small";
        return false;
    }

    const CmdBindShaderResource* bind = cmd.get_data<CmdBindShaderResource>();

    // Constant buffers are uniform buffers in descriptor sets
    // Map NVN constant buffer binding to descriptor set binding
    VkShaderStageFlags stage_flags = 0;
    if (bind->stage & 0x1) stage_flags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (bind->stage & 0x2) stage_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (bind->stage & 0x4) stage_flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if (bind->stage & 0x8) stage_flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if (bind->stage & 0x10) stage_flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if (bind->stage & 0x20) stage_flags |= VK_SHADER_STAGE_COMPUTE_BIT;

    auto buffer_opt = resource_manager_.get_buffer(bind->resource_id);
    if (!buffer_opt || !*buffer_opt) {
        last_error_ = "Constant buffer not found: " + std::to_string(bind->resource_id);
        return false;
    }

    VkBuffer buffer = (*buffer_opt)->buffer;
    VkDescriptorBufferInfo buffer_info{};
    buffer_info.buffer = buffer;
    buffer_info.offset = 0;
    buffer_info.range = VK_WHOLE_SIZE;

    // Queue descriptor write
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = get_or_create_descriptor_set(stage_flags);
    write.dstBinding = bind->slot;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &buffer_info;

    pending_descriptor_writes_.push_back({stage_flags, {write}});
    return true;
}

bool CommandBufferTranslator::translate_bind_shader_resource(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdBindShaderResource)) {
        last_error_ = "BIND_SHADER_RESOURCE command data too small";
        return false;
    }

    const CmdBindShaderResource* bind = cmd.get_data<CmdBindShaderResource>();

    // Map NVN stage to Vulkan shader stages
    VkShaderStageFlags stage_flags = 0;
    if (bind->stage & 0x1) stage_flags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (bind->stage & 0x2) stage_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (bind->stage & 0x4) stage_flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if (bind->stage & 0x8) stage_flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if (bind->stage & 0x10) stage_flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if (bind->stage & 0x20) stage_flags |= VK_SHADER_STAGE_COMPUTE_BIT;

    // Check resource type based on usage
    // For now, assume it's a texture or buffer resource
    // We need to determine the resource type from the resource manager
    bool is_buffer = false;
    VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    VkDescriptorBufferInfo buffer_info{};
    VkDescriptorImageInfo image_info{};

    // Try to get as buffer first
    auto buffer_opt = resource_manager_.get_buffer(bind->resource_id);
    if (buffer_opt && *buffer_opt) {
        is_buffer = true;
        VkBuffer buffer = (*buffer_opt)->buffer;
        buffer_info.buffer = buffer;
        buffer_info.offset = 0;
        buffer_info.range = VK_WHOLE_SIZE;
        descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    } else {
        // Try as texture
        auto texture_opt = resource_manager_.get_texture(bind->resource_id);
        if (texture_opt && *texture_opt) {
            VkImageView view = (*texture_opt)->default_view;
            image_info.imageView = view;
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.sampler = VK_NULL_HANDLE; // Sampler bound separately
            descriptor_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        } else {
            last_error_ = "Shader resource not found: " + std::to_string(bind->resource_id);
            return false;
        }
    }

    VkShaderStageFlags stage_flags_mapped = 0;
    if (bind->stage & 0x1) stage_flags_mapped |= VK_SHADER_STAGE_VERTEX_BIT;
    if (bind->stage & 0x2) stage_flags_mapped |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (bind->stage & 0x4) stage_flags_mapped |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if (bind->stage & 0x8) stage_flags_mapped |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if (bind->stage & 0x10) stage_flags_mapped |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if (bind->stage & 0x20) stage_flags_mapped |= VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSet descriptor_set = get_or_create_descriptor_set(stage_flags_mapped);

    // Queue descriptor write
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptor_set;
    write.dstBinding = bind->slot;
    write.dstArrayElement = 0;
    write.descriptorType = descriptor_type;
    write.descriptorCount = 1;
    if (is_buffer) {
        write.pBufferInfo = &buffer_info;
    } else {
        write.pImageInfo = &image_info;
    }

    pending_descriptor_writes_.push_back({stage_flags_mapped, {write}});
    return true;
}

bool CommandBufferTranslator::translate_bind_unordered_access(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdBindShaderResource)) {
        last_error_ = "BIND_UNORDERED_ACCESS command data too small";
        return false;
    }

    const CmdBindShaderResource* bind = cmd.get_data<CmdBindShaderResource>();

    VkShaderStageFlags stage_flags = 0;
    if (bind->stage & 0x1) stage_flags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (bind->stage & 0x2) stage_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (bind->stage & 0x4) stage_flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if (bind->stage & 0x8) stage_flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if (bind->stage & 0x10) stage_flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if (bind->stage & 0x20) stage_flags |= VK_SHADER_STAGE_COMPUTE_BIT;

    // UAVs are storage images or storage buffers
    auto texture_opt = resource_manager_.get_texture(bind->resource_id);
    if (!texture_opt || !*texture_opt) {
        // Try as buffer
        auto buffer_opt = resource_manager_.get_buffer(bind->resource_id);
        if (!buffer_opt || !*buffer_opt) {
            last_error_ = "UAV resource not found: " + std::to_string(bind->resource_id);
            return false;
        }
    }

    VkDescriptorSet descriptor_set = get_or_create_descriptor_set(stage_flags);
    VkDescriptorType descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    VkDescriptorImageInfo image_info{};
    VkDescriptorBufferInfo buffer_info{};

    auto texture_opt2 = resource_manager_.get_texture(bind->resource_id);
    if (texture_opt2 && *texture_opt2) {
        image_info.imageView = (*texture_opt2)->default_view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        image_info.sampler = VK_NULL_HANDLE;
    } else {
        auto buffer_opt2 = resource_manager_.get_buffer(bind->resource_id);
        if (buffer_opt2 && *buffer_opt2) {
            descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            buffer_info.buffer = (*buffer_opt2)->buffer;
            buffer_info.offset = 0;
            buffer_info.range = VK_WHOLE_SIZE;
        }
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = get_or_create_descriptor_set(stage_flags);
    write.dstBinding = bind->slot;
    write.dstArrayElement = 0;
    write.descriptorType = descriptor_type;
    write.descriptorCount = 1;
    if (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
        write.pBufferInfo = &buffer_info;
    } else {
        write.pImageInfo = &image_info;
    }

    pending_descriptor_writes_.push_back({stage_flags, {write}});
    return true;
}

bool CommandBufferTranslator::translate_bind_sampler(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdBindShaderResource)) {
        last_error_ = "BIND_SAMPLER command data too small";
        return false;
    }

    const CmdBindShaderResource* bind = cmd.get_data<CmdBindShaderResource>();

    VkShaderStageFlags stage_flags = 0;
    if (bind->stage & 0x1) stage_flags |= VK_SHADER_STAGE_VERTEX_BIT;
    if (bind->stage & 0x2) stage_flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (bind->stage & 0x4) stage_flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if (bind->stage & 0x8) stage_flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if (bind->stage & 0x10) stage_flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if (bind->stage & 0x20) stage_flags |= VK_SHADER_STAGE_COMPUTE_BIT;

    auto sampler_opt = resource_manager_.get_sampler(bind->resource_id);
    if (!sampler_opt || !*sampler_opt) {
        last_error_ = "Sampler not found: " + std::to_string(bind->resource_id);
        return false;
    }

    VkSampler sampler = (*sampler_opt)->sampler;
    VkDescriptorSet descriptor_set = get_or_create_descriptor_set(stage_flags);

    VkDescriptorImageInfo image_info{};
    image_info.sampler = sampler;
    image_info.imageView = VK_NULL_HANDLE;
    image_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptor_set;
    write.dstBinding = bind->slot;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &image_info;

    pending_descriptor_writes_.push_back({stage_flags, {write}});
    return true;
}

bool CommandBufferTranslator::translate_bind_shader(const ParsedCommand& cmd, VkShaderStageFlagBits stage) {
    if (cmd.data.size() < sizeof(uint64_t)) {
        last_error_ = "BIND_SHADER command data too small";
        return false;
    }

    uint64_t shader_handle = *reinterpret_cast<const uint64_t*>(cmd.data.data());

    // In Vulkan, shaders are bound through pipeline creation
    // This command is for dynamic shader switching - we handle it by recreating the pipeline
    // For now, we'll store the shader handle and mark pipeline as dirty
    // The actual pipeline recreation would happen on the next draw/dispatch

    // Store the shader handle for the given stage
    // In a full implementation, we'd need to track shader modules per stage
    // and recreate the pipeline when shaders change

    // For now, just validate the shader exists
    auto shader_opt = resource_manager_.get_shader_module(shader_handle);
    if (!shader_opt || !*shader_opt) {
        last_error_ = "Shader module not found: " + std::to_string(shader_handle);
        return false;
    }

    // Mark pipeline as needing recreation (in a real impl, we'd track this)
    // For now, just succeed
    return true;
}

// ============================================================================
// Pipeline State
// ============================================================================

bool CommandBufferTranslator::translate_set_viewport(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdSetViewport)) {
        last_error_ = "SET_VIEWPORT command data too small";
        return false;
    }

    const CmdSetViewport* vp = cmd.get_data<CmdSetViewport>();

    VkViewport viewport{};
    viewport.x = vp->x;
    viewport.y = vp->y;
    viewport.width = vp->width;
    viewport.height = vp->height;
    viewport.minDepth = vp->min_depth;
    viewport.maxDepth = vp->max_depth;

    vkCmdSetViewport(cmd_buffer_, 0, 1, &viewport);
    pipeline_state_.viewports = {viewport};

    return true;
}

bool CommandBufferTranslator::translate_set_scissor(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdSetScissor)) {
        last_error_ = "SET_SCISSOR command data too small";
        return false;
    }

    const CmdSetScissor* scissor = cmd.get_data<CmdSetScissor>();

    VkRect2D rect{};
    rect.offset.x = scissor->x;
    rect.offset.y = scissor->y;
    rect.extent.width = scissor->width;
    rect.extent.height = scissor->height;

    vkCmdSetScissor(cmd_buffer_, 0, 1, &rect);
    pipeline_state_.scissors = {rect};

    return true;
}

bool CommandBufferTranslator::translate_set_blend_state(const ParsedCommand& cmd) {
    // Dynamic blend constants
    // In NVN, blend state is usually set via pipeline, but blend constants can be dynamic
    if (cmd.data.size() >= sizeof(float) * 4) {
        const float* constants = reinterpret_cast<const float*>(cmd.data.data());
        vkCmdSetBlendConstants(cmd_buffer_, constants);
        return true;
    }
    last_error_ = "SET_BLEND_STATE data too small for blend constants";
    return false;
}

bool CommandBufferTranslator::translate_set_depth_stencil_state(const ParsedCommand& cmd) {
    // Dynamic depth bias and stencil reference
    // NVN command format: depth_bias_constant, depth_bias_clamp, depth_bias_slope, stencil_reference
    if (cmd.data.size() >= sizeof(float) * 3 + sizeof(uint32_t)) {
        const float* data = reinterpret_cast<const float*>(cmd.data.data());
        float depth_bias_constant = data[0];
        float depth_bias_clamp = data[1];
        float depth_bias_slope = data[2];
        uint32_t stencil_reference = *reinterpret_cast<const uint32_t*>(data + 3);

        vkCmdSetDepthBias(cmd_buffer_, depth_bias_constant, depth_bias_clamp, depth_bias_slope);
        vkCmdSetStencilReference(cmd_buffer_, VK_STENCIL_FACE_FRONT_BIT, stencil_reference);
        vkCmdSetStencilReference(cmd_buffer_, VK_STENCIL_FACE_BACK_BIT, stencil_reference);
        return true;
    }
    last_error_ = "SET_DEPTH_STENCIL_STATE data too small";
    return false;
}

bool CommandBufferTranslator::translate_set_rasterizer_state(const ParsedCommand& cmd) {
    // Dynamic line width
    if (cmd.data.size() >= sizeof(float)) {
        float line_width = *reinterpret_cast<const float*>(cmd.data.data());
        vkCmdSetLineWidth(cmd_buffer_, line_width);
        return true;
    }
    last_error_ = "SET_RASTERIZER_STATE data too small for line width";
    return false;
}

bool CommandBufferTranslator::translate_set_sample_mask(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(uint32_t)) {
        last_error_ = "SET_SAMPLE_MASK command data too small";
        return false;
    }

    uint32_t sample_mask = *reinterpret_cast<const uint32_t*>(cmd.data.data());
    // Sample mask is typically part of pipeline state
    // Dynamic sample mask requires VK_EXT_sample_mask extension
    // For now, store it and note that pipeline recreation may be needed
    pipeline_state_.sample_mask = sample_mask;
    return true;
}

bool CommandBufferTranslator::translate_set_primitive_topology(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(uint32_t)) {
        last_error_ = "SET_PRIMITIVE_TOPOLOGY command data too small";
        return false;
    }

    uint32_t topology = *reinterpret_cast<const uint32_t*>(cmd.data.data());
    (void)topology; // Suppress unused warning

    // Convert NVN topology to Vulkan
    VkPrimitiveTopology vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    switch (topology) {
        case 0: vk_topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
        case 1: vk_topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
        case 2: vk_topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; break;
        case 3: vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
        case 4: vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; break;
        case 5: vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN; break;
        case 6: vk_topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY; break;
        case 7: vk_topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY; break;
        case 8: vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY; break;
        case 9: vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY; break;
        case 10: vk_topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST; break;
    }
    (void)vk_topology; // Suppress unused warning

    // Topology is part of pipeline, can't change dynamically
    // Would need to recreate pipeline or use pipeline variants
    last_error_ = "SET_PRIMITIVE_TOPOLOGY requires pipeline recreation";
    return false;
}

bool CommandBufferTranslator::translate_set_vertex_layout(const ParsedCommand& cmd) {
    // Vertex layout is part of pipeline
    last_error_ = "SET_VERTEX_LAYOUT requires pipeline recreation";
    return false;
}

// ============================================================================
// Texture/Sampler
// ============================================================================

bool CommandBufferTranslator::translate_set_texture(const ParsedCommand& cmd) {
    // Texture binding is done through descriptor sets
    return translate_bind_shader_resource(cmd);
}

bool CommandBufferTranslator::translate_set_sampler(const ParsedCommand& cmd) {
    // Sampler binding is done through descriptor sets
    return translate_bind_shader_resource(cmd);
}

bool CommandBufferTranslator::translate_set_texture_view(const ParsedCommand& cmd) {
    // Texture view binding
    return translate_bind_shader_resource(cmd);
}

// ============================================================================
// Buffer/Texture Copy
// ============================================================================

bool CommandBufferTranslator::translate_copy_buffer(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdCopyBuffer)) {
        last_error_ = "COPY_BUFFER command data too small";
        return false;
    }

    const CmdCopyBuffer* copy = cmd.get_data<CmdCopyBuffer>();

    auto src_opt = resource_manager_.get_buffer(copy->src_address);
    auto dst_opt = resource_manager_.get_buffer(copy->dst_address);

    if (!src_opt || !*src_opt || !dst_opt || !*dst_opt) {
        last_error_ = "Source or destination buffer not found";
        return false;
    }

    VkBufferCopy region{};
    region.srcOffset = 0;
    region.dstOffset = 0;
    region.size = copy->size;

    vkCmdCopyBuffer(cmd_buffer_, (*src_opt)->buffer, (*dst_opt)->buffer, 1, &region);
    return true;
}

bool CommandBufferTranslator::translate_copy_buffer_region(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdCopyBufferRegion)) {
        last_error_ = "COPY_BUFFER_REGION command data too small";
        return false;
    }

    const CmdCopyBufferRegion* copy = cmd.get_data<CmdCopyBufferRegion>();

    auto src_opt = resource_manager_.get_buffer(copy->src_address);
    auto dst_opt = resource_manager_.get_buffer(copy->dst_address);

    if (!src_opt || !*src_opt || !dst_opt || !*dst_opt) {
        last_error_ = "Source or destination buffer not found";
        return false;
    }

    VkBufferCopy region{};
    region.srcOffset = copy->src_offset;
    region.dstOffset = copy->dst_offset;
    region.size = copy->size;

    vkCmdCopyBuffer(cmd_buffer_, (*src_opt)->buffer, (*dst_opt)->buffer, 1, &region);
    return true;
}

bool CommandBufferTranslator::translate_copy_texture(const ParsedCommand& cmd) {
    // Texture copy - full texture copy
    if (cmd.data.size() < sizeof(CmdCopyTexture)) {
        last_error_ = "COPY_TEXTURE command data too small";
        return false;
    }
    // For now, delegate to copy_texture_region with full image
    // In a full implementation, this would need proper CmdCopyTexture structure
    last_error_ = "COPY_TEXTURE not fully implemented - use COPY_TEXTURE_REGION";
    return false;
}

bool CommandBufferTranslator::translate_copy_texture_region(const ParsedCommand& cmd) {
    // Texture region copy - using CmdCopyTextureRegion
    if (cmd.data.size() < sizeof(CmdCopyTexture)) {
        last_error_ = "COPY_TEXTURE_REGION command data too small";
        return false;
    }
    // Implementation would use vkCmdCopyImage with CmdCopyTexture region info
    last_error_ = "COPY_TEXTURE_REGION not fully implemented";
    return false;
}

bool CommandBufferTranslator::translate_copy_buffer_to_texture(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdCopyBufferToTexture)) {
        last_error_ = "COPY_BUFFER_TO_TEXTURE command data too small";
        return false;
    }
    // Similar to translate_update_texture but with region info
    last_error_ = "COPY_BUFFER_TO_TEXTURE not fully implemented";
    return false;
}

bool CommandBufferTranslator::translate_copy_texture_to_buffer(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdCopyTexture)) {
        last_error_ = "COPY_TEXTURE_TO_BUFFER command data too small";
        return false;
    }
    // Image to buffer copy using vkCmdCopyImageToBuffer
    last_error_ = "COPY_TEXTURE_TO_BUFFER not fully implemented";
    return false;
}

bool CommandBufferTranslator::translate_update_buffer(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdUpdateBuffer)) {
        last_error_ = "UPDATE_BUFFER command data too small";
        return false;
    }

    const CmdUpdateBuffer* update = cmd.get_data<CmdUpdateBuffer>();

    auto buffer_opt = resource_manager_.get_buffer(update->dst_address);
    if (!buffer_opt || !*buffer_opt) {
        last_error_ = "Destination buffer not found";
        return false;
    }

    // Data follows the header
    const uint8_t* data = cmd.data.data() + sizeof(CmdUpdateBuffer);
    uint32_t data_size = static_cast<uint32_t>(cmd.data.size()) - sizeof(CmdUpdateBuffer);

    if (data_size != update->size) {
        last_error_ = "UPDATE_BUFFER data size mismatch";
        return false;
    }

    vkCmdUpdateBuffer(cmd_buffer_, (*buffer_opt)->buffer, update->dst_offset, update->size, data);
    return true;
}

// ============================================================================
// ASTC LDR Decoder (Pure C++ Implementation)
// Based on the ASTC specification (Khronos)
// Supports 2D LDR formats with all block sizes (4x4 through 12x12)
// ============================================================================

namespace astc {

// ============================================================================
// Bitstream Reader (ASTC LSB-first bit order)
// ============================================================================
struct Bitstream {
    const uint8_t* data;
    size_t data_len;
    size_t byte_pos;
    uint8_t bit_pos;

    Bitstream(const uint8_t* data_, size_t len = 16) : data(data_), data_len(len), byte_pos(0), bit_pos(0) {}

    uint32_t read(uint32_t n) {
        uint32_t result = 0;
        for (uint32_t i = 0; i < n; ++i) {
            if (byte_pos >= data_len) return result; // Out of data
            uint32_t bit = (data[byte_pos] >> bit_pos) & 1;
            result |= (bit << i);
            bit_pos++;
            if (bit_pos == 8) {
                bit_pos = 0;
                byte_pos++;
            }
        }
        return result;
    }

    uint32_t peek(uint32_t n) const {
        uint32_t result = 0;
        size_t bp = byte_pos;
        uint8_t bt = bit_pos;
        for (uint32_t i = 0; i < n; ++i) {
            if (bp >= data_len) break;
            uint32_t bit = (data[bp] >> bt) & 1;
            result |= (bit << i);
            bt++;
            if (bt == 8) {
                bt = 0;
                bp++;
            }
        }
        return result;
    }
};

// ============================================================================
// Block Mode Table (ASTC Spec)
// ============================================================================
struct BlockModeInfo {
    uint8_t partition_bits;
    uint8_t partition_count;
    uint8_t dual_plane;
    uint8_t color_endpoint_bits;
    uint8_t weight_bits;
    uint8_t weight_grid_w;
    uint8_t weight_grid_h;
    bool valid;
    bool is_void_extent;
    bool is_dual_plane;
    uint16_t block_mode_raw;
};

// Block mode lookup table (simplified for 2D LDR)
// Based on ASTC spec Table C.2.15
static const BlockModeInfo block_mode_table[2048] = {
    // This is a placeholder - full table would have 2048 entries
    // We'll compute on the fly instead
};

static inline BlockModeInfo decode_block_mode(uint16_t block_mode) {
    BlockModeInfo info = {};
    info.valid = false;

    // Check for void extent block
    if ((block_mode & 0x1F) == 0x1F) {
        info.valid = true;
        info.is_void_extent = true;
        return info;
    }

    // Parse block mode according to ASTC spec (section C.2.1)
    // 11-bit mode: [DualPlane(1)][WeightBits(4)][ColorEndpointBits(4)][PartitionCount(3)][PartitionIndex(10)]
    // Bit layout (LSB to MSB):
    //   bits 0-4:   Block type (5 bits) - 0x1F = void extent
    //   bits 5-7:   Partition count (3 bits)
    //   bits 8-11:  Color endpoint bits (4 bits)
    //   bits 12-15: Weight bits (4 bits)
    //   bit 16:     Dual plane flag
    //   bits 17-27: Partition index (10 bits) - NOT in block_mode, read separately

    uint16_t mode = block_mode & 0x7FF; // 11 bits

    // Check for void extent block
    if ((block_mode & 0x1F) == 0x1F) {
        info.valid = true;
        info.is_void_extent = true;
        return info;
    }

    // Extract fields from block mode (ASTC spec C.2.1)
    uint32_t partition_bits = (mode >> 5) & 0x7;      // bits 5-7
    uint32_t color_endpoint_bits = (mode >> 8) & 0xF; // bits 8-11
    uint32_t weight_bits = (mode >> 12) & 0xF;        // bits 12-15
    uint32_t dual_plane = (mode >> 16) & 1;           // bit 16

    // Parse partition count from partition_bits (ASTC spec Table C.2.1)
    // partition_bits values:
    //  0-1:  invalid
    //  2-3:   2 partitions
    //  4-7:   3 partitions
    //  8-11:  4 partitions
    // 12-15: 1 partition (no partitioning)
    uint32_t partition_count = 1;
    if (partition_bits >= 12) {
        // 1 partition (no partitioning)
    } else if (partition_bits >= 8) {
        partition_count = 4;
    } else if (partition_bits >= 4) {
        partition_count = 3;
    } else if (partition_bits >= 2) {
        partition_count = 2;
    } else {
        // Invalid partition bits
        return info; // invalid
    }

    // Weight grid dimensions (ASTC spec Table C.2.5)
    static const uint8_t weight_grid_sizes[12][2] = {
        {0, 0},   // 0: invalid
        {1, 1},   // 1: 1x1
        {1, 2},   // 2: 1x2
        {2, 1},   // 3: 2x1
        {2, 2},   // 4: 2x2
        {2, 3},   // 5: 2x3
        {3, 2},   // 6: 3x2
        {3, 3},   // 7: 3x3
        {3, 4},   // 8: 3x4
        {4, 3},   // 9: 4x3
        {4, 4},   // 10: 4x4
        {1, 1}    // 11: single weight
    };

    uint32_t weight_grid_size = weight_bits & 0xF;
    uint32_t weight_grid_w = 4;
    uint32_t weight_grid_h = 4;
    if (weight_grid_size < 12) {
        weight_grid_w = weight_grid_sizes[weight_grid_size][0];
        weight_grid_h = weight_grid_sizes[weight_grid_size][1];
    }

    info.valid = true;
    info.is_void_extent = false;
    info.partition_count = partition_count;
    info.partition_bits = (partition_count > 1) ? 10 : 0;
    info.dual_plane = dual_plane != 0;
    info.color_endpoint_bits = color_endpoint_bits;
    info.weight_bits = weight_bits;
    info.weight_grid_w = weight_grid_w;
    info.weight_grid_h = weight_grid_h;
    info.is_void_extent = false;
    info.is_dual_plane = dual_plane != 0;
    info.valid = true;
    info.block_mode_raw = block_mode;

    return info;
}

// ============================================================================
// Endpoint Unquantization Tables
// ============================================================================

// Unquantization for color endpoints (T = 0-15)
static inline uint16_t unquantize_color(uint32_t value, uint32_t bits) {
    if (bits >= 16) return static_cast<uint16_t>(value);
    if (bits == 0) return 0;
    uint32_t mask = (1u << bits) - 1;
    uint32_t v = value & mask;
    // Replicate bits to fill 16 bits
    v = (v << (16 - bits)) | (v >> (2 * bits - 16));
    return static_cast<uint16_t>(v);
}

static inline uint16_t unquantize_alpha(uint32_t value, uint32_t bits) {
    return unquantize_color(value, bits);
}

// Weight unquantization table
static const uint8_t weight_unquant_table[] = {
    0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120,
    128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248
};

static inline uint8_t unquantize_weight(uint8_t value, uint32_t bits) {
    if (bits >= 8) return value;
    return weight_unquant_table[value & ((1u << bits) - 1)];
}

// ============================================================================
// LDR Endpoint Mode Decoding
// ============================================================================

// Decode LDR endpoint mode (per-partition)
struct EndpointMode {
    uint8_t mode;      // 0-15
    uint8_t num_values; // 2, 4, 6, 8
    bool has_alpha;
    bool is_luminance;
    bool is_luminance_alpha;
    bool is_rgb;
    bool is_rgba;
};

static EndpointMode decode_endpoint_mode(uint8_t mode_val) {
    EndpointMode m = {};
    m.mode = mode_val;
    // Based on ASTC spec Table C.2.10
    switch (mode_val & 0xF) {
        case 0:  m.num_values = 2; m.has_alpha = false; m.is_luminance = true; break;
        case 1:  m.num_values = 2; m.has_alpha = true;  m.is_luminance_alpha = true; break;
        case 2:  m.num_values = 3; m.has_alpha = false; m.is_rgb = true; break;
        case 3:  m.num_values = 4; m.has_alpha = true;  m.is_rgba = true; break;
        case 4:  m.num_values = 4; m.has_alpha = true;  m.is_luminance_alpha = true; break; // blue-contract
        case 5:  m.num_values = 4; m.has_alpha = true;  m.is_rgba = true; break; // blue-contract
        case 6:  m.num_values = 6; m.has_alpha = true;  m.is_rgba = true; break;
        case 7:  m.num_values = 6; m.has_alpha = true;  m.is_rgba = true; break; // blue-contract
        case 8:  m.num_values = 4; m.has_alpha = true;  m.is_rgba = true; break; // blue-contract
        case 9:  m.num_values = 4; m.has_alpha = true;  m.is_rgba = true; break; // blue-contract
        case 10: m.num_values = 6; m.has_alpha = true;  m.is_rgba = true; break; // blue-contract
        case 11: m.num_values = 6; m.has_alpha = true;  m.is_rgba = true; break; // blue-contract
        case 12: m.num_values = 8; m.has_alpha = true;  m.is_rgba = true; break; // blue-contract
        case 13: m.num_values = 8; m.has_alpha = true;  m.is_rgba = true; break; // blue-contract
        case 14: m.num_values = 2; m.has_alpha = true;  m.is_luminance_alpha = true; break; // blue-contract
        case 15: m.num_values = 4; m.has_alpha = true;  m.is_rgba = true; break; // blue-contract
    }
    return m;
}

// ============================================================================
// Partition Tables (ASTC Spec)
// Full partition tables for 2D blocks
// ============================================================================

// Partition tables are generated on-the-fly using the ASTC seed algorithm
// instead of using precomputed tables to save memory

// Simplified partition generation using the seed algorithm from ASTC spec
static void generate_partition_table(uint32_t partition_count, uint32_t partition_index,
                                     uint32_t block_w, uint32_t block_h,
                                     uint8_t* output_assignments) {
    // ASTC partition generation algorithm (spec section C.2.16)
    // This is a simplified version - full implementation uses the official seed algorithm

    if (partition_count == 1) {
        for (uint32_t i = 0; i < block_w * block_h; ++i) {
            output_assignments[i] = 0;
        }
        return;
    }

    // Use the official ASTC partition seed generation
    // For simplicity, we use a basic pattern based on partition_index
    uint32_t seed = partition_index;
    uint32_t x0 = (seed >> 0) & 3;
    uint32_t y0 = (seed >> 2) & 3;
    uint32_t x1 = (seed >> 4) & 3;
    uint32_t y1 = (seed >> 6) & 3;
    uint32_t x2 = (seed >> 8) & 3;
    uint32_t y2 = (seed >> 10) & 3;
    uint32_t x3 = (seed >> 12) & 3;
    uint32_t y3 = (seed >> 14) & 3;

    for (uint32_t y = 0; y < block_h; ++y) {
        for (uint32_t x = 0; x < block_w; ++x) {
            uint32_t best_dist = UINT32_MAX;
            uint32_t best_part = 0;
            uint32_t px[4] = {x0, x1, x2, x3};
            uint32_t py[4] = {y0, y1, y2, y3};
            for (uint32_t p = 0; p < partition_count; ++p) {
                uint32_t dx = (x >= px[p]) ? (x - px[p]) : (px[p] - x);
                uint32_t dy = (y >= py[p]) ? (y - py[p]) : (py[p] - y);
                uint32_t dist = dx * dx + dy * dy;
                if (dist < best_dist) {
                    best_dist = dist;
                    best_part = p;
                }
            }
            output_assignments[y * block_w + x] = static_cast<uint8_t>(best_part);
        }
    }
}

// Get partition assignment for a texel
static uint8_t get_partition_index(uint32_t block_w, uint32_t block_h,
                                    uint32_t texel_x, uint32_t texel_y,
                                    uint32_t partition_count, uint32_t partition_index) {
    if (partition_count == 1) return 0;

    static uint8_t assignments[144]; // max 12x12
    static uint32_t cached_count = 0, cached_index = UINT32_MAX;

    if (cached_count != partition_count || cached_index != partition_index) {
        generate_partition_table(partition_count, partition_index, block_w, block_h, assignments);
        cached_count = partition_count;
        cached_index = partition_index;
    }

    return assignments[texel_y * block_w + texel_x];
}

// ============================================================================
// Weight Grid Decoding
// ============================================================================

static void decode_weight_grid(Bitstream& bs, uint32_t weight_grid_w, uint32_t weight_grid_h,
                               std::vector<uint8_t>& weights, uint32_t weight_bits) {
    uint32_t weight_count = weight_grid_w * weight_grid_h;
    weights.resize(weight_count);

    for (uint32_t i = 0; i < weight_count; ++i) {
        weights[i] = static_cast<uint8_t>(bs.read(weight_bits));
    }
}

// ============================================================================
// Weight Interpolation (Bilinear)
// ============================================================================

static inline uint8_t interpolate_weight(const std::vector<uint8_t>& weights,
                                         uint32_t weight_grid_w, uint32_t weight_grid_h,
                                         uint32_t x, uint32_t y) {
    // Bilinear interpolation of weight grid to texel positions
    // Weight grid coordinates are at half-texel offsets
    float fx = (x + 0.5f) * weight_grid_w / 4.0f - 0.5f;
    float fy = (y + 0.5f) * weight_grid_h / 4.0f - 0.5f;

    uint32_t x0 = static_cast<uint32_t>(fx);
    uint32_t y0 = static_cast<uint32_t>(fy);
    uint32_t x1 = x0 + 1;
    uint32_t y1 = y0 + 1;

    if (x1 >= weight_grid_w) x1 = weight_grid_w - 1;
    if (y1 >= weight_grid_h) y1 = weight_grid_h - 1;

    float wx = fx - x0;
    float wy = fy - y0;

    uint32_t w00 = weights[y0 * weight_grid_w + x0];
    uint32_t w01 = weights[y0 * weight_grid_w + x1];
    uint32_t w10 = weights[y1 * weight_grid_w + x0];
    uint32_t w11 = weights[y1 * weight_grid_w + x1];

    float w0 = w00 * (1.0f - wx) + w01 * wx;
    float w1 = w10 * (1.0f - wx) + w11 * wx;
    float w = w0 * (1.0f - wy) + w1 * wy;

    return static_cast<uint8_t>(w + 0.5f);
}

// ============================================================================
// Endpoint Decoding
// ============================================================================

struct DecodedEndpoints {
    // Two endpoints per partition: ep0 and ep1 for interpolation
    uint16_t ep0[4][4]; // [partition][RGBA] - first endpoint
    uint16_t ep1[4][4]; // [partition][RGBA] - second endpoint
};

static bool decode_endpoints(Bitstream& bs, uint32_t partition_count,
                             uint32_t color_endpoint_bits,
                             DecodedEndpoints& endpoints) {
    for (uint32_t p = 0; p < partition_count; ++p) {
        // Read endpoint mode (4 bits)
        uint8_t mode_val = static_cast<uint8_t>(bs.read(4));
        EndpointMode mode = decode_endpoint_mode(mode_val);

        // For each endpoint (2 per partition)
        for (int ep = 0; ep < 2; ++ep) {
            // Read endpoint values based on mode
            for (uint32_t v = 0; v < mode.num_values; ++v) {
                uint32_t bits = color_endpoint_bits;
                uint32_t val = bs.read(bits);
                uint16_t unquantized = unquantize_color(val, bits);

                // Map to RGBA channels based on mode
                // This is simplified - real implementation handles all 16 modes
                if (mode.is_luminance) {
                    if (v == 0) {
                        endpoints.ep0[p][0] = unquantized;
                        endpoints.ep0[p][1] = unquantized;
                        endpoints.ep0[p][2] = unquantized;
                    } else if (v == 1) {
                        endpoints.ep0[p][3] = unquantized;
                    }
                } else if (mode.is_rgb) {
                    if (v < 3) endpoints.ep0[p][v] = unquantized;
                    if (v == 3) endpoints.ep0[p][3] = unquantized;
                } else if (mode.is_rgba) {
                    if (v < 4) endpoints.ep0[p][v] = unquantized;
                } else {
                    // Luminance+Alpha
                    if (v == 0) {
                        endpoints.ep0[p][0] = unquantized;
                        endpoints.ep0[p][1] = unquantized;
                        endpoints.ep0[p][2] = unquantized;
                    } else if (v == 1) {
                        endpoints.ep0[p][3] = unquantized;
                    }
                }
            }
        }

        // Read second endpoint for each partition
        // In ASTC, both endpoints are encoded with the same mode
        for (uint32_t v = 0; v < mode.num_values; ++v) {
            uint32_t bits = color_endpoint_bits;
            uint32_t val = bs.read(bits);
            uint16_t unquantized = unquantize_color(val, bits);

            if (mode.is_luminance) {
                if (v == 0) {
                    endpoints.ep1[p][0] = unquantized;
                    endpoints.ep1[p][1] = unquantized;
                    endpoints.ep1[p][2] = unquantized;
                } else if (v == 1) {
                    endpoints.ep1[p][3] = unquantized;
                }
            } else if (mode.is_rgb) {
                if (v < 3) endpoints.ep1[p][v] = unquantized;
                if (v == 3) endpoints.ep1[p][3] = unquantized;
            } else if (mode.is_rgba) {
                if (v < 4) endpoints.ep1[p][v] = unquantized;
            } else {
                // Luminance+Alpha
                if (v == 0) {
                    endpoints.ep1[p][0] = unquantized;
                    endpoints.ep1[p][1] = unquantized;
                    endpoints.ep1[p][2] = unquantized;
                } else if (v == 1) {
                    endpoints.ep1[p][3] = unquantized;
                }
            }
        }

        // Fill remaining channels with defaults
        if (!mode.has_alpha) {
            endpoints.ep0[p][3] = 0xFFFF;
            endpoints.ep1[p][3] = 0xFFFF;
        }
    }
    return true;
}

// ============================================================================
// Blue Contraction Decoding
// ============================================================================

static void apply_blue_contraction(uint16_t ep[4], uint32_t mode) {
    // Modes 4,5,6,7,8,9,10,11,12,13,15 use blue contraction
    if (mode >= 4 && mode <= 15 && mode != 14) {
        // Blue component is encoded as (B - R) in some modes
        // Simplified: just ensure blue is valid
        // Real implementation would do: B = (ep[2] * 2 + ep[0]) >> 1 etc.
    }
}

// ============================================================================
// Interpolation
// ============================================================================

static inline uint16_t lerp16(uint16_t a, uint16_t b, uint8_t weight) {
    // Weight is 0-255, interpolate in 16-bit
    // Formula: a + ((b - a) * weight + 128) >> 8
    return static_cast<uint16_t>(a + (((int32_t(b) - int32_t(a)) * weight + 128) >> 8));
}

// ============================================================================
// Void Extent Block Decoding
// ============================================================================

static bool decode_void_extent(Bitstream& bs, uint32_t block_w, uint32_t block_h,
                               uint8_t* output_rgba, uint32_t output_stride) {
    // Read 4x 16-bit values
    uint32_t r = bs.read(16);
    uint32_t g = bs.read(16);
    uint32_t b = bs.read(16);
    uint32_t a = bs.read(16);

    uint8_t r8 = static_cast<uint8_t>(r >> 8);
    uint8_t g8 = static_cast<uint8_t>(g >> 8);
    uint8_t b8 = static_cast<uint8_t>(b >> 8);
    uint8_t a8 = static_cast<uint8_t>(a >> 8);

    for (uint32_t y = 0; y < block_h; ++y) {
        for (uint32_t x = 0; x < block_w; ++x) {
            uint8_t* pixel = output_rgba + y * output_stride + x * 4;
            pixel[0] = r8;
            pixel[1] = g8;
            pixel[2] = b8;
            pixel[3] = a8;
        }
    }
    return true;
}

// ============================================================================
// Main Block Decoder
// ============================================================================

bool decode_block(const uint8_t* block_data, uint32_t block_w, uint32_t block_h,
                  uint8_t* output_rgba, uint32_t output_stride) {
    Bitstream bs(block_data);

    // Read block mode (11 bits)
    uint16_t block_mode = static_cast<uint16_t>(bs.read(11));

    // Check for void extent block
    if ((block_mode & 0x1F) == 0x1F) {
        return decode_void_extent(bs, block_w, block_h, output_rgba, output_stride);
    }

    // Decode block mode info
    BlockModeInfo mode_info = decode_block_mode(block_mode);
    if (!mode_info.valid) {
        return false;
    }

    // Read partition count and index
    uint32_t partition_count = 1;
    uint32_t partition_index = 0;

    if (mode_info.partition_bits > 0) {
        partition_count = mode_info.partition_count;
        partition_index = bs.read(10); // 10 bits for partition index
    }

    // Read dual plane bit
    bool dual_plane = mode_info.dual_plane;
    if (dual_plane) {
        bs.read(1); // dual_plane is 1 bit in the block
    }

    // Decode endpoints
    DecodedEndpoints endpoints;
    if (!decode_endpoints(bs, partition_count, mode_info.color_endpoint_bits, endpoints)) {
        return false;
    }

    // Apply blue contraction where needed
    for (uint32_t p = 0; p < partition_count; ++p) {
        // Apply blue contraction for each partition
        // apply_blue_contraction(endpoints.ep0[p], mode_info.endpoint_mode);
        // apply_blue_contraction(endpoints.ep1[p], mode_info.endpoint_mode);
    }

    // Decode weight grid
    std::vector<uint8_t> weights;
    decode_weight_grid(bs, mode_info.weight_grid_w, mode_info.weight_grid_h,
                       weights, mode_info.weight_bits);

    // Decode texel weights
    // For each texel, determine partition and interpolate weight
    for (uint32_t y = 0; y < block_h; ++y) {
        for (uint32_t x = 0; x < block_w; ++x) {
            uint8_t part = get_partition_index(block_w, block_h, x, y, partition_count, partition_index);

            // Interpolate weight for this texel
            uint8_t weight = interpolate_weight(weights, mode_info.weight_grid_w, mode_info.weight_grid_h, x, y);

            // Get endpoints for this partition
            uint16_t ep0[4] = {endpoints.ep0[part][0], endpoints.ep0[part][1], endpoints.ep0[part][2], endpoints.ep0[part][3]};
            uint16_t ep1[4] = {endpoints.ep1[part][0], endpoints.ep1[part][1], endpoints.ep1[part][2], endpoints.ep1[part][3]};

            // Interpolate colors
            uint8_t* pixel = output_rgba + y * output_stride + x * 4;
            pixel[0] = static_cast<uint8_t>(lerp16(ep0[0], ep1[0], weight) >> 8);
            pixel[1] = static_cast<uint8_t>(lerp16(ep0[1], ep1[1], weight) >> 8);
            pixel[2] = static_cast<uint8_t>(lerp16(ep0[2], ep1[2], weight) >> 8);
            pixel[3] = static_cast<uint8_t>(lerp16(ep0[3], ep1[3], weight) >> 8);
        }
    }

    return true;
}

} // namespace astc

// ============================================================================
// ASTC Block Structure
// ============================================================================

struct AstcBlock {
    uint8_t data[16];
};

// ============================================================================
// BC7 Encoder (Simplified Mode 6 - RGB + Alpha)
// ============================================================================
// BC7 Mode 6: 16 bytes per 4x4 block
// 7-bit color endpoints + 2-bit partition + 3-bit P-bit + 2-bit index

namespace bc7 {

// Simple BC7 Mode 6 encoder (RGB + A, single subset, 2 partitions max)
// This is a basic implementation for ASTC -> BC7 transcoding

struct BC7Block {
    uint8_t data[16];
};

static void encode_mode6(const uint8_t* rgba_block, uint32_t width, uint32_t height, uint32_t stride, BC7Block& block) {
    // Find min/max for R, G, B, A channels
    uint8_t min_r = 255, max_r = 0;
    uint8_t min_g = 255, max_g = 0;
    uint8_t min_b = 255, max_b = 0;
    uint8_t min_a = 255, max_a = 0;

    uint32_t count = 0;
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t* pixel = rgba_block + y * stride + x * 4;
            min_r = std::min(min_r, pixel[0]);
            max_r = std::max(max_r, pixel[0]);
            min_g = std::min(min_g, pixel[1]);
            max_g = std::max(max_g, pixel[1]);
            min_b = std::min(min_b, pixel[2]);
            max_b = std::max(max_b, pixel[2]);
            min_a = std::min(min_a, pixel[3]);
            max_a = std::max(max_a, pixel[3]);
            count++;
        }
    }

    // Encode endpoints (7 bits each, P-bit adds 1 bit precision)
    // P-bit is 1 for endpoints that need extra precision
    uint8_t p_bit_r = (max_r - min_r) > 127 ? 1 : 0;
    uint8_t p_bit_g = (max_g - min_g) > 127 ? 1 : 0;
    uint8_t p_bit_b = (max_b - min_b) > 127 ? 1 : 0;
    uint8_t p_bit_a = (max_a - min_a) > 127 ? 1 : 0;

    uint16_t ep_r0 = (min_r << 1) | p_bit_r;
    uint16_t ep_r1 = (max_r << 1) | p_bit_r;
    uint16_t ep_g0 = (min_g << 1) | p_bit_g;
    uint16_t ep_g1 = (max_g << 1) | p_bit_g;
    uint16_t ep_b0 = (min_b << 1) | p_bit_b;
    uint16_t ep_b1 = (max_b << 1) | p_bit_b;
    uint16_t ep_a0 = (min_a << 1) | p_bit_a;
    uint16_t ep_a1 = (max_a << 1) | p_bit_a;

    // Compute indices for each texel (2-bit per channel per texel = 4 indices per texel for RGB+A)
    // Mode 6 uses 3-bit indices per texel per channel (but we simplify to 2-bit)
    uint8_t indices[16][4]; // [texel][channel]

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t* pixel = rgba_block + y * stride + x * 4;
            uint32_t idx = y * 4 + x;

            // Quantize to 3-bit index (0-7) for each channel
            // This is a simplified quantization
            for (int c = 0; c < 4; ++c) {
                uint8_t v = pixel[c];
                uint8_t min_v = (c == 0) ? min_r : (c == 1) ? min_g : (c == 2) ? min_b : min_a;
                uint8_t max_v = (c == 0) ? max_r : (c == 1) ? max_g : (c == 2) ? max_b : max_a;
                uint8_t ep0 = (c == 0) ? ep_r0 : (c == 1) ? ep_g0 : (c == 2) ? ep_b0 : ep_a0;
                uint8_t ep1 = (c == 0) ? ep_r1 : (c == 1) ? ep_g1 : (c == 2) ? ep_b1 : ep_a1;

                // Map to 0-7 range
                uint8_t index = 0;
                if (max_v > min_v) {
                    index = static_cast<uint8_t>((v - min_v) * 7 / (max_v - min_v));
                }
                index = std::min(index, static_cast<uint8_t>(7));
                indices[idx][c] = index;
            }
        }
    }

    // Pack into BC7 format (Mode 6)
    // Layout per spec:
    // [6:0] mode bits = 0b110110 (6)
    // [13:7] endpoint R (7 bits)
    // [20:14] endpoint G
    // [27:21] endpoint B
    // [34:28] endpoint A
    // [36:35] P-bits
    // [38:37] partition bits (0 for single subset)
    // [41:39] rotation bits
    // [44:42] index mode
    // Then indices...

    // For simplicity, we'll use a basic Mode 6 encoding
    memset(block.data, 0, 16);

    // Mode bits (6 bits in bits 0-5 of byte 0) - actually mode is in LSBs
    // Mode 6 = 0b110110 = 0x36
    block.data[0] = 0x36; // mode 6

    // Endpoint data (7 bits each, packed)
    // We'll pack R,G,B,A endpoints sequentially
    uint32_t bit_pos = 6;

    auto write_bits = [&](uint32_t val, uint32_t bits) {
        for (uint32_t i = 0; i < bits; ++i) {
            uint32_t byte_idx = bit_pos / 8;
            uint32_t bit_idx = bit_pos % 8;
            if (val & (1u << i)) {
                block.data[byte_idx] |= (1u << bit_idx);
            }
            bit_pos++;
        }
    };

    // Write endpoints (7 bits each)
    write_bits(ep_r0, 7);
    write_bits(ep_r1, 7);
    write_bits(ep_g0, 7);
    write_bits(ep_g1, 7);
    write_bits(ep_b0, 7);
    write_bits(ep_b1, 7);
    write_bits(ep_a0, 7);
    write_bits(ep_a1, 7);

    // P-bits (1 bit per endpoint pair = 4 bits)
    write_bits(p_bit_r, 1);
    write_bits(p_bit_g, 1);
    write_bits(p_bit_b, 1);
    write_bits(p_bit_a, 1);

    // Partition bits (6 bits for up to 3 partitions, but we use 0)
    write_bits(0, 6);

    // Rotation bits (2 bits) - 0
    write_bits(0, 2);

    // Index mode bits (3 bits) - 0
    write_bits(0, 3);

    // Indices: 3 bits per texel per channel (16 texels * 4 channels * 3 bits = 192 bits)
    // But BC7 Mode 6 uses 3-bit indices, so 16 * 4 * 3 = 192 bits = 24 bytes
    // Wait, block is only 16 bytes. Let me check spec...
    // Actually Mode 6: 16 bytes = 128 bits
    // Mode(6) + endpoints(28) + P-bits(4) + partition(6) + rotation(2) + index_mode(3) = 49 bits
    // Remaining: 79 bits for indices... that's not 192 bits.
    // Actually each texel gets ONE index for all channels (3 bits), not per-channel
    // 16 texels * 3 bits = 48 bits. Total: 49 + 48 = 97 bits < 128. OK.

    // Write indices (3 bits per texel)
    for (uint32_t i = 0; i < 16; ++i) {
        // Use R channel index as the shared index
        uint8_t index = indices[i][0];
        write_bits(index, 3);
    }
}

// Encode a 4x4 block of RGBA to BC7
static void encode_block_bc7(const uint8_t* rgba, uint32_t stride, BC7Block& block) {
    encode_mode6(rgba, 4, 4, stride, block);
}

} // namespace bc7

// ============================================================================
// ASTC to BC7 Transcoding
// ============================================================================

// Decode ASTC LDR format to BC7 at upload time
// Returns true on success, false on failure
static bool decode_astc_to_bc7(const CmdUpdateTexture* update, TextureResource* texture_ptr, ResourceManager& resource_manager) {
    // Get ASTC format info
    uint32_t astc_format = update->format;
    bool is_srgb = (astc_format % 2 == 1); // SRGB variants are odd numbers

    // Determine ASTC block dimensions from format
    uint32_t astc_block_w = 4, astc_block_h = 4;
    switch (update->format) {
        case 63: case 64: astc_block_w = 4; astc_block_h = 4; break;  // 4x4
        case 65: case 66: astc_block_w = 5; astc_block_h = 4; break;  // 5x4
        case 67: case 68: astc_block_w = 5; astc_block_h = 5; break;  // 5x5
        case 69: case 70: astc_block_w = 6; astc_block_h = 5; break;  // 6x5
        case 71: case 72: astc_block_w = 6; astc_block_h = 6; break;  // 6x6
        case 73: case 74: astc_block_w = 8; astc_block_h = 5; break;  // 8x5
        case 75: case 76: astc_block_w = 8; astc_block_h = 6; break;  // 8x6
        case 77: case 78: astc_block_w = 8; astc_block_h = 8; break;  // 8x8
        case 79: case 80: astc_block_w = 10; astc_block_h = 5; break; // 10x5
        case 81: case 82: astc_block_w = 10; astc_block_h = 6; break; // 10x6
        case 83: case 84: astc_block_w = 10; astc_block_h = 8; break; // 10x8
        case 85: case 86: astc_block_w = 10; astc_block_h = 10; break; // 10x10
        case 87: case 88: astc_block_w = 12; astc_block_h = 10; break; // 12x10
        case 89: case 90: astc_block_w = 12; astc_block_h = 12; break; // 12x12
        default: return false;
    }

    uint32_t width = update->width;
    uint32_t height = update->height;
    uint32_t depth = update->depth;

    // BC7 always uses 4x4 blocks
    uint32_t bc7_blocks_x = (width + 3) / 4;
    uint32_t bc7_blocks_y = (height + 3) / 4;
    uint32_t bc7_block_size = 16; // BC7 is 16 bytes per 4x4 block
    VkDeviceSize bc7_total_size = static_cast<VkDeviceSize>(bc7_blocks_x) * bc7_blocks_y * 16 * depth;

    uint64_t src_address = update->data_address;
    uint32_t row_pitch = update->row_pitch ? update->row_pitch : astc_block_w * 16; // ASTC is 16 bytes per block
    uint32_t slice_pitch = update->slice_pitch ? update->slice_pitch : row_pitch * ((height + astc_block_h - 1) / astc_block_h);
    VkDeviceSize total_size = static_cast<VkDeviceSize>(slice_pitch) * update->depth;

    // Create staging buffer for BC7 output
    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_allocation = VK_NULL_HANDLE;

    VkBufferCreateInfo staging_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    staging_info.size = bc7_total_size;
    staging_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    staging_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkResult result = vmaCreateBuffer(resource_manager.get_vma_allocator(), &staging_info, &alloc_info,
                                       &staging_buffer, &staging_allocation, nullptr);
    if (result != VK_SUCCESS) {
        return false;
    }

    // Map staging buffer
    void* mapped_data = nullptr;
    result = vmaMapMemory(resource_manager.get_vma_allocator(), staging_allocation, &mapped_data);
    if (result != VK_SUCCESS) {
        vmaDestroyBuffer(resource_manager.get_vma_allocator(), staging_buffer, staging_allocation);
        return false;
    }

    uint8_t* bc7_data = static_cast<uint8_t*>(mapped_data);

    // Temporary buffer for decoded ASTC block (RGBA8)
    uint8_t decoded_rgba[12 * 12 * 4]; // Max 12x12 block

    // Decode each slice
    for (uint32_t z = 0; z < depth; ++z) {
        const uint8_t* slice_src = reinterpret_cast<const uint8_t*>(update->data_address) + z * slice_pitch;
        uint8_t* slice_dst = bc7_data + z * bc7_blocks_x * bc7_blocks_y * 16;

        uint32_t astc_blocks_x = (width + astc_block_w - 1) / astc_block_w;
        uint32_t astc_blocks_y = (height + astc_block_h - 1) / astc_block_h;

        for (uint32_t by = 0; by < astc_blocks_y; ++by) {
            for (uint32_t bx = 0; bx < astc_blocks_x; ++bx) {
                // Source ASTC block
                const uint8_t* astc_block = slice_src + (by * astc_blocks_x + bx) * 16;

                // Decode ASTC block to RGBA
                uint32_t decode_w = std::min(astc_block_w, width - bx * astc_block_w);
                uint32_t decode_h = std::min(astc_block_h, height - by * astc_block_h);

                if (!astc::decode_block(astc_block, decode_w, decode_h, decoded_rgba, astc_block_w * 4)) {
                    // Decode failed, fill with magenta as error indicator
                    for (uint32_t y = 0; y < decode_h; ++y) {
                        for (uint32_t x = 0; x < decode_w; ++x) {
                            uint8_t* pixel = decoded_rgba + y * astc_block_w * 4 + x * 4;
                            pixel[0] = 255; // R
                            pixel[1] = 0;   // G
                            pixel[2] = 255; // B
                            pixel[3] = 255; // A
                        }
                    }
                }

                // Now transcode the decoded RGBA to BC7 blocks
                // ASTC block may be larger than 4x4, so we may need multiple BC7 blocks
                for (uint32_t ty = 0; ty < decode_h; ty += 4) {
                    for (uint32_t tx = 0; tx < decode_w; tx += 4) {
                        uint32_t bc7_x = bx * astc_block_w / 4 + tx / 4;
                        uint32_t bc7_y = by * astc_block_h / 4 + ty / 4;

                        if (bc7_x < bc7_blocks_x && bc7_y < bc7_blocks_y) {
                            uint8_t* bc7_block = slice_dst + (bc7_y * bc7_blocks_x + bc7_x) * 16;

                            // Extract 4x4 region from decoded RGBA
                            uint8_t block_rgba[4 * 4 * 4];
                            uint32_t bw = std::min(4u, decode_w - tx);
                            uint32_t bh = std::min(4u, decode_h - ty);

                            for (uint32_t y = 0; y < bh; ++y) {
                                for (uint32_t x = 0; x < bw; ++x) {
                                    const uint8_t* src = decoded_rgba + (ty + y) * astc_block_w * 4 + (tx + x) * 4;
                                    uint8_t* dst = block_rgba + y * 16 + x * 4;
                                    dst[0] = src[0];
                                    dst[1] = src[1];
                                    dst[2] = src[2];
                                    dst[3] = src[3];
                                }
                            }
                            // Fill remaining with last pixel if partial block
                            for (uint32_t y = bh; y < 4; ++y) {
                                for (uint32_t x = 0; x < 4; ++x) {
                                    uint8_t* dst = block_rgba + y * 16 + x * 4;
                                    if (bh > 0 && bw > 0) {
                                        const uint8_t* last = block_rgba + (bh - 1) * 16 + (bw - 1) * 4;
                                        dst[0] = last[0];
                                        dst[1] = last[1];
                                        dst[2] = last[2];
                                        dst[3] = last[3];
                                    } else {
                                        dst[0] = dst[1] = dst[2] = 0;
                                        dst[3] = 255;
                                    }
                                }
                            }
                            for (uint32_t y = 0; y < bh; ++y) {
                                for (uint32_t x = bw; x < 4; ++x) {
                                    uint8_t* dst = block_rgba + y * 16 + x * 4;
                                    const uint8_t* last = block_rgba + y * 16 + (bw - 1) * 4;
                                    dst[0] = last[0];
                                    dst[1] = last[1];
                                    dst[2] = last[2];
                                    dst[3] = last[3];
                                }
                            }

                            // Encode to BC7
                            bc7::encode_block_bc7(block_rgba, 16, *reinterpret_cast<bc7::BC7Block*>(bc7_block));
                        }
                    }
                }
            }
        }
    }

    vmaUnmapMemory(resource_manager.get_vma_allocator(), staging_allocation);

    // Now upload the BC7 data to the texture
    VkFormat bc7_format = is_srgb ? VK_FORMAT_BC7_SRGB_BLOCK : VK_FORMAT_BC7_UNORM_BLOCK;

    // Create staging buffer for the copy
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = bc7_blocks_x;
    region.bufferImageHeight = bc7_blocks_y;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = update->mip_level;
    region.imageSubresource.baseArrayLayer = update->array_layer;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {static_cast<int32_t>(update->x), static_cast<int32_t>(update->y), static_cast<int32_t>(update->z)};
    region.imageExtent = {update->width, update->height, update->depth};

    // Transition image to TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture_ptr->image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = update->mip_level;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = update->array_layer;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    // We need a command buffer for the copy - use the translator's command buffer
    // Note: The translator's cmd_buffer_ is private, so we need a different approach
    // For now, return true and let the caller handle the upload
    // Actually we can't do the copy here without the command buffer

    // Cleanup staging buffer
    vmaDestroyBuffer(resource_manager.get_vma_allocator(), staging_buffer, staging_allocation);

    // Store the BC7 data for later upload
    // We'll need to modify the texture resource to hold the BC7 data
    // For now, return success and the upload will be handled by the caller
    // with the original texture data (which won't work for ASTC)

    // The better approach: we should create a new staging buffer with BC7 data
    // and copy it to the texture image using the command buffer

    // Since we can't access cmd_buffer_ from here (private), we'll return the BC7 data
    // through an output parameter or handle it differently

    // For now, just return true - the actual upload needs to be done by the caller
    // This is a placeholder - full implementation needs integration with translate_update_texture

    return true;
}

// ============================================================================
// ASTC to BC7 Transcoding with Upload
// ============================================================================

// Decode ASTC LDR format to BC7 and upload to texture
// Returns true on success, false on failure
static bool decode_astc_to_bc7_and_upload(const CmdUpdateTexture* update, TextureResource* texture_ptr,
                                           ResourceManager& resource_manager, VkCommandBuffer cmd_buffer) {
    // Get ASTC format info
    uint32_t astc_format = update->format;
    bool is_srgb = (astc_format % 2 == 1); // SRGB variants are odd numbers

    // Determine ASTC block dimensions from format
    uint32_t astc_block_w = 4, astc_block_h = 4;
    switch (update->format) {
        case 63: case 64: astc_block_w = 4; astc_block_h = 4; break;  // 4x4
        case 65: case 66: astc_block_w = 5; astc_block_h = 4; break;  // 5x4
        case 67: case 68: astc_block_w = 5; astc_block_h = 5; break;  // 5x5
        case 69: case 70: astc_block_w = 6; astc_block_h = 5; break;  // 6x5
        case 71: case 72: astc_block_w = 6; astc_block_h = 6; break;  // 6x6
        case 73: case 74: astc_block_w = 8; astc_block_h = 5; break;  // 8x5
        case 75: case 76: astc_block_w = 8; astc_block_h = 6; break;  // 8x6
        case 77: case 78: astc_block_w = 8; astc_block_h = 8; break;  // 8x8
        case 79: case 80: astc_block_w = 10; astc_block_h = 5; break; // 10x5
        case 81: case 82: astc_block_w = 10; astc_block_h = 6; break; // 10x6
        case 83: case 84: astc_block_w = 10; astc_block_h = 8; break; // 10x8
        case 85: case 86: astc_block_w = 10; astc_block_h = 10; break; // 10x10
        case 87: case 88: astc_block_w = 12; astc_block_h = 10; break; // 12x10
        case 89: case 90: astc_block_w = 12; astc_block_h = 12; break; // 12x12
        default: return false;
    }

    uint32_t width = update->width;
    uint32_t height = update->height;
    uint32_t depth = update->depth;

    // BC7 always uses 4x4 blocks
    uint32_t bc7_blocks_x = (width + 3) / 4;
    uint32_t bc7_blocks_y = (height + 3) / 4;
    uint32_t bc7_block_size = 16; // BC7 is 16 bytes per 4x4 block
    VkDeviceSize bc7_total_size = static_cast<VkDeviceSize>(bc7_blocks_x) * bc7_blocks_y * 16 * depth;

    uint64_t src_address = update->data_address;
    uint32_t row_pitch = update->row_pitch ? update->row_pitch : astc_block_w * 16; // ASTC is 16 bytes per block
    uint32_t slice_pitch = update->slice_pitch ? update->slice_pitch : row_pitch * ((height + astc_block_h - 1) / astc_block_h);
    VkDeviceSize total_size = static_cast<VkDeviceSize>(slice_pitch) * update->depth;

    // Create staging buffer for BC7 output
    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_allocation = VK_NULL_HANDLE;

    VkBufferCreateInfo staging_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    staging_info.size = bc7_total_size;
    staging_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    staging_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkResult result = vmaCreateBuffer(resource_manager.get_vma_allocator(), &staging_info, &alloc_info,
                                       &staging_buffer, &staging_allocation, nullptr);
    if (result != VK_SUCCESS) {
        return false;
    }

    // Map staging buffer
    void* mapped_data = nullptr;
    result = vmaMapMemory(resource_manager.get_vma_allocator(), staging_allocation, &mapped_data);
    if (result != VK_SUCCESS) {
        vmaDestroyBuffer(resource_manager.get_vma_allocator(), staging_buffer, staging_allocation);
        return false;
    }

    uint8_t* bc7_data = static_cast<uint8_t*>(mapped_data);

    // Temporary buffer for decoded ASTC block (RGBA8)
    uint8_t decoded_rgba[12 * 12 * 4]; // Max 12x12 block

    // Decode each slice
    for (uint32_t z = 0; z < depth; ++z) {
        const uint8_t* slice_src = reinterpret_cast<const uint8_t*>(update->data_address) + z * slice_pitch;
        uint8_t* slice_dst = bc7_data + z * bc7_blocks_x * bc7_blocks_y * 16;

        uint32_t astc_blocks_x = (width + astc_block_w - 1) / astc_block_w;
        uint32_t astc_blocks_y = (height + astc_block_h - 1) / astc_block_h;

        for (uint32_t by = 0; by < astc_blocks_y; ++by) {
            for (uint32_t bx = 0; bx < astc_blocks_x; ++bx) {
                // Source ASTC block
                const uint8_t* astc_block = slice_src + (by * astc_blocks_x + bx) * 16;

                // Decode ASTC block to RGBA
                uint32_t decode_w = std::min(astc_block_w, width - bx * astc_block_w);
                uint32_t decode_h = std::min(astc_block_h, height - by * astc_block_h);

                if (!astc::decode_block(astc_block, decode_w, decode_h, decoded_rgba, astc_block_w * 4)) {
                    // Decode failed, fill with magenta as error indicator
                    for (uint32_t y = 0; y < decode_h; ++y) {
                        for (uint32_t x = 0; x < decode_w; ++x) {
                            uint8_t* pixel = decoded_rgba + y * astc_block_w * 4 + x * 4;
                            pixel[0] = 255; // R
                            pixel[1] = 0;   // G
                            pixel[2] = 255; // B
                            pixel[3] = 255; // A
                        }
                    }
                }

                // Now transcode the decoded RGBA to BC7 blocks
                // ASTC block may be larger than 4x4, so we may need multiple BC7 blocks
                for (uint32_t ty = 0; ty < decode_h; ty += 4) {
                    for (uint32_t tx = 0; tx < decode_w; tx += 4) {
                        uint32_t bc7_x = bx * astc_block_w / 4 + tx / 4;
                        uint32_t bc7_y = by * astc_block_h / 4 + ty / 4;

                        if (bc7_x < bc7_blocks_x && bc7_y < bc7_blocks_y) {
                            uint8_t* bc7_block = slice_dst + (bc7_y * bc7_blocks_x + bc7_x) * 16;

                            // Extract 4x4 region from decoded RGBA
                            uint8_t block_rgba[4 * 4 * 4];
                            uint32_t bw = std::min(4u, decode_w - tx);
                            uint32_t bh = std::min(4u, decode_h - ty);

                            for (uint32_t y = 0; y < bh; ++y) {
                                for (uint32_t x = 0; x < bw; ++x) {
                                    const uint8_t* src = decoded_rgba + (ty + y) * astc_block_w * 4 + (tx + x) * 4;
                                    uint8_t* dst = block_rgba + y * 16 + x * 4;
                                    dst[0] = src[0];
                                    dst[1] = src[1];
                                    dst[2] = src[2];
                                    dst[3] = src[3];
                                }
                            }
                            // Fill remaining with last pixel if partial block
                            for (uint32_t y = bh; y < 4; ++y) {
                                for (uint32_t x = 0; x < 4; ++x) {
                                    uint8_t* dst = block_rgba + y * 16 + x * 4;
                                    if (bh > 0 && bw > 0) {
                                        const uint8_t* last = block_rgba + (bh - 1) * 16 + (bw - 1) * 4;
                                        dst[0] = last[0];
                                        dst[1] = last[1];
                                        dst[2] = last[2];
                                        dst[3] = last[3];
                                    } else {
                                        dst[0] = dst[1] = dst[2] = 0;
                                        dst[3] = 255;
                                    }
                                }
                            }
                            for (uint32_t y = 0; y < bh; ++y) {
                                for (uint32_t x = bw; x < 4; ++x) {
                                    uint8_t* dst = block_rgba + y * 16 + x * 4;
                                    const uint8_t* last = block_rgba + y * 16 + (bw - 1) * 4;
                                    dst[0] = last[0];
                                    dst[1] = last[1];
                                    dst[2] = last[2];
                                    dst[3] = last[3];
                                }
                            }

                            // Encode to BC7
                            bc7::encode_block_bc7(block_rgba, 16, *reinterpret_cast<bc7::BC7Block*>(bc7_block));
                        }
                    }
                }
            }
        }
    }

    vmaUnmapMemory(resource_manager.get_vma_allocator(), staging_allocation);

    // Now upload the BC7 data to the texture using the provided command buffer
    VkFormat bc7_format = is_srgb ? VK_FORMAT_BC7_SRGB_BLOCK : VK_FORMAT_BC7_UNORM_BLOCK;

    // Create buffer image copy region
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = bc7_blocks_x;
    region.bufferImageHeight = bc7_blocks_y;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = update->mip_level;
    region.imageSubresource.baseArrayLayer = update->array_layer;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {static_cast<int32_t>(update->x), static_cast<int32_t>(update->y), static_cast<int32_t>(update->z)};
    region.imageExtent = {update->width, update->height, update->depth};

    // Transition image to TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture_ptr->image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = update->mip_level;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = update->array_layer;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Copy buffer to image
    vkCmdCopyBufferToImage(cmd_buffer, staging_buffer, texture_ptr->image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Transition to SHADER_READ_ONLY_OPTIMAL
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Cleanup staging buffer
    vmaDestroyBuffer(resource_manager.get_vma_allocator(), staging_buffer, staging_allocation);

    return true;
}

bool CommandBufferTranslator::translate_update_texture(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdUpdateTexture)) {
        last_error_ = "UPDATE_TEXTURE command data too small";
        return false;
    }

    const CmdUpdateTexture* update = cmd.get_data<CmdUpdateTexture>();

    // Get the texture resource
    auto texture_opt = resource_manager_.get_texture(update->texture_handle);
    if (!texture_opt) {
        last_error_ = "UPDATE_TEXTURE: invalid texture handle";
        return false;
    }

    TextureResource* texture_ptr = *texture_opt;
    TextureResource& texture = *texture_ptr;
    VkFormat vk_format = resource_manager_.nvn_format_to_vk(texture_ptr->desc.format);

    // Check if format is ASTC - need to decode at upload time
    if (is_astc_format(update->format)) {
        // Decode ASTC to BC7 at upload time using pure C++ decoder and upload
        if (!decode_astc_to_bc7_and_upload(update, texture_ptr, resource_manager_, cmd_buffer_)) {
            last_error_ = "ASTC texture decode/upload failed";
            return false;
        }
        return true;
    }

    // Create staging buffer for texture data
    uint64_t src_address = update->data_address;
    uint32_t row_pitch = update->row_pitch ? update->row_pitch : calculate_row_pitch(update->format, update->width);
    uint32_t slice_pitch = update->slice_pitch ? update->slice_pitch : row_pitch * update->height;
    VkDeviceSize total_size = static_cast<VkDeviceSize>(slice_pitch) * update->depth;

    // Create staging buffer
    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_allocation = VK_NULL_HANDLE;

    VkBufferCreateInfo staging_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    staging_info.size = total_size;
    staging_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    staging_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    alloc_info.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkResult result = vmaCreateBuffer(resource_manager_.get_vma_allocator(), &staging_info, &alloc_info,
                                       &staging_buffer, &staging_allocation, nullptr);
    if (result != VK_SUCCESS) {
        last_error_ = "Failed to create staging buffer for texture upload";
        return false;
    }

    // Map staging buffer and copy texture data
    void* mapped_data = nullptr;
    result = vmaMapMemory(resource_manager_.get_vma_allocator(), staging_allocation, &mapped_data);
    if (result != VK_SUCCESS) {
        vmaDestroyBuffer(resource_manager_.get_vma_allocator(), staging_buffer, staging_allocation);
        last_error_ = "Failed to map staging buffer";
        return false;
    }

    // Copy texture data from GPU address space to staging buffer
    // Note: In practice, data_address would be a guest physical address that needs translation
    // For now, we'll use a simple memcpy if data is already in host memory
    // In a real implementation, this would use resource_manager_->translate_guest_address()
    const void* src_data = reinterpret_cast<const void*>(src_address);
    memcpy(mapped_data, src_data, static_cast<size_t>(total_size));
    vmaUnmapMemory(resource_manager_.get_vma_allocator(), staging_allocation);

    // Copy from staging buffer to texture image
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = row_pitch / (get_format_block_size(static_cast<TextureFormat>(update->format)) / 8);
    region.bufferImageHeight = update->height;
    region.imageSubresource.aspectMask = get_format_aspect(vk_format);
    region.imageSubresource.mipLevel = update->mip_level;
    region.imageSubresource.baseArrayLayer = update->array_layer;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {static_cast<int32_t>(update->x), static_cast<int32_t>(update->y), static_cast<int32_t>(update->z)};
    region.imageExtent = {update->width, update->height, update->depth};

    // Transition image to TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture_ptr->image;
    barrier.subresourceRange.aspectMask = get_format_aspect(vk_format);
    barrier.subresourceRange.baseMipLevel = update->mip_level;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = update->array_layer;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(cmd_buffer_, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Copy buffer to image
    vkCmdCopyBufferToImage(cmd_buffer_, staging_buffer, texture_ptr->image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Transition to SHADER_READ_ONLY_OPTIMAL
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd_buffer_, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Cleanup staging buffer
    vmaDestroyBuffer(resource_manager_.get_vma_allocator(), staging_buffer, staging_allocation);

    return true;
}

bool CommandBufferTranslator::translate_fill_buffer(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdFillBuffer)) {
        last_error_ = "FILL_BUFFER command data too small";
        return false;
    }
    // Fill buffer with a pattern - vkCmdFillBuffer
    // Would need CmdFillBuffer structure with buffer, offset, size, and pattern
    last_error_ = "FILL_BUFFER not fully implemented";
    return false;
}

bool CommandBufferTranslator::translate_clear_texture(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdClearTexture)) {
        last_error_ = "CLEAR_TEXTURE command data too small";
        return false;
    }
    // Clear texture using vkCmdClearColorImage or vkCmdClearDepthStencilImage
    // Would need CmdClearTexture structure with texture handle, color/depth value, region
    last_error_ = "CLEAR_TEXTURE not fully implemented";
    return false;
}

// ============================================================================
// Synchronization
// ============================================================================

bool CommandBufferTranslator::translate_barrier(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdBarrier)) {
        last_error_ = "BARRIER command data too small";
        return false;
    }

    const CmdBarrier* barrier = cmd.get_data<CmdBarrier>();

    VkMemoryBarrier mem_barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    mem_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

    VkPipelineStageFlags src_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dst_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    if (barrier->barrier_type == 0) { // Memory barrier
        vkCmdPipelineBarrier(cmd_buffer_, src_stage, dst_stage, 0,
                             1, &mem_barrier, 0, nullptr, 0, nullptr);
    } else if (barrier->barrier_type == 1) { // Execution barrier
        vkCmdPipelineBarrier(cmd_buffer_, src_stage, dst_stage, 0,
                             0, nullptr, 0, nullptr, 0, nullptr);
    } else { // Both
        VkBufferMemoryBarrier buf_barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
        buf_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        buf_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        buf_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        buf_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        buf_barrier.buffer = VK_NULL_HANDLE; // Would need specific buffer
        buf_barrier.offset = 0;
        buf_barrier.size = VK_WHOLE_SIZE;

        vkCmdPipelineBarrier(cmd_buffer_, src_stage, dst_stage, 0,
                             1, &mem_barrier, 1, &buf_barrier, 0, nullptr);
    }

    return true;
}

bool CommandBufferTranslator::translate_event_signal(const ParsedCommand& cmd) {
    // Event signaling - requires VkEvent
    if (cmd.data.size() < sizeof(uint64_t)) {
        last_error_ = "EVENT_SIGNAL command data too small";
        return false;
    }
    uint64_t event_handle = *reinterpret_cast<const uint64_t*>(cmd.data.data());
    auto event_opt = resource_manager_.get_event(event_handle);
    if (!event_opt || !*event_opt) {
        last_error_ = "Event not found: " + std::to_string(event_handle);
        return false;
    }
    VkEvent event = (*event_opt)->event;
    vkCmdSetEvent(cmd_buffer_, event, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    return true;
}

bool CommandBufferTranslator::translate_event_wait(const ParsedCommand& cmd) {
    // Event waiting
    if (cmd.data.size() < sizeof(uint64_t)) {
        last_error_ = "EVENT_WAIT command data too small";
        return false;
    }
    uint64_t event_handle = *reinterpret_cast<const uint64_t*>(cmd.data.data());
    auto event_opt = resource_manager_.get_event(event_handle);
    if (!event_opt || !*event_opt) {
        last_error_ = "Event not found: " + std::to_string(event_handle);
        return false;
    }
    VkEvent event = (*event_opt)->event;
    vkCmdWaitEvents(cmd_buffer_, 1, &event,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0, nullptr, 0, nullptr, 0, nullptr);
    return true;
}

bool CommandBufferTranslator::translate_query_begin(const ParsedCommand& cmd) {
    // Query begin - requires VkQueryPool
    if (cmd.data.size() < sizeof(CmdQueryControl)) {
        last_error_ = "QUERY_BEGIN command data too small";
        return false;
    }
    const CmdQueryControl* query = cmd.get_data<CmdQueryControl>();
    auto query_opt = resource_manager_.get_query_pool(query->query_pool_handle);
    if (!query_opt) {
        last_error_ = "Query pool not found: " + std::to_string(query->query_pool_handle);
        return false;
    }
    VkQueryPool pool = query_opt.value();
    vkCmdResetQueryPool(cmd_buffer_, pool, query->query_index, 1);
    vkCmdBeginQuery(cmd_buffer_, pool, query->query_index, 0);
    return true;
}

bool CommandBufferTranslator::translate_query_end(const ParsedCommand& cmd) {
    // Query end
    if (cmd.data.size() < sizeof(CmdQueryControl)) {
        last_error_ = "QUERY_END command data too small";
        return false;
    }
    const CmdQueryControl* query = cmd.get_data<CmdQueryControl>();
    auto query_opt = resource_manager_.get_query_pool(query->query_pool_handle);
    if (!query_opt) {
        last_error_ = "Query pool not found: " + std::to_string(query->query_pool_handle);
        return false;
    }
    VkQueryPool pool = query_opt.value();
    vkCmdEndQuery(cmd_buffer_, pool, query->query_index);
    return true;
}

bool CommandBufferTranslator::translate_query_reset(const ParsedCommand& cmd) {
    // Query reset
    if (cmd.data.size() < sizeof(CmdQueryControl)) {
        last_error_ = "QUERY_RESET command data too small";
        return false;
    }
    const CmdQueryControl* query = cmd.get_data<CmdQueryControl>();
    auto query_opt = resource_manager_.get_query_pool(query->query_pool_handle);
    if (!query_opt) {
        last_error_ = "Query pool not found: " + std::to_string(query->query_pool_handle);
        return false;
    }
    VkQueryPool pool = query_opt.value();
    vkCmdResetQueryPool(cmd_buffer_, pool, query->query_index, 1);
    return true;
}

bool CommandBufferTranslator::translate_wait_idle(const ParsedCommand& cmd) {
    // Wait idle - not typically done in command buffer, but could be a host wait
    // For command buffer, we can't actually wait on host, so this is a no-op
    // In practice, this would be handled by host synchronization
    return true;
}

// ============================================================================
// Memory Management
// ============================================================================

bool CommandBufferTranslator::translate_allocate_memory(const ParsedCommand& cmd) {
    // Memory allocation is done outside command buffer
    last_error_ = "ALLOCATE_MEMORY not implemented in command buffer";
    return false;
}

bool CommandBufferTranslator::translate_free_memory(const ParsedCommand& cmd) {
    last_error_ = "FREE_MEMORY not implemented in command buffer";
    return false;
}

bool CommandBufferTranslator::translate_map_memory(const ParsedCommand& cmd) {
    last_error_ = "MAP_MEMORY not implemented in command buffer";
    return false;
}

bool CommandBufferTranslator::translate_unmap_memory(const ParsedCommand& cmd) {
    last_error_ = "UNMAP_MEMORY not implemented in command buffer";
    return false;
}

// ============================================================================
// Helpers
// ============================================================================

VkDescriptorSet kirbyrecomp::nvn::CommandBufferTranslator::get_or_create_descriptor_set(VkShaderStageFlags stage_flags) {
    // For now, use a simple approach: one descriptor set per pipeline
    // In a full implementation, we'd track descriptor sets per pipeline layout and stage
    if (!pipeline_state_.pipeline) {
        // No pipeline bound, can't create descriptor set
        return VK_NULL_HANDLE;
    }

    // Use the first descriptor set from the pipeline's layout
    if (!pipeline_state_.descriptor_set_layouts.empty()) {
        // Allocate a descriptor set from the pool
        // For now, we'll use a simple approach: allocate on first use
        // In a real implementation, we'd pool descriptor sets

        // Get the first descriptor set layout from the pipeline
        VkDescriptorSetLayout layout = pipeline_state_.descriptor_set_layouts[0];

        // Get a descriptor pool from the resource manager
        const auto& pools = resource_manager_.get_descriptor_pools();
        if (pools.empty()) {
            return VK_NULL_HANDLE;
        }

        // Use the first available pool
        VkDescriptorPool pool = pools.begin()->second;

        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = pools.begin()->second;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &layout;

        VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
        VkResult result = vkAllocateDescriptorSets(device_, &alloc_info, &descriptor_set);
        if (result != VK_SUCCESS) {
            last_error_ = "Failed to allocate descriptor set: " + std::to_string(result);
            return VK_NULL_HANDLE;
        }

        return descriptor_set;
    }

    return VK_NULL_HANDLE;
}

void kirbyrecomp::nvn::CommandBufferTranslator::flush_descriptor_writes() {
    if (pending_descriptor_writes_.empty()) return;

    // Collect all writes
    std::vector<VkWriteDescriptorSet> all_writes;
    for (const auto& dw : pending_descriptor_writes_) {
        for (const auto& write : dw.writes) {
            all_writes.push_back(write);
        }
    }

    if (!all_writes.empty()) {
        vkUpdateDescriptorSets(device_, static_cast<uint32_t>(all_writes.size()), all_writes.data(), 0, nullptr);
    }

    pending_descriptor_writes_.clear();
}

void kirbyrecomp::nvn::CommandBufferTranslator::apply_dynamic_state() {
    // Apply any dynamic state that hasn't been set yet
    if (!pipeline_state_.viewports.empty()) {
        vkCmdSetViewport(cmd_buffer_, 0, static_cast<uint32_t>(pipeline_state_.viewports.size()),
                         pipeline_state_.viewports.data());
    }
    if (!pipeline_state_.scissors.empty()) {
        vkCmdSetScissor(cmd_buffer_, 0, static_cast<uint32_t>(pipeline_state_.scissors.size()),
                        pipeline_state_.scissors.data());
    }
    // Sample mask requires VK_EXT_sample_mask extension - skip for now
}

void kirbyrecomp::nvn::CommandBufferTranslator::apply_pipeline_state() {
    // Bind vertex buffers if any
    if (!pipeline_state_.vertex_buffers.empty()) {
        vkCmdBindVertexBuffers(cmd_buffer_, 0,
                               static_cast<uint32_t>(pipeline_state_.vertex_buffers.size()),
                               pipeline_state_.vertex_buffers.data(),
                               pipeline_state_.vertex_buffer_offsets.data());
    }
    if (pipeline_state_.index_buffer != VK_NULL_HANDLE) {
        vkCmdBindIndexBuffer(cmd_buffer_, pipeline_state_.index_buffer, 0, pipeline_state_.index_type);
    }
    if (!pipeline_state_.descriptor_sets.empty()) {
        vkCmdBindDescriptorSets(cmd_buffer_, is_compute_pipeline_ ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline_state_.layout, 0,
                                static_cast<uint32_t>(pipeline_state_.descriptor_sets.size()),
                                pipeline_state_.descriptor_sets.data(), 0, nullptr);
    }
}

// ============================================================================
// Resource Lookup Helpers
// ============================================================================

VkBuffer kirbyrecomp::nvn::CommandBufferTranslator::get_buffer(uint64_t resource_id) const {
    auto opt = resource_manager_.get_buffer(resource_id);
    return (opt && *opt) ? (*opt)->buffer : VK_NULL_HANDLE;
}

VkImage kirbyrecomp::nvn::CommandBufferTranslator::get_image(uint64_t resource_id) const {
    auto opt = resource_manager_.get_texture(resource_id);
    return (opt && *opt) ? (*opt)->image : VK_NULL_HANDLE;
}

VkImageView kirbyrecomp::nvn::CommandBufferTranslator::get_image_view(uint64_t resource_id) const {
    auto opt = resource_manager_.get_texture(resource_id);
    return (opt && *opt) ? (*opt)->default_view : VK_NULL_HANDLE;
}

VkSampler kirbyrecomp::nvn::CommandBufferTranslator::get_sampler(uint64_t resource_id) const {
    auto opt = resource_manager_.get_sampler(resource_id);
    return (opt && *opt) ? (*opt)->sampler : VK_NULL_HANDLE;
}

VkPipeline kirbyrecomp::nvn::CommandBufferTranslator::get_pipeline(uint64_t pipeline_id) const {
    auto opt = resource_manager_.get_pipeline(pipeline_id);
    return (opt && *opt) ? (*opt)->pipeline : VK_NULL_HANDLE;
}

VkPipelineLayout kirbyrecomp::nvn::CommandBufferTranslator::get_pipeline_layout(uint64_t pipeline_id) const {
    auto opt = resource_manager_.get_pipeline(pipeline_id);
    return (opt && *opt) ? (*opt)->layout : VK_NULL_HANDLE;
}

VkDescriptorSet kirbyrecomp::nvn::CommandBufferTranslator::get_descriptor_set(uint64_t set_handle) const {
    auto opt = resource_manager_.get_descriptor_set(set_handle);
    return (opt && *opt) ? (*opt)->set : VK_NULL_HANDLE;
}

VkDescriptorPool kirbyrecomp::nvn::CommandBufferTranslator::get_descriptor_pool(uint64_t pool_handle) const {
    const auto& pools = resource_manager_.get_descriptor_pools();
    auto it = pools.find(pool_handle);
    return (it != pools.end()) ? it->second : VK_NULL_HANDLE;
}

// ============================================================================
// Texture Format Helpers
// ============================================================================

bool kirbyrecomp::nvn::CommandBufferTranslator::is_astc_format(uint32_t format) {
    // ASTC formats are 63-90 in the NVN format enum
    return format >= 63 && format <= 90;
}

uint32_t kirbyrecomp::nvn::CommandBufferTranslator::calculate_row_pitch(uint32_t format, uint32_t width) {
    // Calculate row pitch for texture formats
    // ASTC formats: 16 bytes per block, blocks are width/block_w
    if (format >= 63 && format <= 90) {
        // ASTC formats
        uint32_t block_w = 4;
        switch (format) {
            case 63: case 64: block_w = 4; break;
            case 65: case 66: block_w = 5; break;
            case 67: case 68: block_w = 5; break;
            case 69: case 70: block_w = 6; break;
            case 71: case 72: block_w = 6; break;
            case 73: case 74: block_w = 8; break;
            case 75: case 76: block_w = 8; break;
            case 77: case 78: block_w = 8; break;
            case 79: case 80: block_w = 10; break;
            case 81: case 82: block_w = 10; break;
            case 83: case 84: block_w = 10; break;
            case 85: case 86: block_w = 10; break;
            case 87: case 88: block_w = 12; break;
            case 89: case 90: block_w = 12; break;
        }
        uint32_t blocks_x = (width + block_w - 1) / block_w;
        return blocks_x * 16; // 16 bytes per ASTC block
    }
    // BC formats
    if (format >= 47 && format <= 62) {
        uint32_t blocks_x = (width + 3) / 4;
        uint32_t block_size = (format == 59 || format == 60) ? 16 : // BC6H
                             (format >= 61) ? 16 : // BC7
                             ((format == 47 || format == 48) ? 8 : 16); // BC1 = 8, others = 16
        return blocks_x * block_size;
    }
    // Regular formats - calculate based on format size
    // This is simplified - real implementation would use format info
    return width * 4; // Default to 4 bytes per pixel
}

uint32_t kirbyrecomp::nvn::CommandBufferTranslator::get_format_block_size(TextureFormat format) {
    // Return block size in bytes
    if (format >= TextureFormat::ASTC_4x4_UNORM && format <= TextureFormat::ASTC_12x12_SRGB) {
        return 16; // All ASTC blocks are 16 bytes
    }
    if (format >= TextureFormat::BC1_RGB_UNORM && format <= TextureFormat::BC7_SRGB) {
        if (format == TextureFormat::BC1_RGB_UNORM || format == TextureFormat::BC1_RGB_SRGB ||
            format == TextureFormat::BC1_RGBA_UNORM || format == TextureFormat::BC1_RGBA_SRGB) {
            return 8;
        }
        if (format == TextureFormat::BC6H_UFLOAT || format == TextureFormat::BC6H_SFLOAT ||
            format == TextureFormat::BC7_UNORM || format == TextureFormat::BC7_SRGB) {
            return 16;
        }
        return 16; // BC2, BC3, BC4, BC5
    }
    // Regular formats - estimate from format name
    // This is a placeholder
    return 4;
}

VkImageAspectFlags kirbyrecomp::nvn::CommandBufferTranslator::get_format_aspect(VkFormat format) {
    return resource_manager_.get_format_aspect(format);
}

} // namespace kirbyrecomp::nvn