// NVN Resource Manager Implementation

#include "kirbyrecomp/nvn/resource_manager.h"

#include <cstring>
#include <algorithm>
#include <iostream>

namespace kirbyrecomp::nvn {

// ============================================================================
// ResourceManager Implementation
// ============================================================================

ResourceManager::ResourceManager(VkDevice device, VkPhysicalDevice physical_device)
    : device_(device), physical_device_(physical_device) {
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties_);
}

ResourceManager::~ResourceManager() {
    // Cleanup all resources
    for (auto& [handle, buffer] : buffers_) {
        if (buffer.view != VK_NULL_HANDLE) {
            vkDestroyBufferView(device_, buffer.view, nullptr);
        }
        if (buffer.buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(device_, buffer.buffer, nullptr);
        }
        if (buffer.memory != VK_NULL_HANDLE) {
            vkFreeMemory(device_, buffer.memory, nullptr);
        }
    }
    buffers_.clear();

    for (auto& [handle, texture] : textures_) {
        for (auto view : texture.mip_views) {
            if (view != VK_NULL_HANDLE) vkDestroyImageView(device_, view, nullptr);
        }
        for (auto view : texture.array_views) {
            if (view != VK_NULL_HANDLE) vkDestroyImageView(device_, view, nullptr);
        }
        if (texture.default_view != VK_NULL_HANDLE) {
            vkDestroyImageView(device_, texture.default_view, nullptr);
        }
        if (texture.image != VK_NULL_HANDLE) {
            vkDestroyImage(device_, texture.image, nullptr);
        }
        if (texture.memory != VK_NULL_HANDLE) {
            vkFreeMemory(device_, texture.memory, nullptr);
        }
    }
    textures_.clear();

    for (auto& [handle, sampler] : samplers_) {
        if (sampler.sampler != VK_NULL_HANDLE) {
            vkDestroySampler(device_, sampler.sampler, nullptr);
        }
    }
    samplers_.clear();

    for (auto& [handle, pipeline] : pipelines_) {
        if (pipeline.pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(device_, pipeline.pipeline, nullptr);
        }
        if (pipeline.layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_, pipeline.layout, nullptr);
        }
        for (auto layout : pipeline.descriptor_set_layouts) {
            if (layout != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(device_, layout, nullptr);
            }
        }
    }
    pipelines_.clear();

    for (auto& [handle, pool] : descriptor_pools_) {
        if (pool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device_, pool, nullptr);
        }
    }
    descriptor_pools_.clear();

    for (auto& [handle, pool] : command_pools_) {
        if (pool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device_, pool, nullptr);
        }
    }
    command_pools_.clear();
}

bool ResourceManager::initialize() {
    // Nothing special needed for initialization
    return true;
}

// ============================================================================
// Buffer Management
// ============================================================================

uint64_t ResourceManager::create_buffer(const BufferDesc& desc) {
    uint64_t handle = allocate_handle();
    BufferResource resource;
    resource.desc = desc;

    if (!create_buffer_internal(resource)) {
        last_error_ = "Failed to create buffer";
        return 0;
    }

    buffers_[handle] = std::move(resource);
    return handle;
}

