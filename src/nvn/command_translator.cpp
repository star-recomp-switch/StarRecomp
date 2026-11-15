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
    // Constant buffers are bound as uniform buffers in descriptor sets
    return translate_bind_shader_resource(cmd);
}

bool CommandBufferTranslator::translate_bind_shader_resource(const ParsedCommand& cmd) {
    if (cmd.data.size() < sizeof(CmdBindShaderResource)) {
        last_error_ = "BIND_SHADER_RESOURCE command data too small";
        return false;
    }

    const CmdBindShaderResource* bind = cmd.get_data<CmdBindShaderResource>();

    // Find or create descriptor set for the pipeline layout
    // This is simplified - in practice we'd need proper descriptor set management

    // For now, just validate the resource exists based on stage
    switch (bind->stage) {
        case 0: // Vertex
        case 1: // Pixel/Fragment
        case 2: // Geometry
        case 3: // Hull
        case 4: // Domain
        case 5: // Compute
            // Resource validation would go here
            break;
    }

    // Queue descriptor write for later flush
    // (Implementation would need proper descriptor set layout binding info)

    return true;
}

bool CommandBufferTranslator::translate_bind_unordered_access(const ParsedCommand& cmd) {
    // UAV binding - similar to shader resource but with write access
    return translate_bind_shader_resource(cmd);
}

bool CommandBufferTranslator::translate_bind_sampler(const ParsedCommand& cmd) {
    // Sampler binding - handled as part of descriptor sets
    return translate_bind_shader_resource(cmd);
}

bool CommandBufferTranslator::translate_bind_shader(const ParsedCommand& cmd, VkShaderStageFlagBits stage) {
    if (cmd.data.size() < sizeof(uint64_t)) {
        last_error_ = "BIND_SHADER command data too small";
        return false;
    }

    uint64_t shader_handle = *reinterpret_cast<const uint64_t*>(cmd.data.data());

    // Shader binding is typically done through pipeline creation
    // This command might be for dynamic shader switching
    last_error_ = "Dynamic shader binding not fully implemented";
    return false;
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
    // Blend state is typically part of pipeline
    // Dynamic blend constants can be set
    last_error_ = "SET_BLEND_STATE not fully implemented (use pipeline variants)";
    return false;
}

bool CommandBufferTranslator::translate_set_depth_stencil_state(const ParsedCommand& cmd) {
    // Depth/stencil state is typically part of pipeline
    // Dynamic depth bias, stencil reference can be set
    last_error_ = "SET_DEPTH_STENCIL_STATE not fully implemented (use pipeline variants)";
    return false;
}

bool CommandBufferTranslator::translate_set_rasterizer_state(const ParsedCommand& cmd) {
    // Rasterizer state is typically part of pipeline
    // Dynamic line width can be set
    last_error_ = "SET_RASTERIZER_STATE not fully implemented (use pipeline variants)";
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
    // Texture copy - would need proper image copy info
    last_error_ = "COPY_TEXTURE not fully implemented";
    return false;
}

bool CommandBufferTranslator::translate_copy_texture_region(const ParsedCommand& cmd) {
    last_error_ = "COPY_TEXTURE_REGION not fully implemented";
    return false;
}

bool CommandBufferTranslator::translate_copy_buffer_to_texture(const ParsedCommand& cmd) {
    last_error_ = "COPY_BUFFER_TO_TEXTURE not fully implemented";
    return false;
}

bool CommandBufferTranslator::translate_copy_texture_to_buffer(const ParsedCommand& cmd) {
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

bool CommandBufferTranslator::translate_update_texture(const ParsedCommand& cmd) {
    last_error_ = "UPDATE_TEXTURE not fully implemented";
    return false;
}

bool CommandBufferTranslator::translate_fill_buffer(const ParsedCommand& cmd) {
    // Fill buffer with a pattern
    last_error_ = "FILL_BUFFER not implemented";
    return false;
}

bool CommandBufferTranslator::translate_clear_texture(const ParsedCommand& cmd) {
    // Clear texture - could use vkCmdClearColorImage
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
    last_error_ = "EVENT_SIGNAL not fully implemented";
    return false;
}

bool CommandBufferTranslator::translate_event_wait(const ParsedCommand& cmd) {
    // Event waiting
    last_error_ = "EVENT_WAIT not fully implemented";
    return false;
}

bool CommandBufferTranslator::translate_query_begin(const ParsedCommand& cmd) {
    last_error_ = "QUERY_BEGIN not implemented";
    return false;
}

bool CommandBufferTranslator::translate_query_end(const ParsedCommand& cmd) {
    last_error_ = "QUERY_END not implemented";
    return false;
}

bool CommandBufferTranslator::translate_query_reset(const ParsedCommand& cmd) {
    last_error_ = "QUERY_RESET not implemented";
    return false;
}

bool CommandBufferTranslator::translate_wait_idle(const ParsedCommand& cmd) {
    // Wait idle - not typically done in command buffer
    last_error_ = "WAIT_IDLE not implemented in command buffer";
    return false;
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

void CommandBufferTranslator::flush_descriptor_writes() {
    for (const auto& write : pending_descriptor_writes_) {
        // Would need to get the descriptor set from resource manager
        // vkUpdateDescriptorSets(device_, ...);
    }
    pending_descriptor_writes_.clear();
}

void CommandBufferTranslator::apply_dynamic_state() {
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

void CommandBufferTranslator::apply_pipeline_state() {
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

VkBuffer CommandBufferTranslator::get_buffer(uint64_t resource_id) const {
    auto opt = resource_manager_.get_buffer(resource_id);
    return (opt && *opt) ? (*opt)->buffer : VK_NULL_HANDLE;
}

VkImage CommandBufferTranslator::get_image(uint64_t resource_id) const {
    auto opt = resource_manager_.get_texture(resource_id);
    return (opt && *opt) ? (*opt)->image : VK_NULL_HANDLE;
}

VkImageView CommandBufferTranslator::get_image_view(uint64_t resource_id) const {
    auto opt = resource_manager_.get_texture(resource_id);
    return (opt && *opt) ? (*opt)->default_view : VK_NULL_HANDLE;
}

VkSampler CommandBufferTranslator::get_sampler(uint64_t resource_id) const {
    auto opt = resource_manager_.get_sampler(resource_id);
    return (opt && *opt) ? (*opt)->sampler : VK_NULL_HANDLE;
}

VkPipeline CommandBufferTranslator::get_pipeline(uint64_t pipeline_id) const {
    auto opt = resource_manager_.get_pipeline(pipeline_id);
    return (opt && *opt) ? (*opt)->pipeline : VK_NULL_HANDLE;
}

VkPipelineLayout CommandBufferTranslator::get_pipeline_layout(uint64_t pipeline_id) const {
    auto opt = resource_manager_.get_pipeline(pipeline_id);
    return (opt && *opt) ? (*opt)->layout : VK_NULL_HANDLE;
}

VkDescriptorSet CommandBufferTranslator::get_descriptor_set(uint64_t set_handle) const {
    auto opt = resource_manager_.get_descriptor_set(set_handle);
    return (opt && *opt) ? (*opt)->set : VK_NULL_HANDLE;
}

VkDescriptorPool CommandBufferTranslator::get_descriptor_pool(uint64_t pool_handle) const {
    const auto& pools = resource_manager_.get_descriptor_pools();
    auto it = pools.find(pool_handle);
    return (it != pools.end()) ? it->second : VK_NULL_HANDLE;
}

} // namespace kirbyrecomp::nvn