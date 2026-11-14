#pragma once

// SPIR-V Generator for Maxwell → SPIR-V Translation
// Generates valid SPIR-V binary modules from decoded Maxwell instructions

#include "kirbyrecomp/shader/shader.h"
#include "kirbyrecomp/shader/maxwell_decoder.h"
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>

namespace kirbyrecomp::shader::spirv {

// ============================================================================
// SPIR-V Instruction Encoding
// ============================================================================

enum class SpvOp : uint16_t {
    // Module structure
    OpNop                   = 0,
    OpUndef                 = 1,
    OpSourceContinued       = 2,
    OpSource                = 3,
    OpSourceExtension       = 4,
    OpName                  = 5,
    OpMemberName            = 6,
    OpString                = 7,
    OpLine                  = 8,
    OpExtension             = 10,
    OpExtInstImport         = 11,
    OpExtInst               = 12,
    OpMemoryModel           = 14,
    OpEntryPoint            = 15,
    OpExecutionMode         = 16,
    OpCapability            = 17,

    // Debug
    OpString                = 7,
    OpLine                  = 8,

    // Type declaration
    OpTypeVoid              = 19,
    OpTypeBool              = 20,
    OpTypeInt               = 21,
    OpTypeFloat             = 22,
    OpTypeVector            = 23,
    OpTypeMatrix            = 24,
    OpTypeImage             = 25,
    OpTypeSampler           = 26,
    OpTypeSampledImage      = 27,
    OpTypeArray             = 28,
    OpTypeRuntimeArray      = 29,
    OpTypeStruct            = 30,
    OpTypeOpaque            = 31,
    OpTypePointer           = 32,
    OpTypeFunction          = 33,
    OpTypeEvent             = 34,
    OpTypeDeviceEvent       = 35,
    OpTypeReserveId         = 36,
    OpTypeQueue             = 37,
    OpTypePipe              = 38,
    OpTypeForwardPointer    = 39,
    OpConstantTrue          = 41,
    OpConstantFalse         = 42,
    OpConstant              = 43,
    OpConstantComposite     = 44,
    OpConstantSampler       = 45,
    OpConstantNull          = 46,
    OpSpecConstantTrue      = 48,
    OpSpecConstantFalse     = 49,
    OpSpecConstant          = 50,
    OpSpecConstantComposite = 51,
    OpSpecConstantOp        = 52,

    // Memory
    OpVariable              = 59,
    OpImageTexelPointer     = 60,
    OpLoad                  = 61,
    OpStore                 = 62,
    OpCopyMemory            = 63,
    OpCopyMemorySized       = 64,
    OpAccessChain           = 65,
    OpInBoundsAccessChain   = 66,
    OpPtrAccessChain        = 67,
    OpArrayLength           = 68,
    OpGenericPtrMemSemantics= 69,
    OpInBoundsPtrAccessChain= 70,

    // Function
    OpFunction              = 54,
    OpFunctionParameter     = 55,
    OpFunctionEnd           = 56,
    OpFunctionCall          = 57,

    // Control flow
    OpLabel                 = 248,
    OpBranch                = 249,
    OpBranchConditional     = 250,
    OpSwitch                = 251,
    OpKill                  = 252,
    OpReturn                = 253,
    OpReturnValue           = 254,
    OpUnreachable           = 255,
    OpPhi                   = 245,
    OpLoopMerge             = 246,
    OpSelectionMerge        = 247,

    // Integer arithmetic
    OpIAdd                  = 66,
    OpISub                  = 67,
    OpIMul                  = 68,
    OpSDiv                  = 69,
    OpUDiv                  = 70,
    OpSRem                  = 71,
    OpUMod                  = 72,
    OpSMod                  = 73,
    OpShiftRightLogical     = 74,
    OpShiftRightArithmetic  = 75,
    OpShiftLeftLogical      = 76,
    OpBitwiseOr             = 77,
    OpBitwiseXor            = 78,
    OpBitwiseAnd            = 79,
    OpNot                   = 80,
    OpBitFieldInsert        = 81,
    OpBitFieldSExtract      = 82,
    OpBitFieldUExtract      = 83,
    OpBitReverse            = 84,
    OpBitCount              = 85,