bool ResourceManager::create_buffer_internal(BufferResource& resource) {
    VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = resource.desc.size;
    buffer_info.usage = 0;

    // Convert usage flags
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(BufferUsage::VertexBuffer)) {
        buffer_info.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(BufferUsage::IndexBuffer)) {
        buffer_info.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(BufferUsage::ConstantBuffer)) {
        buffer_info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(BufferUsage::ShaderResource)) {
        buffer_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(BufferUsage::UnorderedAccess)) {
        buffer_info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(BufferUsage::IndirectArgs)) {
        buffer_info.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(BufferUsage::CopySrc)) {
        buffer_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(BufferUsage::CopyDst)) {
        buffer_info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    // Default to transfer dst if none specified
    if (buffer_info.usage == 0) {
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(device_, &buffer_info, nullptr, &resource.buffer);
    if (result != VK_SUCCESS) {
        last_error_ = "vkCreateBuffer failed: " + std::to_string(result);
        return false;
    }

    // Get memory requirements
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(device_, resource.buffer, &mem_reqs);

    // Determine memory type
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (resource.desc.persistent_map) {
        properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }

    uint32_t memory_type = find_memory_type(mem_reqs.memoryTypeBits, properties);
    if (memory_type == UINT32_MAX) {
        last_error_ = "Failed to find suitable memory type for buffer";
        vkDestroyBuffer(device_, resource.buffer, nullptr);
        resource.buffer = VK_NULL_HANDLE;
        return false;
    }

    VkMemoryAllocateInfo alloc_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = memory_type;

    // Add buffer device address info for shader device address
    VkMemoryAllocateFlagsInfoKHR flags_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR};
    if (buffer_info.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        alloc_info.pNext = &flags_info;
    }

    result = vkAllocateMemory(device_, &alloc_info, nullptr, &resource.memory);
    if (result != VK_SUCCESS) {
        last_error_ = "vkAllocateMemory failed: " + std::to_string(result);
        vkDestroyBuffer(device_, resource.buffer, nullptr);
        resource.buffer = VK_NULL_HANDLE;
        return false;
    }

    result = vkBindBufferMemory(device_, resource.buffer, resource.memory, 0);
    if (result != VK_SUCCESS) {
        last_error_ = "vkBindBufferMemory failed: " + std::to_string(result);
        vkFreeMemory(device_, resource.memory, nullptr);
        vkDestroyBuffer(device_, resource.buffer, nullptr);
        resource.memory = VK_NULL_HANDLE;
        resource.buffer = VK_NULL_HANDLE;
        return false;
    }

    // Get GPU address if shader device address is used
    if (buffer_info.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfoKHR addr_info{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR};
        addr_info.buffer = resource.buffer;
        resource.gpu_address = vkGetBufferDeviceAddress(device_, &addr_info);
    }

    // Create buffer view for constant/storage buffers if needed
    if (buffer_info.usage & (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)) {
        VkBufferViewCreateInfo view_info{VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO};
        view_info.buffer = resource.buffer;
        view_info.format = VK_FORMAT_UNDEFINED;  // Raw buffer view
        view_info.offset = 0;
        view_info.range = VK_WHOLE_SIZE;
        vkCreateBufferView(device_, &view_info, nullptr, &resource.view);
    }

    return true;
}

void ResourceManager::destroy_buffer(uint64_t handle) {
    auto it = buffers_.find(handle);
    if (it == buffers_.end()) return;

    BufferResource& resource = it->second;
    if (resource.mapped_ptr) {
        unmap_buffer(handle);
    }
    if (resource.view != VK_NULL_HANDLE) {
        vkDestroyBufferView(device_, resource.view, nullptr);
    }
    if (resource.buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device_, resource.buffer, nullptr);
    }
    if (resource.memory != VK_NULL_HANDLE) {
        vkFreeMemory(device_, resource.memory, nullptr);
    }
    buffers_.erase(it);
}

std::optional<BufferResource*> ResourceManager::get_buffer(uint64_t handle) {
    auto it = buffers_.find(handle);
    if (it == buffers_.end()) return std::nullopt;
    return &it->second;
}

std::optional<const BufferResource*> ResourceManager::get_buffer(uint64_t handle) const {
    auto it = buffers_.find(handle);
    if (it == buffers_.end()) return std::nullopt;
    return &it->second;
}

void* ResourceManager::map_buffer(uint64_t handle) {
    auto opt = get_buffer(handle);
    if (!opt || !*opt) return nullptr;

    BufferResource* resource = *opt;
    if (resource->mapped_ptr) return resource->mapped_ptr;

    VkResult result = vkMapMemory(device_, resource->memory, 0, VK_WHOLE_SIZE, 0, &resource->mapped_ptr);
    if (result != VK_SUCCESS) {
        last_error_ = "vkMapMemory failed: " + std::to_string(result);
        return nullptr;
    }
    return resource->mapped_ptr;
}

