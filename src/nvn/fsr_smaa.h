#pragma once

// FSR 1.0 + SMAA Integration for NVN Runtime
// Implements AMD FidelityFX Super Resolution 1.0 and SMAA anti-aliasing

#include "kirbyrecomp/nvn/resource_manager.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <optional>

namespace kirbyrecomp::nvn {

// ============================================================================
// FSR 1.0 Constants and Structures
// ============================================================================

// FSR 1.0 quality modes
enum class FsrQualityMode : uint32_t {
    UltraQuality = 0,  // 1.3x scale
    Quality      = 1,  // 1.5x scale
    Balanced     = 2,  // 1.7x scale
    Performance  = 3,  // 2.0x scale
};

// FSR 1.0 dispatch parameters
struct FsrDispatchDesc {
    VkImageView input_image_view;
    VkImageView output_image_view;
    VkExtent2D input_extent;
    VkExtent2D output_extent;
    FsrQualityMode quality_mode;
    float sharpness;  // 0.0 to 1.0
};

// FSR 1.0 constants (from FidelityFX SDK)
struct FsrConstants {
    // EASU (Edge Adaptive Spatial Upsampling) constants
    float input_size[2];
    float output_size[2];
    float input_size_rcp[2];
    float output_size_rcp[2];
    float sharpness;
    int render_size[2];
    int padding[2];

    // RCAS (Robust Contrast Adaptive Sharpening) constants
    float rcas_config[4];
};

// ============================================================================
// SMAA Constants and Structures
// ============================================================================

// SMAA quality presets
enum class SmaaQualityPreset : uint32_t {
    Low    = 0,  // SMAA 1x
    Medium = 1,  // SMAA 2x (T2x)
    High   = 2,  // SMAA 4x (S2x + T2x)
    Ultra  = 3,  // SMAA 4x (S2x + T2x + P4x)
};

// SMAA push constants
struct SmaaPushConstants {
    int rt_metrics[4];           // (1.0/width, 1.0/height, width, height)
    float quality_preset;        // 0.0=Low, 1.0=Medium, 2.0=High, 3.0=Ultra
    float predicate_threshold;   // Edge detection threshold
    float depth_threshold;       // Depth edge detection threshold
    float max_search_steps;      // Maximum search steps
    float max_search_steps_diag; // Maximum diagonal search steps
    float corner_rounding;       // Corner rounding factor
    float padding[3];
};

// ============================================================================
// FSR + SMAA Pipeline Manager
// ============================================================================

class FsrSmaaPipeline {
public:
    FsrSmaaPipeline(ResourceManager& resource_manager);
    ~FsrSmaaPipeline() = default;

    // Initialize FSR 1.0 pipelines
    bool initialize_fsr1(VkRenderPass render_pass, VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT);

    // Initialize SMAA pipelines
    bool initialize_smaa(VkRenderPass render_pass, VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT);

    // Execute FSR 1.0 upscale
    bool execute_fsr1(VkCommandBuffer cmd_buffer,
                      const FsrDispatchDesc& desc,
                      VkPipelineLayout pipeline_layout);

    // Execute SMAA anti-aliasing
    bool execute_smaa(VkCommandBuffer cmd_buffer,
                      VkImageView input_image_view,
                      VkImageView output_image_view,
                      VkExtent2D extent,
                      SmaaQualityPreset preset,
                      VkPipelineLayout pipeline_layout);

    // Create FSR 1.0 EASU pipeline (Edge Adaptive Spatial Upsampling)
    bool create_fsr1_easu_pipeline(VkRenderPass render_pass, VkSampleCountFlagBits sample_count);

    // Create FSR 1.0 RCAS pipeline (Robust Contrast Adaptive Sharpening)
    bool create_fsr1_rcas_pipeline(VkRenderPass render_pass, VkSampleCountFlagBits sample_count);

    // Create SMAA edge detection pipeline
    bool create_smaa_edge_detection_pipeline(VkRenderPass render_pass, VkSampleCountFlagBits sample_count);

    // Create SMAA blending weight calculation pipeline
    bool create_smaa_blend_weight_pipeline(VkRenderPass render_pass, VkSampleCountFlagBits sample_count);

