#pragma once

// NVN Resource Manager
// Manages Vulkan resources (buffers, images, samplers, pipelines) for NVN translation

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <optional>
#include <string>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

// Include shader reflection types (needed for PipelineDesc, ShaderModuleResource)
#include "kirbyrecomp/shader/shader.h"

namespace kirbyrecomp::nvn {

// ============================================================================
// Resource Handles
// ============================================================================

// NVN resource types
enum class NvnResourceType : uint32_t {
    Buffer          = 0,
    Texture         = 1,
    Sampler         = 2,
    ShaderModule    = 3,
    Pipeline        = 4,
    PipelineLayout  = 5,
    DescriptorSet   = 6,
    DescriptorPool  = 7,
    QueryPool       = 8,
    Fence           = 9,
    Semaphore       = 10,
    Event           = 11,
};

// Buffer usage flags (matching NVN)
enum class BufferUsage : uint32_t {
    None                    = 0,
    VertexBuffer            = 1 << 0,
    IndexBuffer             = 1 << 1,
    ConstantBuffer          = 1 << 2,
    ShaderResource          = 1 << 3,
    UnorderedAccess         = 1 << 4,
    IndirectArgs            = 1 << 5,
    CopySrc                 = 1 << 6,
    CopyDst                 = 1 << 7,
    TransformFeedback       = 1 << 8,
};

// Texture usage flags
enum class TextureUsage : uint32_t {
    None                    = 0,
    ShaderResource          = 1 << 0,
    RenderTarget            = 1 << 1,
    DepthStencil            = 1 << 2,
    UnorderedAccess         = 1 << 3,
    CopySrc                 = 1 << 4,
    CopyDst                 = 1 << 5,
    ResolveSrc              = 1 << 6,
    ResolveDst              = 1 << 7,
    Present                 = 1 << 8,
};

// Texture formats (matching NVN)
enum class TextureFormat : uint32_t {
    Unknown                 = 0,
    R8_UNORM                = 1,
    R8_SNORM                = 2,
    R8_UINT                 = 3,
    R8_SINT                 = 4,
    R16_UNORM               = 5,
    R16_SNORM               = 6,
    R16_UINT                = 7,
    R16_SINT                = 8,
    R16_SFLOAT              = 9,
    RG8_UNORM               = 10,
    RG8_SNORM               = 11,
    RG8_UINT                = 12,
    RG8_SINT                = 13,
    R32_UINT                = 14,
    R32_SINT                = 15,
    R32_SFLOAT              = 16,
    RG16_UNORM              = 17,
    RG16_SNORM              = 18,
    RG16_UINT               = 19,
    RG16_SINT               = 20,
    RG16_SFLOAT             = 21,
    RGBA8_UNORM             = 22,
    RGBA8_SNORM             = 23,
    RGBA8_UINT              = 24,
    RGBA8_SINT              = 25,
    RGBA8_SRGB              = 26,
    BGRA8_UNORM             = 27,
    BGRA8_SRGB              = 28,
    RGB10A2_UNORM           = 29,
    RG11B10_UFLOAT          = 30,
    RGB9E5_UFLOAT           = 31,
    RG32_UINT               = 32,
    RG32_SINT               = 33,
    RG32_SFLOAT             = 34,
    RGBA16_UNORM            = 35,
    RGBA16_SNORM            = 36,
    RGBA16_UINT             = 37,
    RGBA16_SINT             = 38,
    RGBA16_SFLOAT           = 39,
    RGBA32_UINT             = 40,
    RGBA32_SINT             = 41,
    RGBA32_SFLOAT           = 42,
    D16_UNORM               = 43,
    D24_UNORM_S8_UINT       = 44,
    D32_SFLOAT              = 45,
    D32_SFLOAT_S8_UINT      = 46,
    BC1_RGB_UNORM           = 47,
    BC1_RGB_SRGB            = 48,
    BC1_RGBA_UNORM          = 49,
    BC1_RGBA_SRGB           = 50,
    BC2_UNORM               = 51,
    BC2_SRGB                = 52,
    BC3_UNORM               = 53,
    BC3_SRGB                = 54,
    BC4_UNORM               = 55,
    BC4_SNORM               = 56,
    BC5_UNORM               = 57,
    BC5_SNORM               = 58,
    BC6H_UFLOAT             = 59,
    BC6H_SFLOAT             = 60,
    BC7_UNORM               = 61,
    BC7_SRGB                = 62,
    ASTC_4x4_UNORM          = 63,
    ASTC_4x4_SRGB           = 64,
    ASTC_5x4_UNORM          = 65,
    ASTC_5x4_SRGB           = 66,
    ASTC_5x5_UNORM          = 67,
    ASTC_5x5_SRGB           = 68,
    ASTC_6x5_UNORM          = 69,
    ASTC_6x5_SRGB           = 70,
    ASTC_6x6_UNORM          = 71,
    ASTC_6x6_SRGB           = 72,
    ASTC_8x5_UNORM          = 73,
    ASTC_8x5_SRGB           = 74,
    ASTC_8x6_UNORM          = 75,
    ASTC_8x6_SRGB           = 76,
    ASTC_8x8_UNORM          = 77,
    ASTC_8x8_SRGB           = 78,
    ASTC_10x5_UNORM         = 79,
    ASTC_10x5_SRGB          = 80,
    ASTC_10x6_UNORM         = 81,
    ASTC_10x6_SRGB          = 82,
    ASTC_10x8_UNORM         = 83,
    ASTC_10x8_SRGB          = 84,
    ASTC_10x10_UNORM        = 85,
    ASTC_10x10_SRGB         = 86,
    ASTC_12x10_UNORM        = 87,
    ASTC_12x10_SRGB         = 88,
    ASTC_12x12_UNORM        = 89,
    ASTC_12x12_SRGB         = 90,
};

// ============================================================================
// Resource Descriptions
// ============================================================================

struct BufferDesc {
    uint64_t size;
    BufferUsage usage;
    uint32_t stride;  // For vertex buffers
    bool persistent_map;  // Persistent mapping
};

struct TextureDesc {
    TextureFormat format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;           // 1 for 2D, >1 for 3D
    uint32_t array_layers;    // 1 for non-array, >1 for array
    uint32_t mip_levels;
    uint32_t sample_count;
    TextureUsage usage;
    uint32_t flags;           // Cube map, etc.
};

struct SamplerDesc {
    enum class Filter : uint32_t {
        Nearest = 0,
        Linear  = 1,
    };
    enum class AddressMode : uint32_t {
        Repeat         = 0,
        MirrorRepeat   = 1,
        ClampToEdge    = 2,
        ClampToBorder  = 3,
        MirrorClampToEdge = 4,
    };
    enum class CompareOp : uint32_t {
        Never         = 0,
        Less          = 1,
        Equal         = 2,
        LessOrEqual   = 3,
        Greater       = 4,
        NotEqual      = 5,
        GreaterOrEqual= 6,
        Always        = 7,
    };