    // Floating point
    OpFAdd                  = 86,
    OpFSub                  = 87,
    OpFMul                  = 88,
    OpFDiv                  = 89,
    OpFRem                  = 90,
    OpFMod                  = 91,
    OpFOrdEqual             = 92,
    OpFUnordEqual           = 93,
    OpFOrdNotEqual          = 94,
    OpFUnordNotEqual        = 95,
    OpFOrdLessThan          = 96,
    OpFUnordLessThan        = 97,
    OpFOrdGreaterThan       = 98,
    OpFUnordGreaterThan     = 99,
    OpFOrdLessThanEqual     = 100,
    OpFUnordLessThanEqual   = 101,
    OpFOrdGreaterThanEqual  = 102,
    OpFUnordGreaterThanEqual= 103,
    OpFMax                  = 104,
    OpFMin                  = 105,
    OpFNegate               = 106,
    OpFAbs                  = 107,
    OpFSqrt                 = 108,
    OpFConvert              = 109,
    OpSConvert              = 110,
    OpUConvert              = 111,
    OpFNegate               = 106,

    // Vector
    OpVectorShuffle         = 112,
    OpCompositeExtract      = 113,
    OpCompositeInsert       = 114,
    OpCompositeConstruct    = 115,

    // Image/Sampler
    OpSampledImage          = 116,
    OpImageSampleImplicitLod= 117,
    OpImageSampleExplicitLod= 118,
    OpImageSampleDrefImplicitLod= 119,
    OpImageSampleDrefExplicitLod= 120,
    OpImageSampleProjImplicitLod= 121,
    OpImageSampleProjExplicitLod= 122,
    OpImageSampleProjDrefImplicitLod= 123,
    OpImageSampleProjDrefExplicitLod= 124,
    OpImageFetch            = 125,
    OpImageGather           = 126,
    OpImageDrefGather       = 127,
    OpImageRead             = 128,
    OpImageWrite            = 129,
    OpImage                 = 130,
    OpImageQueryFormat      = 131,
    OpImageQueryOrder       = 132,
    OpImageQuerySizeLod     = 133,
    OpImageQuerySize        = 134,
    OpImageQueryLod         = 135,
    OpImageQueryLevels      = 136,
    OpImageQuerySamples     = 137,

    // Conversion
    OpConvertFToU           = 138,
    OpConvertFToS           = 139,
    OpConvertSToF           = 140,
    OpConvertUToF           = 141,
    OpUConvert              = 111,
    OpSConvert              = 110,
    OpFConvert              = 109,

    // Atomic
    OpAtomicLoad            = 175,
    OpAtomicStore           = 176,
    OpAtomicExchange        = 177,
    OpAtomicCompareExchange = 178,
    OpAtomicCompareExchangeWeak= 179,
    OpAtomicIIncrement      = 180,
    OpAtomicIDecrement      = 181,
    OpAtomicIAdd            = 182,
    OpAtomicISub            = 183,
    OpAtomicSMin            = 184,
    OpAtomicUMin            = 185,
    OpAtomicSMax            = 186,
    OpAtomicUMax            = 187,
    OpAtomicAnd             = 188,
    OpAtomicOr              = 189,
    OpAtomicXor             = 190,

    // Barrier
    OpControlBarrier        = 210,
    OpMemoryBarrier         = 211,