void ResourceManager::unmap_buffer(uint64_t handle) {
    auto opt = get_buffer(handle);
    if (!opt || !*opt) return;

    BufferResource* resource = *opt;
    if (!resource->mapped_ptr) return;

    vkUnmapMemory(device_, resource->memory);
    resource->mapped_ptr = nullptr;
}

uint64_t ResourceManager::get_buffer_gpu_address(uint64_t handle) {
    auto opt = get_buffer(handle);
    if (!opt || !*opt) return 0;
    return (*opt)->gpu_address;
}

// ============================================================================
// Texture Management
// ============================================================================

uint64_t ResourceManager::create_texture(const TextureDesc& desc) {
    uint64_t handle = allocate_handle();
    TextureResource resource;
    resource.desc = desc;

    if (!create_texture_internal(resource)) {
        last_error_ = "Failed to create texture";
        return 0;
    }

    textures_[handle] = std::move(resource);
    return handle;
}

bool ResourceManager::create_texture_internal(TextureResource& resource) {
    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.imageType = (resource.desc.depth > 1) ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
    image_info.format = nvn_format_to_vk(resource.desc.format);
    image_info.extent.width = resource.desc.width;
    image_info.extent.height = resource.desc.height;
    image_info.extent.depth = resource.desc.depth;
    image_info.mipLevels = resource.desc.mip_levels;
    image_info.arrayLayers = resource.desc.array_layers;
    image_info.samples = static_cast<VkSampleCountFlagBits>(resource.desc.sample_count);
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = 0;

    // Convert usage flags
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(TextureUsage::ShaderResource)) {
        image_info.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(TextureUsage::RenderTarget)) {
        image_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(TextureUsage::DepthStencil)) {
        image_info.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(TextureUsage::UnorderedAccess)) {
        image_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(TextureUsage::CopySrc)) {
        image_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(TextureUsage::CopyDst)) {
        image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(TextureUsage::ResolveSrc)) {
        image_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(TextureUsage::ResolveDst)) {
        image_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (static_cast<uint32_t>(resource.desc.usage) & static_cast<uint32_t>(TextureUsage::Present)) {
        image_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    // Default usage
    if (image_info.usage == 0) {
        image_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    // Cube map
    if (resource.desc.flags & 0x1) {  // Assuming bit 0 = cube map
        image_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    VkResult result = vkCreateImage(device_, &image_info, nullptr, &resource.image);
    if (result != VK_SUCCESS) {
        last_error_ = "vkCreateImage failed: " + std::to_string(result);
        return false;
    }

    // Get memory requirements
    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(device_, resource.image, &mem_reqs);

    uint32_t memory_type = find_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (memory_type == UINT32_MAX) {
        last_error_ = "Failed to find suitable memory type for texture";
        vkDestroyImage(device_, resource.image, nullptr);
        resource.image = VK_NULL_HANDLE;
        return false;
    }

    VkMemoryAllocateInfo alloc_info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = memory_type;

    result = vkAllocateMemory(device_, &alloc_info, nullptr, &resource.memory);
    if (result != VK_SUCCESS) {
        last_error_ = "vkAllocateMemory failed: " + std::to_string(result);
        vkDestroyImage(device_, resource.image, nullptr);
        resource.image = VK_NULL_HANDLE;
        return false;
    }

    result = vkBindImageMemory(device_, resource.image, resource.memory, 0);
    if (result != VK_SUCCESS) {
        last_error_ = "vkBindImageMemory failed: " + std::to_string(result);
        vkFreeMemory(device_, resource.memory, nullptr);
        vkDestroyImage(device_, resource.image, nullptr);
        resource.memory = VK_NULL_HANDLE;
        resource.image = VK_NULL_HANDLE;
        return false;
    }

    // Create default image view
    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.image = resource.image;
    view_info.viewType = (resource.desc.array_layers > 1) ?
        (resource.desc.depth > 1 ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D_ARRAY) :
        (resource.desc.depth > 1 ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D);
    view_info.format = image_info.format;
    view_info.subresourceRange.aspectMask = get_format_aspect(view_info.format);
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = resource.desc.mip_levels;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = resource.desc.array_layers;

    result = vkCreateImageView(device_, &view_info, nullptr, &resource.default_view);
    if (result != VK_SUCCESS) {
        last_error_ = "vkCreateImageView failed: " + std::to_string(result);
        vkFreeMemory(device_, resource.memory, nullptr);
        vkDestroyImage(device_, resource.image, nullptr);
        resource.memory = VK_NULL_HANDLE;
        resource.image = VK_NULL_HANDLE;
        return false;
    }

    // Create per-mip views if needed
    if (resource.desc.mip_levels > 1) {
        resource.mip_views.resize(resource.desc.mip_levels);
        for (uint32_t i = 0; i < resource.desc.mip_levels; ++i) {
            view_info.subresourceRange.baseMipLevel = i;
            view_info.subresourceRange.levelCount = 1;
            vkCreateImageView(device_, &view_info, nullptr, &resource.mip_views[i]);
        }
    }

    // Create per-array views if needed
    if (resource.desc.array_layers > 1) {
        resource.array_views.resize(resource.desc.array_layers);
        for (uint32_t i = 0; i < resource.desc.array_layers; ++i) {
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = resource.desc.mip_levels;
            view_info.subresourceRange.baseArrayLayer = i;
            view_info.subresourceRange.layerCount = 1;
            vkCreateImageView(device_, &view_info, nullptr, &resource.array_views[i]);
        }
    }

    return true;
}

void ResourceManager::destroy_texture(uint64_t handle) {
    auto it = textures_.find(handle);
    if (it == textures_.end()) return;

    TextureResource& resource = it->second;
    for (auto view : resource.mip_views) {
        if (view != VK_NULL_HANDLE) vkDestroyImageView(device_, view, nullptr);
    }
    for (auto view : resource.array_views) {
        if (view != VK_NULL_HANDLE) vkDestroyImageView(device_, view, nullptr);
    }
    if (resource.default_view != VK_NULL_HANDLE) {
        vkDestroyImageView(device_, resource.default_view, nullptr);
    }
    if (resource.image != VK_NULL_HANDLE) {
        vkDestroyImage(device_, resource.image, nullptr);
    }
    if (resource.memory != VK_NULL_HANDLE) {
        vkFreeMemory(device_, resource.memory, nullptr);
    }
    textures_.erase(it);
}

std::optional<TextureResource*> ResourceManager::get_texture(uint64_t handle) {
    auto it = textures_.find(handle);
    if (it == textures_.end()) return std::nullopt;
    return &it->second;
}

std::optional<const TextureResource*> ResourceManager::get_texture(uint64_t handle) const {
    auto it = textures_.find(handle);
    if (it == textures_.end()) return std::nullopt;
    return &it->second;
}

// ============================================================================
// Sampler Management
// ============================================================================

uint64_t ResourceManager::create_sampler(const SamplerDesc& desc) {
    uint64_t handle = allocate_handle();
    SamplerResource resource;
    resource.desc = desc;

    if (!create_sampler_internal(resource)) {
        last_error_ = "Failed to create sampler";
        return 0;
    }

    samplers_[handle] = std::move(resource);
    return handle;
}

bool ResourceManager::create_sampler_internal(SamplerResource& resource) {
    VkSamplerCreateInfo sampler_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

    // Convert filters
    auto convert_filter = [](SamplerDesc::Filter f) -> VkFilter {
        return (f == SamplerDesc::Filter::Nearest) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;
    };

    sampler_info.magFilter = convert_filter(resource.desc.mag_filter);
    sampler_info.minFilter = convert_filter(resource.desc.min_filter);
    sampler_info.mipmapMode = (resource.desc.mip_filter == SamplerDesc::Filter::Nearest) ?
        VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;

    // Convert address modes
    auto convert_address = [](SamplerDesc::AddressMode m) -> VkSamplerAddressMode {
        switch (m) {
            case SamplerDesc::AddressMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case SamplerDesc::AddressMode::MirrorRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case SamplerDesc::AddressMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case SamplerDesc::AddressMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            case SamplerDesc::AddressMode::MirrorClampToEdge: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        }
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    };

    sampler_info.addressModeU = convert_address(resource.desc.address_u);
    sampler_info.addressModeV = convert_address(resource.desc.address_v);
    sampler_info.addressModeW = convert_address(resource.desc.address_w);

    sampler_info.mipLodBias = resource.desc.mip_lod_bias;
    sampler_info.anisotropyEnable = resource.desc.anisotropy_enable ? VK_TRUE : VK_FALSE;
    sampler_info.maxAnisotropy = resource.desc.max_anisotropy;
    sampler_info.compareEnable = resource.desc.compare_enable ? VK_TRUE : VK_FALSE;

    auto convert_compare = [](SamplerDesc::CompareOp op) -> VkCompareOp {
        switch (op) {
            case SamplerDesc::CompareOp::Never: return VK_COMPARE_OP_NEVER;
            case SamplerDesc::CompareOp::Less: return VK_COMPARE_OP_LESS;
            case SamplerDesc::CompareOp::Equal: return VK_COMPARE_OP_EQUAL;
            case SamplerDesc::CompareOp::LessOrEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
            case SamplerDesc::CompareOp::Greater: return VK_COMPARE_OP_GREATER;
            case SamplerDesc::CompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
            case SamplerDesc::CompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case SamplerDesc::CompareOp::Always: return VK_COMPARE_OP_ALWAYS;
        }
        return VK_COMPARE_OP_NEVER;
    };
    sampler_info.compareOp = convert_compare(resource.desc.compare_op);

    sampler_info.minLod = resource.desc.min_lod;
    sampler_info.maxLod = resource.desc.max_lod;

    // Border color
    if (resource.desc.border_color[0] != 0 || resource.desc.border_color[1] != 0 ||
        resource.desc.border_color[2] != 0 || resource.desc.border_color[3] != 0) {
        sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_CUSTOM_EXT;
    } else {
        sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    }

    VkResult result = vkCreateSampler(device_, &sampler_info, nullptr, &resource.sampler);
    if (result != VK_SUCCESS) {
        last_error_ = "vkCreateSampler failed: " + std::to_string(result);
        return false;
    }

    return true;
}

void ResourceManager::destroy_sampler(uint64_t handle) {
    auto it = samplers_.find(handle);
    if (it == samplers_.end()) return;

    if (it->second.sampler != VK_NULL_HANDLE) {
        vkDestroySampler(device_, it->second.sampler, nullptr);
    }
    samplers_.erase(it);
}

std::optional<SamplerResource*> ResourceManager::get_sampler(uint64_t handle) {
    auto it = samplers_.find(handle);
    if (it == samplers_.end()) return std::nullopt;
    return &it->second;
}

std::optional<const SamplerResource*> ResourceManager::get_sampler(uint64_t handle) const {
    auto it = samplers_.find(handle);
    if (it == samplers_.end()) return std::nullopt;
    return &it->second;
}

// ============================================================================
// Pipeline Management (stub - to be implemented)
// ============================================================================

uint64_t ResourceManager::create_graphics_pipeline(const class PipelineDesc& desc) {
    // TODO: Implement graphics pipeline creation
    return 0;
}

uint64_t ResourceManager::create_compute_pipeline(const class ComputePipelineDesc& desc) {
    // TODO: Implement compute pipeline creation
    return 0;
}

void ResourceManager::destroy_pipeline(uint64_t handle) {
    auto it = pipelines_.find(handle);
    if (it == pipelines_.end()) return;

    PipelineResource& resource = it->second;
    if (resource.pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device_, resource.pipeline, nullptr);
    }
    if (resource.layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device_, resource.layout, nullptr);
    }
    for (auto layout : resource.descriptor_set_layouts) {
        if (layout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device_, layout, nullptr);
        }
    }
    pipelines_.erase(it);
}