    Filter min_filter = Filter::Linear;
    Filter mag_filter = Filter::Linear;
    Filter mip_filter = Filter::Linear;
    AddressMode address_u = AddressMode::Repeat;
    AddressMode address_v = AddressMode::Repeat;
    AddressMode address_w = AddressMode::Repeat;
    float mip_lod_bias = 0.0f;
    bool anisotropy_enable = false;
    float max_anisotropy = 1.0f;
    bool compare_enable = false;
    CompareOp compare_op = CompareOp::Never;
    float min_lod = 0.0f;
    float max_lod = 1000.0f;
    float border_color[4] = {0, 0, 0, 0};
};

// ============================================================================
// Pipeline Descriptions (using shader reflection)
// ============================================================================

struct VertexInputAttribute {
    uint32_t location;
    uint32_t binding;
    VkFormat format;
    uint32_t offset;
};

struct VertexInputBinding {
    uint32_t binding;
    uint32_t stride;
    VkVertexInputRate input_rate;
};

struct PipelineDesc {
    // Shader stages
    std::vector<uint64_t> shader_handles;  // Shader module handles

    // Shader reflection data for each stage
    std::vector<kirbyrecomp::shader::ShaderReflection> shader_reflections;

    // Vertex input state
    std::vector<VertexInputAttribute> vertex_attributes;
    std::vector<VertexInputBinding> vertex_bindings;

