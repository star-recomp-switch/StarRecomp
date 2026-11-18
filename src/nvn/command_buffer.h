#pragma once

// NVN Command Buffer Parser
// Parses Maxwell FIFO command format and translates to Vulkan commands

#include <cstdint>
#include <vector>
#include <span>
#include <optional>
#include <unordered_map>
#include <string>
#include <vulkan/vulkan.h>

namespace kirbyrecomp::nvn {

// ============================================================================
// Maxwell FIFO Command Format
// Based on NVIDIA Maxwell command buffer format and envytools
// ============================================================================

// Command header (8 bytes)
#pragma pack(push, 1)
struct MaxwellCommandHeader {
    uint32_t opcode;      // Command opcode
    uint32_t size;        // Command size in bytes (including header)
};
#pragma pack(pop)

// Common command opcodes (Maxwell class 0x906F / 0x90C0)
enum class MaxwellCmdOpcode : uint32_t {
    // State setup
    SET_OBJECT                  = 0x0000,
    SET_REFERENCE               = 0x0001,
    SET_CONTEXT_DMA             = 0x0002,
    SET_OFFSET                  = 0x0003,
    SET_PITCH                   = 0x0004,
    SET_BASE                    = 0x0005,
    SET_LIMIT                   = 0x0006,
    SET_FORMAT                  = 0x0007,
    SET_SWIZZLE                 = 0x0008,

    // Draw commands
    DRAW_ARRAYS                 = 0x0100,
    DRAW_ARRAYS_INSTANCED       = 0x0101,
    DRAW_ELEMENTS               = 0x0102,
    DRAW_ELEMENTS_INSTANCED     = 0x0103,
    DRAW_TRANSFORM_FEEDBACK     = 0x0104,
    DRAW_TRANSFORM_FEEDBACK_INST = 0x0105,
    DRAW_AUTO                   = 0x0106,
    MULTI_DRAW_ARRAYS           = 0x0107,
    MULTI_DRAW_ELEMENTS         = 0x0108,

    // Compute dispatch
    DISPATCH                    = 0x0110,
    DISPATCH_INDIRECT           = 0x0111,

    // Resource binding
    BIND_VERTEX_BUFFER          = 0x0200,
    BIND_INDEX_BUFFER           = 0x0201,
    BIND_CONSTANT_BUFFER        = 0x0202,
    BIND_SHADER_RESOURCE        = 0x0203,
    BIND_UNORDERED_ACCESS       = 0x0204,
    BIND_SAMPLER                = 0x0205,
    BIND_VERTEX_SHADER          = 0x0206,
    BIND_PIXEL_SHADER           = 0x0207,
    BIND_GEOMETRY_SHADER        = 0x0208,
    BIND_HULL_SHADER            = 0x0209,
    BIND_DOMAIN_SHADER          = 0x020A,
    BIND_COMPUTE_SHADER         = 0x020B,

    // Pipeline state
    SET_VIEWPORT                = 0x0300,
    SET_SCISSOR                 = 0x0301,
    SET_BLEND_STATE             = 0x0302,
    SET_DEPTH_STENCIL_STATE     = 0x0303,
    SET_RASTERIZER_STATE        = 0x0304,
    SET_SAMPLE_MASK             = 0x0305,
    SET_PRIMITIVE_TOPOLOGY      = 0x0306,
    SET_VERTEX_LAYOUT           = 0x0307,

    // Texture/Sampler
    SET_TEXTURE                 = 0x0400,
    SET_SAMPLER                 = 0x0401,
    SET_TEXTURE_VIEW            = 0x0402,

    // Buffer operations
    COPY_BUFFER                 = 0x0500,
    COPY_BUFFER_REGION          = 0x0501,
    COPY_TEXTURE                = 0x0502,
    COPY_TEXTURE_REGION         = 0x0503,
    COPY_BUFFER_TO_TEXTURE      = 0x0504,
    COPY_TEXTURE_TO_BUFFER      = 0x0505,
    UPDATE_BUFFER               = 0x0506,
    UPDATE_TEXTURE              = 0x0507,
    FILL_BUFFER                 = 0x0508,
    CLEAR_TEXTURE               = 0x0509,

    // Synchronization
    BARRIER                     = 0x0600,
    EVENT_SIGNAL                = 0x0601,
    EVENT_WAIT                  = 0x0602,
    QUERY_BEGIN                 = 0x0603,
    QUERY_END                   = 0x0604,
    QUERY_RESET                 = 0x0605,
    WAIT_IDLE                   = 0x0606,

    // Memory management
    ALLOCATE_MEMORY             = 0x0700,
    FREE_MEMORY                 = 0x0701,
    MAP_MEMORY                  = 0x0702,
    UNMAP_MEMORY                = 0x0703,

    // Debug
    INSERT_MARKER               = 0x0800,
    PUSH_DEBUG_GROUP            = 0x0801,
    POP_DEBUG_GROUP             = 0x0802,

