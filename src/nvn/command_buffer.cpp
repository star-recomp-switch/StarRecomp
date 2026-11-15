// NVN Command Buffer Parser Implementation

#include "kirbyrecomp/nvn/command_buffer.h"

#include <cstring>
#include <algorithm>
#include <iostream>

namespace kirbyrecomp::nvn {

// ============================================================================
// CommandBufferParser Implementation
// ============================================================================

std::optional<std::vector<ParsedCommand>> CommandBufferParser::parse(std::span<const uint8_t> buffer) {
    last_error_.clear();

    if (buffer.size() < sizeof(MaxwellCommandHeader)) {
        last_error_ = "Buffer too small for command header";
        return std::nullopt;
    }

    std::vector<ParsedCommand> commands;
    uint64_t offset = 0;

    while (offset + sizeof(MaxwellCommandHeader) <= buffer.size()) {
        auto cmd_opt = parse_command(buffer, offset);
        if (!cmd_opt) {
            return std::nullopt;
        }

        const auto& cmd = *cmd_opt;
        commands.push_back(cmd);

        if (cmd.opcode == MaxwellCmdOpcode::END) {
            break;
        }

        offset += cmd.size;
    }

    return commands;
}

std::optional<ParsedCommand> CommandBufferParser::parse_command(std::span<const uint8_t> buffer, uint64_t offset) {
    last_error_.clear();

    if (offset + sizeof(MaxwellCommandHeader) > buffer.size()) {
        last_error_ = "Buffer too small for command header at offset " + std::to_string(offset);
        return std::nullopt;
    }

    const MaxwellCommandHeader* header = reinterpret_cast<const MaxwellCommandHeader*>(buffer.data() + offset);

    ParsedCommand cmd;
    cmd.opcode = static_cast<MaxwellCmdOpcode>(header->opcode);
    cmd.size = header->size;
    cmd.offset = offset;

    if (offset + cmd.size > buffer.size()) {
        last_error_ = "Command extends beyond buffer at offset " + std::to_string(offset);
        return std::nullopt;
    }

    // Copy command data (excluding header)
    if (cmd.size > sizeof(MaxwellCommandHeader)) {
        cmd.data.assign(buffer.data() + offset + sizeof(MaxwellCommandHeader),
                       buffer.data() + offset + cmd.size);
    }

    // Parse command-specific data
    bool success = true;
    switch (cmd.opcode) {
        case MaxwellCmdOpcode::DRAW_ARRAYS:
        case MaxwellCmdOpcode::DRAW_ARRAYS_INSTANCED:
            success = parse_draw_arrays(cmd.data.data(), static_cast<uint32_t>(cmd.data.size()), cmd);
            break;
        case MaxwellCmdOpcode::DRAW_ELEMENTS:
        case MaxwellCmdOpcode::DRAW_ELEMENTS_INSTANCED:
            success = parse_draw_elements(cmd.data.data(), static_cast<uint32_t>(cmd.data.size()), cmd);
            break;
        case MaxwellCmdOpcode::DISPATCH:
        case MaxwellCmdOpcode::DISPATCH_INDIRECT:
            success = parse_dispatch(cmd.data.data(), static_cast<uint32_t>(cmd.data.size()), cmd);
            break;
        case MaxwellCmdOpcode::BIND_VERTEX_BUFFER:
            success = parse_bind_vertex_buffer(cmd.data.data(), static_cast<uint32_t>(cmd.data.size()), cmd);
            break;
        case MaxwellCmdOpcode::BIND_INDEX_BUFFER:
            success = parse_bind_index_buffer(cmd.data.data(), static_cast<uint32_t>(cmd.data.size()), cmd);
            break;
        case MaxwellCmdOpcode::BIND_SHADER_RESOURCE:
        case MaxwellCmdOpcode::BIND_CONSTANT_BUFFER:
        case MaxwellCmdOpcode::BIND_UNORDERED_ACCESS:
        case MaxwellCmdOpcode::BIND_SAMPLER:
            success = parse_bind_shader_resource(cmd.data.data(), static_cast<uint32_t>(cmd.data.size()), cmd);
            break;
        case MaxwellCmdOpcode::SET_VIEWPORT:
            success = parse_set_viewport(cmd.data.data(), static_cast<uint32_t>(cmd.data.size()), cmd);
            break;
        case MaxwellCmdOpcode::SET_SCISSOR:
            success = parse_set_scissor(cmd.data.data(), static_cast<uint32_t>(cmd.data.size()), cmd);
            break;
        case MaxwellCmdOpcode::BARRIER:
            success = parse_barrier(cmd.data.data(), static_cast<uint32_t>(cmd.data.size()), cmd);
            break;
        case MaxwellCmdOpcode::COPY_BUFFER:
        case MaxwellCmdOpcode::COPY_BUFFER_REGION:
            success = parse_copy_buffer(cmd.data.data(), static_cast<uint32_t>(cmd.data.size()), cmd);
            break;
        case MaxwellCmdOpcode::COPY_TEXTURE:
        case MaxwellCmdOpcode::COPY_TEXTURE_REGION:
        case MaxwellCmdOpcode::COPY_BUFFER_TO_TEXTURE:
        case MaxwellCmdOpcode::COPY_TEXTURE_TO_BUFFER:
            success = parse_copy_texture(cmd.data.data(), static_cast<uint32_t>(cmd.data.size()), cmd);
            break;
        default:
            // Unknown command - keep raw data
            break;
    }

    if (!success) {
        return std::nullopt;
    }

    return cmd;
}

// ============================================================================
// Command-Specific Parsers
// ============================================================================

bool CommandBufferParser::parse_draw_arrays(const uint8_t* data, uint32_t size, ParsedCommand& cmd) {
    if (size < sizeof(CmdDrawArrays)) {
        last_error_ = "DRAW_ARRAYS command too small";
        return false;
    }
    // Data is already in cmd.data
    return true;
}

bool CommandBufferParser::parse_draw_elements(const uint8_t* data, uint32_t size, ParsedCommand& cmd) {
    if (size < sizeof(CmdDrawElements)) {
        last_error_ = "DRAW_ELEMENTS command too small";
        return false;
    }
    return true;
}

bool CommandBufferParser::parse_dispatch(const uint8_t* data, uint32_t size, ParsedCommand& cmd) {
    if (size < sizeof(CmdDispatch)) {
        last_error_ = "DISPATCH command too small";
        return false;
    }
    return true;
}

bool CommandBufferParser::parse_bind_vertex_buffer(const uint8_t* data, uint32_t size, ParsedCommand& cmd) {
    if (size < sizeof(CmdBindVertexBuffer)) {
        last_error_ = "BIND_VERTEX_BUFFER command too small";
        return false;
    }
    return true;
}

bool CommandBufferParser::parse_bind_index_buffer(const uint8_t* data, uint32_t size, ParsedCommand& cmd) {
    if (size < sizeof(CmdBindIndexBuffer)) {
        last_error_ = "BIND_INDEX_BUFFER command too small";
        return false;
    }
    return true;
}

bool CommandBufferParser::parse_bind_shader_resource(const uint8_t* data, uint32_t size, ParsedCommand& cmd) {
    if (size < sizeof(CmdBindShaderResource)) {
        last_error_ = "BIND_SHADER_RESOURCE command too small";
        return false;
    }
    return true;
}

bool CommandBufferParser::parse_set_viewport(const uint8_t* data, uint32_t size, ParsedCommand& cmd) {
    if (size < sizeof(CmdSetViewport)) {
        last_error_ = "SET_VIEWPORT command too small";
        return false;
    }
    return true;
}

bool CommandBufferParser::parse_set_scissor(const uint8_t* data, uint32_t size, ParsedCommand& cmd) {
    if (size < sizeof(CmdSetScissor)) {
        last_error_ = "SET_SCISSOR command too small";
        return false;
    }
    return true;
}

bool CommandBufferParser::parse_barrier(const uint8_t* data, uint32_t size, ParsedCommand& cmd) {
    if (size < sizeof(CmdBarrier)) {
        last_error_ = "BARRIER command too small";
        return false;
    }
    return true;
}

bool CommandBufferParser::parse_copy_buffer(const uint8_t* data, uint32_t size, ParsedCommand& cmd) {
    if (size < sizeof(CmdCopyBuffer)) {
        last_error_ = "COPY_BUFFER command too small";
        return false;
    }
    return true;
}

bool CommandBufferParser::parse_copy_texture(const uint8_t* data, uint32_t size, ParsedCommand& cmd) {
    // Texture copy commands have variable size
    return true;
}

} // namespace kirbyrecomp::nvn