    // Input assembly
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    bool primitive_restart_enable = false;

    // Viewport/Scissor (dynamic)
    uint32_t viewport_count = 1;
    uint32_t scissor_count = 1;

    // Rasterization
    VkPolygonMode polygon_mode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cull_mode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace front_face = VK_FRONT_FACE_CLOCKWISE;
    float line_width = 1.0f;

    // Multisample
    VkSampleCountFlagBits rasterization_samples = VK_SAMPLE_COUNT_1_BIT;
    bool sample_shading_enable = false;

    // Depth/Stencil
    bool depth_test_enable = true;
    bool depth_write_enable = true;
    VkCompareOp depth_compare_op = VK_COMPARE_OP_LESS_OR_EQUAL;
    bool depth_bounds_test_enable = false;
    bool stencil_test_enable = false;

    // Color blend
    struct BlendAttachmentState {
        bool blend_enable = false;
        VkBlendFactor src_color_blend_factor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor dst_color_blend_factor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp color_blend_op = VK_BLEND_OP_ADD;
        VkBlendFactor src_alpha_blend_factor = VK_BLEND_FACTOR_ONE;
        VkBlendFactor dst_alpha_blend_factor = VK_BLEND_FACTOR_ZERO;
        VkBlendOp alpha_blend_op = VK_BLEND_OP_ADD;
        VkColorComponentFlags color_write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    };
    std::vector<BlendAttachmentState> color_blend_attachments;

    // Dynamic state
    std::vector<VkDynamicState> dynamic_states = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    // Push constant ranges (from shader reflection)
    struct PushConstantRange {
        VkShaderStageFlags stage_flags;
        uint32_t offset;
        uint32_t size;
    };
    std::vector<PushConstantRange> push_constant_ranges;
};

struct ComputePipelineDesc {
    uint64_t shader_handle;
    kirbyrecomp::shader::ShaderReflection shader_reflection;
    std::vector<PipelineDesc::PushConstantRange> push_constant_ranges;
};

// ============================================================================
// Resource Objects
// ============================================================================

struct BufferResource {
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;  // For backward compat, stores VmaAllocation
    VmaAllocation vma_allocation = VK_NULL_HANDLE;
    VkBufferView view = VK_NULL_HANDLE;
    BufferDesc desc;
    uint64_t gpu_address = 0;  // For GPU virtual addressing
    void* mapped_ptr = nullptr;
};

struct TextureResource {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;  // For backward compat, stores VmaAllocation
    VmaAllocation vma_allocation = VK_NULL_HANDLE;
    VkImageView default_view = VK_NULL_HANDLE;
    std::vector<VkImageView> mip_views;
    std::vector<VkImageView> array_views;
    TextureDesc desc;
};

struct SamplerResource {
    VkSampler sampler = VK_NULL_HANDLE;
    SamplerDesc desc;
};

struct PipelineResource {
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
};

struct DescriptorSetResource {
    VkDescriptorSet set = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
};

struct ShaderModuleResource {
    VkShaderModule module = VK_NULL_HANDLE;
    std::vector<uint32_t> spirv_binary;
    kirbyrecomp::shader::ShaderReflection reflection;
};

// ============================================================================
// Synchronization Primitives
// ============================================================================

struct FenceResource {
    VkFence fence = VK_NULL_HANDLE;
    bool signaled = false;
};

struct SemaphoreResource {
    VkSemaphore semaphore = VK_NULL_HANDLE;
    bool is_timeline = false;
    uint64_t current_value = 0;
};

struct TimelineSemaphorePool {
    std::vector<VkSemaphore> semaphores;
    std::vector<uint64_t> values;
    uint32_t next_index = 0;
};

struct EventResource {
    VkEvent event = VK_NULL_HANDLE;
    bool set = false;
};

// ============================================================================
// Resource Manager
// ============================================================================

class ResourceManager {
public:
    explicit ResourceManager(VkDevice device, VkPhysicalDevice physical_device);
    ~ResourceManager();

    // Non-copyable, movable
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = default;
    ResourceManager& operator=(ResourceManager&&) = default;

    // Initialize
    bool initialize();