    // Subgroup
    OpSubgroupBallotKHR     = 5376,
    OpSubgroupFirstInvocationKHR = 5377,
    OpSubgroupAllKHR        = 5378,
    OpSubgroupAnyKHR        = 5379,
    OpSubgroupAllEqualKHR   = 5380,
};

// Execution models
enum class ExecutionModel : uint32_t {
    Vertex                  = 0,
    TessellationControl     = 1,
    TessellationEvaluation  = 2,
    Geometry                = 3,
    Fragment                = 4,
    GLCompute               = 5,
    Kernel                  = 6,
};

// Storage classes
enum class StorageClass : uint32_t {
    UniformConstant         = 0,
    Input                   = 1,
    Uniform                 = 2,
    Output                  = 3,
    Workgroup               = 4,
    CrossWorkgroup          = 5,
    Private                 = 6,
    Function                = 7,
    Generic                 = 8,
    PushConstant            = 9,
    AtomicCounter           = 10,
    Image                   = 11,
    StorageBuffer           = 12,
    CallableDataKHR         = 5358,
    IncomingCallableDataKHR = 5359,
    RayPayloadKHR           = 5362,
    HitAttributeKHR         = 5363,
    IncomingRayPayloadKHR   = 5364,
    ShaderRecordBufferKHR   = 5365,
};

// Built-in variables
enum class BuiltIn : uint32_t {
    Position                = 0,
    PointSize               = 1,
    ClipDistance            = 3,
    CullDistance            = 4,
    VertexId                = 5,
    InstanceId              = 6,
    PrimitiveId             = 7,
    InvocationId            = 8,
    Layer                   = 9,
    ViewportIndex           = 10,
    TessLevelOuter          = 11,
    TessLevelInner          = 12,
    TessCoord               = 13,
    PatchVertices           = 14,
    FragCoord               = 15,
    PointCoord              = 16,
    FrontFacing             = 17,
    SampleId                = 18,
    SamplePosition          = 19,
    SampleMask              = 20,
    FragDepth               = 22,
    HelperInvocation        = 23,
    NumWorkgroups           = 24,
    WorkgroupSize           = 25,
    WorkgroupId             = 26,
    LocalInvocationId       = 27,
    GlobalInvocationId      = 28,
    LocalInvocationIndex    = 29,
    WorkDim                 = 30,
    GlobalSize              = 31,
    EnqueuedWorkgroupSize   = 32,
    GlobalOffset            = 33,
    GlobalLinearId          = 34,
    SubgroupSize            = 36,
    SubgroupMaxSize         = 37,
    NumSubgroups            = 38,
    NumEnqueuedSubgroups    = 39,
    SubgroupId              = 40,
    SubgroupLocalInvocationId = 41,
};

// ============================================================================
// SPIR-V Builder
// ============================================================================

class SpvBuilder {
public:
    SpvBuilder() = default;
    ~SpvBuilder() = default;

    // Create a new SPIR-V module
    void begin_module(uint32_t version = 0x00010300, uint32_t generator = 0x00010000);

    // Finalize and get the module
    SpvModule end_module();

    // Capability declarations
    void capability(uint32_t capability);
    void extension(const std::string& name);
    void ext_inst_import(uint32_t result_id, const std::string& name);

    // Memory model
    void memory_model(uint32_t addressing_model, uint32_t memory_model);

    // Entry point
    void entry_point(ExecutionModel model, uint32_t entry_point_id,
                     const std::string& name, const std::vector<uint32_t>& interface_ids);

    // Execution mode
    void execution_mode(uint32_t entry_point_id, uint32_t mode, const std::vector<uint32_t>& args);

    // Type declarations
    uint32_t type_void();
    uint32_t type_bool();
    uint32_t type_int(uint32_t width, uint32_t signedness);
    uint32_t type_float(uint32_t width);
    uint32_t type_vector(uint32_t component_type_id, uint32_t component_count);
    uint32_t type_pointer(StorageClass storage_class, uint32_t type_id);
    uint32_t type_function(uint32_t return_type_id, const std::vector<uint32_t>& param_type_ids);
    uint32_t type_array(uint32_t element_type_id, uint32_t length_id);
    uint32_t type_runtime_array(uint32_t element_type_id);
    uint32_t type_struct(const std::vector<uint32_t>& member_type_ids);
    uint32_t type_image(uint32_t sampled_type_id, uint32_t dim, uint32_t depth,
                        uint32_t arrayed, uint32_t ms, uint32_t sampled,
                        StorageClass storage_class, uint32_t format);
    uint32_t type_sampler();