    // Create SMAA neighborhood blending pipeline
    bool create_smaa_neighborhood_blending_pipeline(VkRenderPass render_pass, VkSampleCountFlagBits sample_count);

    // Getters for pipeline handles
    uint64_t get_fsr1_easu_pipeline() const { return fsr1_easu_pipeline_; }
    uint64_t get_fsr1_rcas_pipeline() const { return fsr1_rcas_pipeline_; }
    uint64_t get_smaa_edge_pipeline() const { return smaa_edge_pipeline_; }
    uint64_t get_smaa_blend_pipeline() const { return smaa_blend_pipeline_; }
    uint64_t get_smaa_neighborhood_pipeline() const { return smaa_neighborhood_pipeline_; }

    // Get last error
    const std::string& get_last_error() const { return last_error_; }

private:
    ResourceManager& resource_manager_;
    std::string last_error_;

    // FSR 1.0 pipelines
    uint64_t fsr1_easu_pipeline_ = 0;
    uint64_t fsr1_rcas_pipeline_ = 0;
    uint64_t fsr1_easu_shader_ = 0;
    uint64_t fsr1_rcas_shader_ = 0;

    // SMAA pipelines
    uint64_t smaa_edge_pipeline_ = 0;
    uint64_t smaa_blend_pipeline_ = 0;
    uint64_t smaa_neighborhood_pipeline_ = 0;
    uint64_t smaa_edge_shader_ = 0;
    uint64_t smaa_blend_shader_ = 0;
    uint64_t smaa_neighborhood_shader_ = 0;

    // Shared descriptor set layout
    VkDescriptorSetLayout fsr_smaa_desc_set_layout_ = VK_NULL_HANDLE;
    VkPipelineLayout fsr_pipeline_layout_ = VK_NULL_HANDLE;
    VkPipelineLayout smaa_pipeline_layout_ = VK_NULL_HANDLE;

    // Helper: Load SPIR-V shader module
    bool load_shader_module(const std::vector<uint32_t>& spirv, uint64_t& shader_handle);

    // Helper: Create descriptor set layout
    bool create_descriptor_set_layouts();

    // Helper: Create pipeline layouts
    bool create_pipeline_layouts();

    // FSR 1.0 EASU shader SPIR-V (embedded)
    static const std::vector<uint32_t> FSR1_EASU_SPV;
    static const std::vector<uint32_t> FSR1_RCAS_SPV;

    // SMAA shaders SPIR-V (embedded)
    static const std::vector<uint32_t> SMAA_EDGE_SPV;
    static const std::vector<uint32_t> SMAA_BLEND_SPV;
    static const std::vector<uint32_t> SMAA_NEIGHBORHOOD_SPV;
};

// ============================================================================
// FSR 1.0 Quality Mode Helpers
// ============================================================================

inline float fsr1_get_scale_factor(FsrQualityMode mode) {
    switch (mode) {
        case FsrQualityMode::UltraQuality: return 1.3f;
        case FsrQualityMode::Quality:      return 1.5f;
        case FsrQualityMode::Balanced:     return 1.7f;
        case FsrQualityMode::Performance:  return 2.0f;
    }
    return 1.0f;
}

inline void fsr1_calculate_extents(FsrQualityMode mode, uint32_t display_width, uint32_t display_height,
                                   uint32_t& render_width, uint32_t& render_height) {
    float scale = fsr1_get_scale_factor(mode);
    render_width = static_cast<uint32_t>(display_width / scale + 0.5f);
    render_height = static_cast<uint32_t>(display_height / scale + 0.5f);
}

// ============================================================================
// SMAA Quality Preset Helpers
// ============================================================================

inline float smaa_get_preset_value(SmaaQualityPreset preset) {
    return static_cast<float>(preset);
}

inline uint32_t smaa_get_search_steps(SmaaQualityPreset preset) {
    switch (preset) {
        case SmaaQualityPreset::Low:    return 8;
        case SmaaQualityPreset::Medium: return 16;
        case SmaaQualityPreset::High:   return 32;
        case SmaaQualityPreset::Ultra:  return 64;
    }
    return 16;
}

} // namespace kirbyrecomp::nvn