    // Buffer management
    uint64_t create_buffer(const BufferDesc& desc);
    void destroy_buffer(uint64_t handle);
    std::optional<BufferResource*> get_buffer(uint64_t handle);
    std::optional<const BufferResource*> get_buffer(uint64_t handle) const;

    // Buffer mapping
    void* map_buffer(uint64_t handle);
    void unmap_buffer(uint64_t handle);
    uint64_t get_buffer_gpu_address(uint64_t handle);

    // Texture management
    uint64_t create_texture(const TextureDesc& desc);
    void destroy_texture(uint64_t handle);
    std::optional<TextureResource*> get_texture(uint64_t handle);
    std::optional<const TextureResource*> get_texture(uint64_t handle) const;

    // Sampler management
    uint64_t create_sampler(const SamplerDesc& desc);
    void destroy_sampler(uint64_t handle);
    std::optional<SamplerResource*> get_sampler(uint64_t handle);
    std::optional<const SamplerResource*> get_sampler(uint64_t handle) const;

    // Shader module management
    uint64_t create_shader_module(const std::vector<uint32_t>& spirv_binary,
                                  const kirbyrecomp::shader::ShaderReflection& reflection);
    void destroy_shader_module(uint64_t handle);
    std::optional<ShaderModuleResource*> get_shader_module(uint64_t handle);
    std::optional<const ShaderModuleResource*> get_shader_module(uint64_t handle) const;

    // Pipeline management
    uint64_t create_graphics_pipeline(const PipelineDesc& desc);
    uint64_t create_compute_pipeline(const ComputePipelineDesc& desc);
    void destroy_pipeline(uint64_t handle);
    std::optional<PipelineResource*> get_pipeline(uint64_t handle);
    std::optional<const PipelineResource*> get_pipeline(uint64_t handle) const;

    // Descriptor set layout creation from shader reflection
    VkDescriptorSetLayout create_descriptor_set_layout(const kirbyrecomp::shader::ShaderReflection& reflection,
                                                        uint32_t set = 0);
    std::vector<VkDescriptorSetLayout> create_descriptor_set_layouts(
        const std::vector<kirbyrecomp::shader::ShaderReflection>& reflections);
    VkPipelineLayout create_pipeline_layout(
        const std::vector<VkDescriptorSetLayout>& set_layouts,
        const std::vector<PipelineDesc::PushConstantRange>& push_constant_ranges);

    // Descriptor set management
    uint64_t create_descriptor_pool(const std::vector<VkDescriptorPoolSize>& pool_sizes, uint32_t max_sets);
    uint64_t allocate_descriptor_set(uint64_t pool_handle, const std::vector<VkDescriptorSetLayout>& layouts);
    void free_descriptor_set(uint64_t pool_handle, uint64_t set_handle);
    std::optional<DescriptorSetResource*> get_descriptor_set(uint64_t handle);

    // Update descriptor sets
    void update_descriptor_sets(uint64_t set_handle,
                                const std::vector<VkWriteDescriptorSet>& writes,
                                const std::vector<VkCopyDescriptorSet>& copies = {});