std::optional<PipelineResource*> ResourceManager::get_pipeline(uint64_t handle) {
    auto it = pipelines_.find(handle);
    if (it == pipelines_.end()) return std::nullopt;
    return &it->second;
}

std::optional<const PipelineResource*> ResourceManager::get_pipeline(uint64_t handle) const {
    auto it = pipelines_.find(handle);
    if (it == pipelines_.end()) return std::nullopt;
    return &it->second;
}

// ============================================================================
// Descriptor Set Management (stub - to be implemented)
// ============================================================================

uint64_t ResourceManager::create_descriptor_pool(const std::vector<VkDescriptorPoolSize>& pool_sizes, uint32_t max_sets) {
    uint64_t handle = allocate_handle();
    VkDescriptorPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = max_sets;

    VkDescriptorPool pool;
    VkResult result = vkCreateDescriptorPool(device_, &pool_info, nullptr, &pool);
    if (result != VK_SUCCESS) {
        last_error_ = "vkCreateDescriptorPool failed: " + std::to_string(result);
        return 0;
    }

    descriptor_pools_[handle] = pool;
    return handle;
}

uint64_t ResourceManager::allocate_descriptor_set(uint64_t pool_handle, const std::vector<VkDescriptorSetLayout>& layouts) {
    auto pool_it = descriptor_pools_.find(pool_handle);
    if (pool_it == descriptor_pools_.end()) return 0;

    uint64_t handle = allocate_handle();
    DescriptorSetResource resource;

    VkDescriptorSetAllocateInfo alloc_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc_info.descriptorPool = pool_it->second;
    alloc_info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    alloc_info.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> sets(layouts.size());
    VkResult result = vkAllocateDescriptorSets(device_, &alloc_info, sets.data());
    if (result != VK_SUCCESS) {
        last_error_ = "vkAllocateDescriptorSets failed: " + std::to_string(result);
        return 0;
    }

    resource.set = sets[0];
    resource.pool = pool_it->second;
    descriptor_sets_[handle] = std::move(resource);
    return handle;
}