    // Constants
    uint32_t constant_int(uint32_t type_id, int64_t value);
    uint32_t constant_uint(uint32_t type_id, uint64_t value);
    uint32_t constant_float(uint32_t type_id, double value);
    uint32_t constant_true(uint32_t type_id);
    uint32_t constant_false(uint32_t type_id);
    uint32_t constant_composite(uint32_t type_id, const std::vector<uint32_t>& constituent_ids);

    // Variables
    uint32_t variable(uint32_t pointer_type_id, StorageClass storage_class,
                      uint32_t initializer = 0);
    uint32_t variable_with_name(uint32_t pointer_type_id, StorageClass storage_class,
                                const std::string& name, uint32_t initializer = 0);

    // Functions
    uint32_t function_begin(uint32_t return_type_id, uint32_t function_type_id);
    uint32_t function_parameter(uint32_t type_id);
    void function_end();
    uint32_t function_call(uint32_t return_type_id, uint32_t function_id,
                           const std::vector<uint32_t>& arg_ids);

    // Basic blocks
    uint32_t label();
    void branch(uint32_t target_label);
    void branch_conditional(uint32_t condition_id, uint32_t true_label,
                            uint32_t false_label, const std::vector<uint32_t>& weights = {});
    void selection_merge(uint32_t merge_label, uint32_t selection_control);
    void loop_merge(uint32_t merge_label, uint32_t continue_label, uint32_t loop_control);
    void phi(uint32_t result_type_id, uint32_t result_id,
             const std::vector<std::pair<uint32_t, uint32_t>>& pairs);
    void return_inst();
    void unreachable();
    void kill();

    // Memory access
    uint32_t load(uint32_t result_type_id, uint32_t pointer_id,
                  uint32_t memory_access = 0);
    void store(uint32_t pointer_id, uint32_t object_id, uint32_t memory_access = 0);
    uint32_t access_chain(uint32_t result_type_id, uint32_t base_id,
                          const std::vector<uint32_t>& index_ids);
    uint32_t in_bounds_access_chain(uint32_t result_type_id, uint32_t base_id,
                                     const std::vector<uint32_t>& index_ids);