    // Command pool/buffer
    VkCommandPool create_command_pool(uint32_t queue_family, VkCommandPoolCreateFlags flags = 0);
    std::vector<VkCommandBuffer> allocate_command_buffers(VkCommandPool pool, uint32_t count,
                                                          VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    void free_command_buffers(VkCommandPool pool, const std::vector<VkCommandBuffer>& buffers);
    void reset_command_pool(VkCommandPool pool);

    // Synchronization - Fence Management
    uint64_t create_fence(bool signaled = false);
    void destroy_fence(uint64_t handle);
    std::optional<FenceResource*> get_fence(uint64_t handle);
    std::optional<const FenceResource*> get_fence(uint64_t handle) const;
    VkResult wait_fence(uint64_t handle, uint64_t timeout_ns = UINT64_MAX);
    VkResult reset_fence(uint64_t handle);
    VkResult wait_fences(const std::vector<uint64_t>& handles, bool wait_all, uint64_t timeout_ns = UINT64_MAX);

    // Synchronization - Semaphore Management
    uint64_t create_semaphore(bool is_timeline = false, uint64_t initial_value = 0);
    void destroy_semaphore(uint64_t handle);
    std::optional<SemaphoreResource*> get_semaphore(uint64_t handle);
    std::optional<const SemaphoreResource*> get_semaphore(uint64_t handle) const;
    VkResult signal_semaphore(uint64_t handle, uint64_t value = 0);
    VkResult wait_semaphore(uint64_t handle, uint64_t value, uint64_t timeout_ns = UINT64_MAX);

    // Synchronization - Timeline Semaphore Pool
    uint64_t create_timeline_semaphore_pool(uint32_t count);
    void destroy_timeline_semaphore_pool(uint64_t handle);
    uint64_t acquire_timeline_semaphore(uint64_t pool_handle, uint64_t initial_value = 0);
    void release_timeline_semaphore(uint64_t pool_handle, uint64_t semaphore_handle);

    // Synchronization - Event Management
    uint64_t create_event();
    void destroy_event(uint64_t handle);
    std::optional<EventResource*> get_event(uint64_t handle);
    std::optional<const EventResource*> get_event(uint64_t handle) const;
    VkResult set_event(uint64_t handle);
    VkResult reset_event(uint64_t handle);
    VkResult wait_event(VkCommandBuffer cmd_buffer, uint64_t handle, VkPipelineStageFlags stage_mask);

    // Synchronization - Submit with synchronization
    VkResult submit_command_buffer(VkCommandBuffer cmd_buffer,
                                   const std::vector<uint64_t>& wait_semaphores,
                                   const std::vector<uint64_t>& wait_values,
                                   const std::vector<VkPipelineStageFlags>& wait_stages,
                                   const std::vector<uint64_t>& signal_semaphores,
                                   const std::vector<uint64_t>& signal_values,
                                   uint64_t fence_handle = 0);

    // Memory utilities
    uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const;
    VkFormat nvn_format_to_vk(TextureFormat format) const;
    VkImageAspectFlags get_format_aspect(VkFormat format) const;

    // Public accessors for descriptor pools (needed by command translator)
    const std::unordered_map<uint64_t, VkDescriptorPool>& get_descriptor_pools() const {
        return descriptor_pools_;
    }
    std::unordered_map<uint64_t, VkDescriptorPool>& get_descriptor_pools() {
        return descriptor_pools_;
    }

    // Pipeline cache accessor
    VkPipelineCache get_pipeline_cache() const { return pipeline_cache_; }

    // Get device
    VkDevice device() const { return device_; }
    VkPhysicalDevice physical_device() const { return physical_device_; }

    // Get last error
    const std::string& get_last_error() const { return last_error_; }

private:
    VkDevice device_ = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkPhysicalDeviceMemoryProperties memory_properties_;
    std::string last_error_;

    // VMA allocator
    VmaAllocator vma_allocator_ = VK_NULL_HANDLE;

    // Graphics queue for command submission
    VkQueue graphics_queue_ = VK_NULL_HANDLE;

    // Pipeline cache for install-time compilation
    VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
    std::string pipeline_cache_path_;

    // Resource storage
    std::unordered_map<uint64_t, BufferResource> buffers_;
    std::unordered_map<uint64_t, TextureResource> textures_;
    std::unordered_map<uint64_t, SamplerResource> samplers_;
    std::unordered_map<uint64_t, ShaderModuleResource> shader_modules_;
    std::unordered_map<uint64_t, PipelineResource> pipelines_;
    std::unordered_map<uint64_t, DescriptorSetResource> descriptor_sets_;
    std::unordered_map<uint64_t, VkDescriptorPool> descriptor_pools_;
    std::unordered_map<uint64_t, VkCommandPool> command_pools_;

    // Synchronization resources
    std::unordered_map<uint64_t, FenceResource> fences_;
    std::unordered_map<uint64_t, SemaphoreResource> semaphores_;
    std::unordered_map<uint64_t, TimelineSemaphorePool> timeline_semaphore_pools_;
    std::unordered_map<uint64_t, EventResource> events_;

    uint64_t next_handle_ = 1;

    // Helpers
    uint64_t allocate_handle() { return next_handle_++; }
    bool create_buffer_internal(BufferResource& resource);
    bool create_texture_internal(TextureResource& resource);
    bool create_sampler_internal(SamplerResource& resource);
};

} // namespace kirbyrecomp::nvn