void ResourceManager::free_descriptor_set(uint64_t pool_handle, uint64_t set_handle) {
    auto set_it = descriptor_sets_.find(set_handle);
    if (set_it == descriptor_sets_.end()) return;

    auto pool_it = descriptor_pools_.find(pool_handle);
    if (pool_it == descriptor_pools_.end()) return;

    vkFreeDescriptorSets(device_, pool_it->second, 1, &set_it->second.set);
    descriptor_sets_.erase(set_it);
}

std::optional<DescriptorSetResource*> ResourceManager::get_descriptor_set(uint64_t handle) {
    auto it = descriptor_sets_.find(handle);
    if (it == descriptor_sets_.end()) return std::nullopt;
    return &it->second;
}

void ResourceManager::update_descriptor_sets(uint64_t set_handle,
                                             const std::vector<VkWriteDescriptorSet>& writes,
                                             const std::vector<VkCopyDescriptorSet>& copies) {
    auto it = descriptor_sets_.find(set_handle);
    if (it == descriptor_sets_.end()) return;

    // Patch descriptor set in writes
    std::vector<VkWriteDescriptorSet> patched_writes = writes;
    for (auto& write : patched_writes) {
        write.dstSet = it->second.set;
    }

    vkUpdateDescriptorSets(device_,
                           static_cast<uint32_t>(patched_writes.size()),
                           patched_writes.data(),
                           static_cast<uint32_t>(copies.size()),
                           copies.data());
}