    // Arithmetic
    uint32_t iadd(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t isub(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t imul(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t sdiv(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t udiv(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t fadd(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t fsub(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t fmul(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t fdiv(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t fnegate(uint32_t result_type_id, uint32_t operand);
    uint32_t fabs(uint32_t result_type_id, uint32_t operand);
    uint32_t fsqrt(uint32_t result_type_id, uint32_t operand);

    // Bitwise
    uint32_t bitwise_or(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t bitwise_xor(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t bitwise_and(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t bitwise_not(uint32_t result_type_id, uint32_t operand);
    uint32_t shift_left_logical(uint32_t result_type_id, uint32_t base, uint32_t shift);
    uint32_t shift_right_logical(uint32_t result_type_id, uint32_t base, uint32_t shift);
    uint32_t shift_right_arithmetic(uint32_t result_type_id, uint32_t base, uint32_t shift);

    // Comparison
    uint32_t i_equal(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t i_not_equal(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t s_greater_than(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t s_less_than(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t f_ord_equal(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t f_ord_less_than(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);
    uint32_t f_ord_greater_than(uint32_t result_type_id, uint32_t operand1, uint32_t operand2);

    // Vector operations
    uint32_t composite_construct(uint32_t result_type_id, const std::vector<uint32_t>& constituent_ids);
    uint32_t composite_extract(uint32_t result_type_id, uint32_t composite_id,
                               const std::vector<uint32_t>& indexes);
    uint32_t vector_shuffle(uint32_t result_type_id, uint32_t vector1, uint32_t vector2,
                            const std::vector<uint32_t>& components);

    // Conversion
    uint32_t convert_f_to_u(uint32_t result_type_id, uint32_t operand);
    uint32_t convert_f_to_s(uint32_t result_type_id, uint32_t operand);
    uint32_t convert_s_to_f(uint32_t result_type_id, uint32_t operand);
    uint32_t convert_u_to_f(uint32_t result_type_id, uint32_t operand);
    uint32_t f_convert(uint32_t result_type_id, uint32_t operand);
    uint32_t s_convert(uint32_t result_type_id, uint32_t operand);
    uint32_t u_convert(uint32_t result_type_id, uint32_t operand);

    // Image/Sampler
    uint32_t sampled_image(uint32_t result_type_id, uint32_t image_id, uint32_t sampler_id);
    uint32_t image_sample_implicit_lod(uint32_t result_type_id, uint32_t sampled_image_id,
                                        uint32_t coordinate_id, const std::vector<uint32_t>& operands = {});
    uint32_t image_fetch(uint32_t result_type_id, uint32_t image_id,
                         uint32_t coordinate_id, const std::vector<uint32_t>& operands = {});
    uint32_t image_query_size(uint32_t result_type_id, uint32_t image_id, uint32_t lod_id = 0);

    // Barrier
    void control_barrier(uint32_t execution, uint32_t memory, uint32_t semantics);
    void memory_barrier(uint32_t memory, uint32_t semantics);

    // Naming/debug
    void name(uint32_t target_id, const std::string& name);
    void member_name(uint32_t type_id, uint32_t member_index, const std::string& name);

    // Get next available ID
    uint32_t next_id() const { return next_id_; }

    // Get current module
    const SpvModule& module() const { return module_; }

private:
    SpvModule module_;
    uint32_t next_id_ = 1;

    // Emit an instruction
    void emit(uint16_t opcode, const std::vector<uint32_t>& operands);
    void emit_with_result(uint16_t opcode, uint32_t result_type, uint32_t result_id,
                          const std::vector<uint32_t>& operands);
};

// ============================================================================
// Shader Translation Context
// ============================================================================

struct TranslationContext {
    SpvBuilder builder;
    std::unordered_map<std::string, uint32_t> register_map;  // Maxwell register -> SPIR-V ID
    std::unordered_map<uint32_t, uint32_t> ssa_map;          // SSA value -> SPIR-V ID
    std::unordered_map<std::string, uint32_t> function_map;  // Function name -> SPIR-V ID
    std::unordered_map<uint32_t, uint32_t> label_map;        // Basic block address -> Label ID
    std::vector<uint32_t> current_block_vars;                // Variables in current block

    // Shader metadata
    NvnShaderStage stage = NvnShaderStage::Fragment;
    uint32_t local_size_x = 64;
    uint32_t local_size_y = 1;
    uint32_t local_size_z = 1;

    // Type IDs (cached)
    uint32_t void_type = 0;
    uint32_t bool_type = 0;
    uint32_t int32_type = 0;
    uint32_t uint32_type = 0;
    uint32_t float32_type = 0;
    uint32_t vec4_type = 0;

    // Function state
    uint32_t current_function = 0;
    uint32_t entry_point_id = 0;
};

// Translate a decoded Maxwell instruction to SPIR-V
bool translate_instruction(TranslationContext& ctx, const maxwell::DecodedInstruction& inst);

// Translate a full shader
std::optional<SpvModule> translate_shader(
    const std::vector<maxwell::DecodedInstruction>& instructions,
    NvnShaderStage stage,
    const ShaderTranslationOptions& options = {});

} // namespace kirbyrecomp::shader::spirv