    // End of command buffer
    END                         = 0xFFFFFFFF,
};

// ============================================================================
// Parsed Command Representation
// ============================================================================

struct ParsedCommand {
    MaxwellCmdOpcode opcode;
    uint32_t size;
    std::vector<uint8_t> data;  // Raw command data (excluding header)
    uint64_t offset;            // Offset in command buffer

    // Helper to get typed data
    template<typename T>
    const T* get_data() const {
        return reinterpret_cast<const T*>(data.data());
    }
};

// ============================================================================
// Command Buffer Parser
// ============================================================================

class CommandBufferParser {
public:
    CommandBufferParser() = default;
    ~CommandBufferParser() = default;

    // Parse a command buffer
    std::optional<std::vector<ParsedCommand>> parse(std::span<const uint8_t> buffer);

    // Parse a single command at offset
    std::optional<ParsedCommand> parse_command(std::span<const uint8_t> buffer, uint64_t offset);

    // Get last error
    const std::string& get_last_error() const { return last_error_; }

private:
    std::string last_error_;

    // Command-specific parsers
    bool parse_draw_arrays(const uint8_t* data, uint32_t size, ParsedCommand& cmd);
    bool parse_draw_elements(const uint8_t* data, uint32_t size, ParsedCommand& cmd);
    bool parse_dispatch(const uint8_t* data, uint32_t size, ParsedCommand& cmd);
    bool parse_bind_vertex_buffer(const uint8_t* data, uint32_t size, ParsedCommand& cmd);
    bool parse_bind_index_buffer(const uint8_t* data, uint32_t size, ParsedCommand& cmd);
    bool parse_bind_shader_resource(const uint8_t* data, uint32_t size, ParsedCommand& cmd);
    bool parse_set_viewport(const uint8_t* data, uint32_t size, ParsedCommand& cmd);
    bool parse_set_scissor(const uint8_t* data, uint32_t size, ParsedCommand& cmd);
    bool parse_barrier(const uint8_t* data, uint32_t size, ParsedCommand& cmd);
    bool parse_copy_buffer(const uint8_t* data, uint32_t size, ParsedCommand& cmd);
    bool parse_copy_texture(const uint8_t* data, uint32_t size, ParsedCommand& cmd);
};

// ============================================================================
// Command Structures (matching Maxwell FIFO format)
// ============================================================================

#pragma pack(push, 1)
struct CmdDrawArrays {
    uint32_t vertex_count;
    uint32_t instance_count;
    uint32_t first_vertex;
    uint32_t first_instance;
};

struct CmdDrawElements {
    uint32_t index_count;
    uint32_t instance_count;
    uint32_t first_index;
    int32_t vertex_offset;
    uint32_t first_instance;
};

struct CmdDispatch {
    uint32_t group_count_x;
    uint32_t group_count_y;
    uint32_t group_count_z;
};

struct CmdBindVertexBuffer {
    uint32_t slot;
    uint64_t buffer_address;
    uint32_t stride;
    uint32_t offset;
};

struct CmdBindIndexBuffer {
    uint64_t buffer_address;
    uint32_t index_type;  // 0 = 16-bit, 1 = 32-bit
    uint32_t offset;
};

struct CmdBindShaderResource {
    uint32_t stage;       // Shader stage bitmask
    uint32_t slot;
    uint64_t resource_id;
};

struct CmdSetViewport {
    float x, y, width, height;
    float min_depth, max_depth;
};

struct CmdSetScissor {
    int32_t x, y;
    uint32_t width, height;
};

struct CmdBarrier {
    uint32_t barrier_type;  // 0 = memory, 1 = execution, 2 = both
    uint32_t flags;
};

struct CmdCopyBuffer {
    uint64_t src_address;
    uint64_t dst_address;
    uint32_t size;
};

struct CmdCopyBufferRegion {
    uint64_t src_address;
    uint64_t dst_address;
    uint32_t src_offset;
    uint32_t dst_offset;
    uint32_t size;
};

struct CmdUpdateBuffer {
    uint64_t dst_address;
    uint32_t dst_offset;
    uint32_t size;
    // Data follows
};

#pragma pack(push, 1)
struct CmdUpdateTexture {
    uint64_t texture_handle;
    uint32_t mip_level;
    uint32_t array_layer;
    uint32_t x, y, z;          // Destination offset
    uint32_t width, height, depth;  // Region size
    uint32_t format;           // TextureFormat
    uint32_t row_pitch;        // Bytes per row
    uint32_t slice_pitch;      // Bytes per slice
    uint64_t data_address;     // Source data address
};
#pragma pack(pop)

// Command Buffer Translator is defined in command_translator.h
// Forward declaration
class ResourceManager;
class CommandBufferTranslator;

} // namespace kirbyrecomp::nvn