// ============================================================================
// Command Pool/Buffer Management
// ============================================================================

VkCommandPool ResourceManager::create_command_pool(uint32_t queue_family, VkCommandPoolCreateFlags flags) {
    uint64_t handle = allocate_handle();

    VkCommandPoolCreateInfo pool_info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_info.queueFamilyIndex = queue_family;
    pool_info.flags = flags;

    VkCommandPool pool;
    VkResult result = vkCreateCommandPool(device_, &pool_info, nullptr, &pool);
    if (result != VK_SUCCESS) {
        last_error_ = "vkCreateCommandPool failed: " + std::to_string(result);
        return VK_NULL_HANDLE;
    }

    command_pools_[handle] = pool;
    return pool;
}

std::vector<VkCommandBuffer> ResourceManager::allocate_command_buffers(VkCommandPool pool, uint32_t count, VkCommandBufferLevel level) {
    VkCommandBufferAllocateInfo alloc_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    alloc_info.commandPool = pool;
    alloc_info.level = level;
    alloc_info.commandBufferCount = count;

    std::vector<VkCommandBuffer> buffers(count);
    VkResult result = vkAllocateCommandBuffers(device_, &alloc_info, buffers.data());
    if (result != VK_SUCCESS) {
        last_error_ = "vkAllocateCommandBuffers failed: " + std::to_string(result);
        return {};
    }
    return buffers;
}

void ResourceManager::free_command_buffers(VkCommandPool pool, const std::vector<VkCommandBuffer>& buffers) {
    if (!buffers.empty()) {
        vkFreeCommandBuffers(device_, pool, static_cast<uint32_t>(buffers.size()), buffers.data());
    }
}

void ResourceManager::reset_command_pool(VkCommandPool pool) {
    vkResetCommandPool(device_, pool, 0);
}

// ============================================================================
// Synchronization
// ============================================================================

VkFence ResourceManager::create_fence(bool signaled) {
    VkFenceCreateInfo fence_info{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (signaled) fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence;
    vkCreateFence(device_, &fence_info, nullptr, &fence);
    return fence;
}

VkSemaphore ResourceManager::create_semaphore() {
    VkSemaphoreCreateInfo sem_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkSemaphore semaphore;
    vkCreateSemaphore(device_, &sem_info, nullptr, &semaphore);
    return semaphore;
}

VkEvent ResourceManager::create_event() {
    VkEventCreateInfo event_info{VK_STRUCTURE_TYPE_EVENT_CREATE_INFO};
    VkEvent event;
    vkCreateEvent(device_, &event_info, nullptr, &event);
    return event;
}

void ResourceManager::destroy_fence(VkFence fence) {
    if (fence != VK_NULL_HANDLE) vkDestroyFence(device_, fence, nullptr);
}

void ResourceManager::destroy_semaphore(VkSemaphore semaphore) {
    if (semaphore != VK_NULL_HANDLE) vkDestroySemaphore(device_, semaphore, nullptr);
}

void ResourceManager::destroy_event(VkEvent event) {
    if (event != VK_NULL_HANDLE) vkDestroyEvent(device_, event, nullptr);
}

// ============================================================================
// Memory Utilities
// ============================================================================

uint32_t ResourceManager::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
    for (uint32_t i = 0; i < memory_properties_.memoryTypeCount; ++i) {
        if ((type_filter & (1 << i)) &&
            (memory_properties_.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return UINT32_MAX;
}

VkFormat ResourceManager::nvn_format_to_vk(TextureFormat format) const {
    // Simplified format mapping - would need complete mapping
    switch (format) {
        case TextureFormat::R8_UNORM: return VK_FORMAT_R8_UNORM;
        case TextureFormat::R8_SNORM: return VK_FORMAT_R8_SNORM;
        case TextureFormat::R8_UINT: return VK_FORMAT_R8_UINT;
        case TextureFormat::R8_SINT: return VK_FORMAT_R8_SINT;
        case TextureFormat::R16_UNORM: return VK_FORMAT_R16_UNORM;
        case TextureFormat::R16_SNORM: return VK_FORMAT_R16_SNORM;
        case TextureFormat::R16_UINT: return VK_FORMAT_R16_UINT;
        case TextureFormat::R16_SINT: return VK_FORMAT_R16_SINT;
        case TextureFormat::R16_SFLOAT: return VK_FORMAT_R16_SFLOAT;
        case TextureFormat::RG8_UNORM: return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::RG8_SNORM: return VK_FORMAT_R8G8_SNORM;
        case TextureFormat::RG8_UINT: return VK_FORMAT_R8G8_UINT;
        case TextureFormat::RG8_SINT: return VK_FORMAT_R8G8_SINT;
        case TextureFormat::R32_UINT: return VK_FORMAT_R32_UINT;
        case TextureFormat::R32_SINT: return VK_FORMAT_R32_SINT;
        case TextureFormat::R32_SFLOAT: return VK_FORMAT_R32_SFLOAT;
        case TextureFormat::RG16_UNORM: return VK_FORMAT_R16G16_UNORM;
        case TextureFormat::RG16_SNORM: return VK_FORMAT_R16G16_SNORM;
        case TextureFormat::RG16_UINT: return VK_FORMAT_R16G16_UINT;
        case TextureFormat::RG16_SINT: return VK_FORMAT_R16G16_SINT;
        case TextureFormat::RG16_SFLOAT: return VK_FORMAT_R16G16_SFLOAT;
        case TextureFormat::RGBA8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;
        case TextureFormat::RGBA8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
        case TextureFormat::RGBA8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
        case TextureFormat::RGBA8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::BGRA8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::BGRA8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
        case TextureFormat::RGB10A2_UNORM: return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case TextureFormat::RG11B10_UFLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case TextureFormat::RGB9E5_UFLOAT: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case TextureFormat::RG32_UINT: return VK_FORMAT_R32G32_UINT;
        case TextureFormat::RG32_SINT: return VK_FORMAT_R32G32_SINT;
        case TextureFormat::RG32_SFLOAT: return VK_FORMAT_R32G32_SFLOAT;
        case TextureFormat::RGBA16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
        case TextureFormat::RGBA16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
        case TextureFormat::RGBA16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
        case TextureFormat::RGBA16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
        case TextureFormat::RGBA16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
        case TextureFormat::RGBA32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
        case TextureFormat::RGBA32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
        case TextureFormat::RGBA32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case TextureFormat::D16_UNORM: return VK_FORMAT_D16_UNORM;
        case TextureFormat::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::D32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::D32_SFLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case TextureFormat::BC1_RGB_UNORM: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case TextureFormat::BC1_RGB_SRGB: return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
        case TextureFormat::BC1_RGBA_UNORM: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case TextureFormat::BC1_RGBA_SRGB: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case TextureFormat::BC2_UNORM: return VK_FORMAT_BC2_UNORM_BLOCK;
        case TextureFormat::BC2_SRGB: return VK_FORMAT_BC2_SRGB_BLOCK;
        case TextureFormat::BC3_UNORM: return VK_FORMAT_BC3_UNORM_BLOCK;
        case TextureFormat::BC3_SRGB: return VK_FORMAT_BC3_SRGB_BLOCK;
        case TextureFormat::BC4_UNORM: return VK_FORMAT_BC4_UNORM_BLOCK;
        case TextureFormat::BC4_SNORM: return VK_FORMAT_BC4_SNORM_BLOCK;
        case TextureFormat::BC5_UNORM: return VK_FORMAT_BC5_UNORM_BLOCK;
        case TextureFormat::BC5_SNORM: return VK_FORMAT_BC5_SNORM_BLOCK;
        case TextureFormat::BC6H_UFLOAT: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case TextureFormat::BC6H_SFLOAT: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case TextureFormat::BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
        case TextureFormat::BC7_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
        default: return VK_FORMAT_UNDEFINED;
    }
}

VkImageAspectFlags ResourceManager::get_format_aspect(VkFormat format) const {
    // Determine aspect from format
    switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

} // namespace kirbyrecomp::nvn