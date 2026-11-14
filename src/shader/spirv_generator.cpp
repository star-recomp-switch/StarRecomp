// SPIR-V Generator Implementation

#include "kirbyrecomp/shader/spirv_generator.h"

#include <cstring>
#include <sstream>
#include <algorithm>

namespace kirbyrecomp::shader::spirv {

// ============================================================================
// SpvBuilder Implementation
// ============================================================================

void SpvBuilder::begin_module(uint32_t version, uint32_t generator) {
    module_ = SpvModule{};
    next_id_ = 1;

    // SPIR-V header (5 words)
    module_.words.push_back(0x07230203);  // Magic
    module_.words.push_back(version);      // Version
    module_.words.push_back(generator);    // Generator
    module_.words.push_back(0);            // Bound (placeholder)
    module_.words.push_back(0);            // Reserved
}

SpvModule SpvBuilder::end_module() {
    // Update bound in header
    module_.words[3] = next_id_;
    return std::move(module_);
}

void SpvBuilder::capability(uint32_t capability) {
    emit(static_cast<uint16_t>(SpvOp::OpCapability), {capability});
}

void SpvBuilder::extension(const std::string& name) {
    // Convert string to word array
    std::vector<uint32_t> operands;
    for (size_t i = 0; i < name.size(); i += 4) {
        uint32_t word = 0;
        for (size_t j = 0; j < 4 && i + j < name.size(); ++j) {
            word |= static_cast<uint32_t>(name[i + j]) << (j * 8);
        }
        operands.push_back(word);
    }
    emit(static_cast<uint16_t>(SpvOp::OpExtension), operands);
}

void SpvBuilder::ext_inst_import(uint32_t result_id, const std::string& name) {
    std::vector<uint32_t> operands;
    operands.push_back(result_id);
    for (size_t i = 0; i < name.size(); i += 4) {
        uint32_t word = 0;
        for (size_t j = 0; j < 4 && i + j < name.size(); ++j) {
            word |= static_cast<uint32_t>(name[i + j]) << (j * 8);
        }
        operands.push_back(word);
    }
    emit(static_cast<uint16_t>(SpvOp::OpExtInstImport), operands);
}

void SpvBuilder::memory_model(uint32_t addressing_model, uint32_t memory_model) {
    emit(static_cast<uint16_t>(SpvOp::OpMemoryModel), {addressing_model, memory_model});
}

void SpvBuilder::entry_point(ExecutionModel model, uint32_t entry_point_id,
                             const std::string& name, const std::vector<uint32_t>& interface_ids) {
    std::vector<uint32_t> operands;
    operands.push_back(static_cast<uint32_t>(model));
    operands.push_back(entry_point_id);
    for (size_t i = 0; i < name.size(); i += 4) {
        uint32_t word = 0;
        for (size_t j = 0; j < 4 && i + j < name.size(); ++j) {
            word |= static_cast<uint32_t>(name[i + j]) << (j * 8);
        }
        operands.push_back(word);
    }
    for (uint32_t id : interface_ids) {
        operands.push_back(id);
    }
    emit(static_cast<uint16_t>(SpvOp::OpEntryPoint), operands);
}

void SpvBuilder::execution_mode(uint32_t entry_point_id, uint32_t mode,
                                const std::vector<uint32_t>& args) {
    std::vector<uint32_t> operands;
    operands.push_back(entry_point_id);
    operands.push_back(mode);
    operands.insert(operands.end(), args.begin(), args.end());
    emit(static_cast<uint16_t>(SpvOp::OpExecutionMode), operands);
}

uint32_t SpvBuilder::type_void() {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypeVoid), 0, id, {});
    return id;
}

uint32_t SpvBuilder::type_bool() {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypeBool), 0, id, {});
    return id;
}

uint32_t SpvBuilder::type_int(uint32_t width, uint32_t signedness) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypeInt), 0, id, {width, signedness});
    return id;
}

uint32_t SpvBuilder::type_float(uint32_t width) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypeFloat), 0, id, {width});
    return id;
}

uint32_t SpvBuilder::type_vector(uint32_t component_type_id, uint32_t component_count) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypeVector), 0, id,
                     {component_type_id, component_count});
    return id;
}

uint32_t SpvBuilder::type_pointer(StorageClass storage_class, uint32_t type_id) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypePointer), 0, id,
                     {static_cast<uint32_t>(storage_class), type_id});
    return id;
}

uint32_t SpvBuilder::type_function(uint32_t return_type_id,
                                   const std::vector<uint32_t>& param_type_ids) {
    uint32_t id = next_id_++;
    std::vector<uint32_t> operands;
    operands.push_back(return_type_id);
    operands.insert(operands.end(), param_type_ids.begin(), param_type_ids.end());
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypeFunction), 0, id, operands);
    return id;
}

uint32_t SpvBuilder::type_array(uint32_t element_type_id, uint32_t length_id) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypeArray), 0, id,
                     {element_type_id, length_id});
    return id;
}

uint32_t SpvBuilder::type_runtime_array(uint32_t element_type_id) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypeRuntimeArray), 0, id,
                     {element_type_id});
    return id;
}

uint32_t SpvBuilder::type_struct(const std::vector<uint32_t>& member_type_ids) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypeStruct), 0, id, member_type_ids);
    return id;
}

uint32_t SpvBuilder::type_image(uint32_t sampled_type_id, uint32_t dim, uint32_t depth,
                                uint32_t arrayed, uint32_t ms, uint32_t sampled,
                                StorageClass storage_class, uint32_t format) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypeImage), 0, id,
                     {sampled_type_id, dim, depth, arrayed, ms, sampled,
                      static_cast<uint32_t>(storage_class), format});
    return id;
}

uint32_t SpvBuilder::type_sampler() {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpTypeSampler), 0, id, {});
    return id;
}

uint32_t SpvBuilder::constant_int(uint32_t type_id, int64_t value) {
    uint32_t id = next_id_++;
    // Sign-extend to 64-bit
    uint64_t uvalue = static_cast<uint64_t>(value);
    emit_with_result(static_cast<uint16_t>(SpvOp::OpConstant), type_id, id,
                     {static_cast<uint32_t>(uvalue & 0xFFFFFFFF),
                      static_cast<uint32_t>(uvalue >> 32)});
    return id;
}

uint32_t SpvBuilder::constant_uint(uint32_t type_id, uint64_t value) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpConstant), type_id, id,
                     {static_cast<uint32_t>(value & 0xFFFFFFFF),
                      static_cast<uint32_t>(value >> 32)});
    return id;
}

uint32_t SpvBuilder::constant_float(uint32_t type_id, double value) {
    uint32_t id = next_id_++;
    uint32_t bits = *reinterpret_cast<const uint32_t*>(&value);
    emit_with_result(static_cast<uint16_t>(SpvOp::OpConstant), type_id, id, {bits});
    return id;
}

uint32_t SpvBuilder::constant_true(uint32_t type_id) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpConstantTrue), type_id, id, {});
    return id;
}

uint32_t SpvBuilder::constant_false(uint32_t type_id) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpConstantFalse), type_id, id, {});
    return id;
}

uint32_t SpvBuilder::constant_composite(uint32_t type_id,
                                        const std::vector<uint32_t>& constituent_ids) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpConstantComposite), type_id, id,
                     constituent_ids);
    return id;
}

uint32_t SpvBuilder::variable(uint32_t pointer_type_id, StorageClass storage_class,
                              uint32_t initializer) {
    uint32_t id = next_id_++;
    std::vector<uint32_t> operands = {pointer_type_id, static_cast<uint32_t>(storage_class)};
    if (initializer != 0) operands.push_back(initializer);
    emit_with_result(static_cast<uint16_t>(SpvOp::OpVariable), 0, id, operands);
    return id;
}

uint32_t SpvBuilder::variable_with_name(uint32_t pointer_type_id, StorageClass storage_class,
                                        const std::string& name, uint32_t initializer) {
    uint32_t id = variable(pointer_type_id, storage_class, initializer);
    if (!name.empty()) name(id, name);
    return id;
}

uint32_t SpvBuilder::function_begin(uint32_t return_type_id, uint32_t function_type_id) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFunction), return_type_id, id,
                     {function_type_id, 0});  // Function control = 0
    return id;
}

uint32_t SpvBuilder::function_parameter(uint32_t type_id) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFunctionParameter), type_id, id, {});
    return id;
}

void SpvBuilder::function_end() {
    emit(static_cast<uint16_t>(SpvOp::OpFunctionEnd), {});
}

uint32_t SpvBuilder::function_call(uint32_t return_type_id, uint32_t function_id,
                                   const std::vector<uint32_t>& arg_ids) {
    uint32_t id = next_id_++;
    std::vector<uint32_t> operands = {function_id};
    operands.insert(operands.end(), arg_ids.begin(), arg_ids.end());
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFunctionCall), return_type_id, id, operands);
    return id;
}

uint32_t SpvBuilder::label() {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpLabel), 0, id, {});
    return id;
}

void SpvBuilder::branch(uint32_t target_label) {
    emit(static_cast<uint16_t>(SpvOp::OpBranch), {target_label});
}

void SpvBuilder::branch_conditional(uint32_t condition_id, uint32_t true_label,
                                    uint32_t false_label,
                                    const std::vector<uint32_t>& weights) {
    std::vector<uint32_t> operands = {condition_id, true_label, false_label};
    operands.insert(operands.end(), weights.begin(), weights.end());
    emit(static_cast<uint16_t>(SpvOp::OpBranchConditional), operands);
}

void SpvBuilder::selection_merge(uint32_t merge_label, uint32_t selection_control) {
    emit(static_cast<uint16_t>(SpvOp::OpSelectionMerge), {merge_label, selection_control});
}

void SpvBuilder::loop_merge(uint32_t merge_label, uint32_t continue_label,
                            uint32_t loop_control) {
    emit(static_cast<uint16_t>(SpvOp::OpLoopMerge), {merge_label, continue_label, loop_control});
}

void SpvBuilder::phi(uint32_t result_type_id, uint32_t result_id,
                     const std::vector<std::pair<uint32_t, uint32_t>>& pairs) {
    std::vector<uint32_t> operands;
    for (const auto& p : pairs) {
        operands.push_back(p.first);  // parent label
        operands.push_back(p.second); // value
    }
    emit_with_result(static_cast<uint16_t>(SpvOp::OpPhi), result_type_id, result_id, operands);
}

void SpvBuilder::return_inst() {
    emit(static_cast<uint16_t>(SpvOp::OpReturn), {});
}

void SpvBuilder::unreachable() {
    emit(static_cast<uint16_t>(SpvOp::OpUnreachable), {});
}

void SpvBuilder::kill() {
    emit(static_cast<uint16_t>(SpvOp::OpKill), {});
}

uint32_t SpvBuilder::load(uint32_t result_type_id, uint32_t pointer_id,
                          uint32_t memory_access) {
    uint32_t id = next_id_++;
    std::vector<uint32_t> operands = {pointer_id};
    if (memory_access) operands.push_back(memory_access);
    emit_with_result(static_cast<uint16_t>(SpvOp::OpLoad), result_type_id, id, operands);
    return id;
}

void SpvBuilder::store(uint32_t pointer_id, uint32_t object_id,
                       uint32_t memory_access) {
    std::vector<uint32_t> operands = {pointer_id, object_id};
    if (memory_access) operands.push_back(memory_access);
    emit(static_cast<uint16_t>(SpvOp::OpStore), operands);
}

uint32_t SpvBuilder::access_chain(uint32_t result_type_id, uint32_t base_id,
                                  const std::vector<uint32_t>& index_ids) {
    uint32_t id = next_id_++;
    std::vector<uint32_t> operands = {base_id};
    operands.insert(operands.end(), index_ids.begin(), index_ids.end());
    emit_with_result(static_cast<uint16_t>(SpvOp::OpAccessChain), result_type_id, id, operands);
    return id;
}

uint32_t SpvBuilder::in_bounds_access_chain(uint32_t result_type_id, uint32_t base_id,
                                             const std::vector<uint32_t>& index_ids) {
    uint32_t id = next_id_++;
    std::vector<uint32_t> operands = {base_id};
    operands.insert(operands.end(), index_ids.begin(), index_ids.end());
    emit_with_result(static_cast<uint16_t>(SpvOp::OpInBoundsAccessChain), result_type_id, id,
                     operands);
    return id;
}

uint32_t SpvBuilder::iadd(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpIAdd), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::isub(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpISub), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::imul(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpIMul), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::sdiv(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpSDiv), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::udiv(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpUDiv), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::fadd(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFAdd), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::fsub(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFSub), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::fmul(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFMul), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::fdiv(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFDiv), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::fnegate(uint32_t result_type_id, uint32_t operand) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFNegate), result_type_id, id,
                     {operand});
    return id;
}

uint32_t SpvBuilder::fabs(uint32_t result_type_id, uint32_t operand) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFAbs), result_type_id, id,
                     {operand});
    return id;
}

uint32_t SpvBuilder::fsqrt(uint32_t result_type_id, uint32_t operand) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFSqrt), result_type_id, id,
                     {operand});
    return id;
}

uint32_t SpvBuilder::bitwise_or(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpBitwiseOr), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::bitwise_xor(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpBitwiseXor), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::bitwise_and(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpBitwiseAnd), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::bitwise_not(uint32_t result_type_id, uint32_t operand) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpNot), result_type_id, id,
                     {operand});
    return id;
}

uint32_t SpvBuilder::shift_left_logical(uint32_t result_type_id, uint32_t base, uint32_t shift) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpShiftLeftLogical), result_type_id, id,
                     {base, shift});
    return id;
}

uint32_t SpvBuilder::shift_right_logical(uint32_t result_type_id, uint32_t base, uint32_t shift) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpShiftRightLogical), result_type_id, id,
                     {base, shift});
    return id;
}

uint32_t SpvBuilder::shift_right_arithmetic(uint32_t result_type_id, uint32_t base, uint32_t shift) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpShiftRightArithmetic), result_type_id, id,
                     {base, shift});
    return id;
}

uint32_t SpvBuilder::i_equal(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpIEqual), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::i_not_equal(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpINotEqual), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::s_greater_than(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpSGreaterThan), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::s_less_than(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpSLessThan), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::f_ord_equal(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFOrdEqual), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::f_ord_less_than(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFOrdLessThan), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::f_ord_greater_than(uint32_t result_type_id, uint32_t operand1, uint32_t operand2) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFOrdGreaterThan), result_type_id, id,
                     {operand1, operand2});
    return id;
}

uint32_t SpvBuilder::composite_construct(uint32_t result_type_id,
                                         const std::vector<uint32_t>& constituent_ids) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpCompositeConstruct), result_type_id, id,
                     constituent_ids);
    return id;
}

uint32_t SpvBuilder::composite_extract(uint32_t result_type_id, uint32_t composite_id,
                                       const std::vector<uint32_t>& indexes) {
    uint32_t id = next_id_++;
    std::vector<uint32_t> operands = {composite_id};
    operands.insert(operands.end(), indexes.begin(), indexes.end());
    emit_with_result(static_cast<uint16_t>(SpvOp::OpCompositeExtract), result_type_id, id,
                     operands);
    return id;
}

uint32_t SpvBuilder::vector_shuffle(uint32_t result_type_id, uint32_t vector1, uint32_t vector2,
                                    const std::vector<uint32_t>& components) {
    uint32_t id = next_id_++;
    std::vector<uint32_t> operands = {vector1, vector2};
    operands.insert(operands.end(), components.begin(), components.end());
    emit_with_result(static_cast<uint16_t>(SpvOp::OpVectorShuffle), result_type_id, id, operands);
    return id;
}

uint32_t SpvBuilder::convert_f_to_u(uint32_t result_type_id, uint32_t operand) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpConvertFToU), result_type_id, id, {operand});
    return id;
}

uint32_t SpvBuilder::convert_f_to_s(uint32_t result_type_id, uint32_t operand) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpConvertFToS), result_type_id, id, {operand});
    return id;
}

uint32_t SpvBuilder::convert_s_to_f(uint32_t result_type_id, uint32_t operand) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpConvertSToF), result_type_id, id, {operand});
    return id;
}

uint32_t SpvBuilder::convert_u_to_f(uint32_t result_type_id, uint32_t operand) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpConvertUToF), result_type_id, id, {operand});
    return id;
}

uint32_t SpvBuilder::f_convert(uint32_t result_type_id, uint32_t operand) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpFConvert), result_type_id, id, {operand});
    return id;
}

uint32_t SpvBuilder::s_convert(uint32_t result_type_id, uint32_t operand) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpSConvert), result_type_id, id, {operand});
    return id;
}

uint32_t SpvBuilder::u_convert(uint32_t result_type_id, uint32_t operand) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpUConvert), result_type_id, id, {operand});
    return id;
}

uint32_t SpvBuilder::sampled_image(uint32_t result_type_id, uint32_t image_id, uint32_t sampler_id) {
    uint32_t id = next_id_++;
    emit_with_result(static_cast<uint16_t>(SpvOp::OpSampledImage), result_type_id, id,
                     {image_id, sampler_id});
    return id;
}

uint32_t SpvBuilder::image_sample_implicit_lod(uint32_t result_type_id, uint32_t sampled_image_id,
                                                uint32_t coordinate_id,
                                                const std::vector<uint32_t>& operands) {
    uint32_t id = next_id_++;
    std::vector<uint32_t> ops = {sampled_image_id, coordinate_id};
    ops.insert(ops.end(), operands.begin(), operands.end());
    emit_with_result(static_cast<uint16_t>(SpvOp::OpImageSampleImplicitLod), result_type_id, id,
                     ops);
    return id;
}

uint32_t SpvBuilder::image_fetch(uint32_t result_type_id, uint32_t image_id,
                                 uint32_t coordinate_id,
                                 const std::vector<uint32_t>& operands) {
    uint32_t id = next_id_++;
    std::vector<uint32_t> ops = {image_id, coordinate_id};
    ops.insert(ops.end(), operands.begin(), operands.end());
    emit_with_result(static_cast<uint16_t>(SpvOp::OpImageFetch), result_type_id, id, ops);
    return id;
}

uint32_t SpvBuilder::image_query_size(uint32_t result_type_id, uint32_t image_id,
                                      uint32_t lod_id) {
    uint32_t id = next_id_++;
    std::vector<uint32_t> ops = {image_id};
    if (lod_id != 0) ops.push_back(lod_id);
    emit_with_result(static_cast<uint16_t>(SpvOp::OpImageQuerySize), result_type_id, id, ops);
    return id;
}

void SpvBuilder::control_barrier(uint32_t execution, uint32_t memory, uint32_t semantics) {
    emit(static_cast<uint16_t>(SpvOp::OpControlBarrier), {execution, memory, semantics});
}

void SpvBuilder::memory_barrier(uint32_t memory, uint32_t semantics) {
    emit(static_cast<uint16_t>(SpvOp::OpMemoryBarrier), {memory, semantics});
}

void SpvBuilder::name(uint32_t target_id, const std::string& name) {
    std::vector<uint32_t> operands = {target_id};
    for (size_t i = 0; i < name.size(); i += 4) {
        uint32_t word = 0;
        for (size_t j = 0; j < 4 && i + j < name.size(); ++j) {
            word |= static_cast<uint32_t>(name[i + j]) << (j * 8);
        }
        operands.push_back(word);
    }
    emit(static_cast<uint16_t>(SpvOp::OpName), operands);
}

void SpvBuilder::member_name(uint32_t type_id, uint32_t member_index, const std::string& name) {
    std::vector<uint32_t> operands = {type_id, member_index};
    for (size_t i = 0; i < name.size(); i += 4) {
        uint32_t word = 0;
        for (size_t j = 0; j < 4 && i + j < name.size(); ++j) {
            word |= static_cast<uint32_t>(name[i + j]) << (j * 8);
        }
        operands.push_back(word);
    }
    emit(static_cast<uint16_t>(SpvOp::OpMemberName), operands);
}

void SpvBuilder::emit(uint16_t opcode, const std::vector<uint32_t>& operands) {
    uint32_t word_count = static_cast<uint32_t>(operands.size()) + 1;
    module_.words.push_back((word_count << 16) | opcode);
    module_.words.insert(module_.words.end(), operands.begin(), operands.end());
}

void SpvBuilder::emit_with_result(uint16_t opcode, uint32_t result_type, uint32_t result_id,
                                  const std::vector<uint32_t>& operands) {
    std::vector<uint32_t> all_operands;
    if (result_type != 0) all_operands.push_back(result_type);
    all_operands.push_back(result_id);
    all_operands.insert(all_operands.end(), operands.begin(), operands.end());
    emit(opcode, all_operands);
}

// ============================================================================
// Translation Context Helpers
// ============================================================================

static uint32_t get_or_create_register(TranslationContext& ctx, const maxwell::Register& reg) {
    std::string key = reg.to_string();
    auto it = ctx.register_map.find(key);
    if (it != ctx.register_map.end()) return it->second;

    // Create new SSA variable for this register
    uint32_t var_id = ctx.builder.variable(
        ctx.builder.type_pointer(spirv::StorageClass::Function, ctx.int32_type),
        spirv::StorageClass::Function);
    ctx.register_map[key] = var_id;
    ctx.builder.name(var_id, key);
    return var_id;
}

static uint32_t load_register(TranslationContext& ctx, const maxwell::Register& reg) {
    uint32_t var_id = get_or_create_register(ctx, reg);
    return ctx.builder.load(ctx.int32_type, var_id);
}

static void store_register(TranslationContext& ctx, const maxwell::Register& reg, uint32_t value_id) {
    uint32_t var_id = get_or_create_register(ctx, reg);
    ctx.builder.store(var_id, value_id);
}

// ============================================================================
// Instruction Translation
// ============================================================================

bool translate_instruction(TranslationContext& ctx, const maxwell::DecodedInstruction& inst) {
    using Opcode = maxwell::MaxwellOpcode;

    // Handle predicate
    bool has_predicate = inst.predicate.index != 7;
    uint32_t pred_value = 0;
    if (has_predicate) {
        pred_value = load_register(ctx, inst.predicate);
        if (inst.predicate_neg) {
            // Invert predicate: pred = !pred
            pred_value = ctx.builder.i_equal(ctx.bool_type, pred_value,
                                             ctx.builder.constant_false(ctx.bool_type));
        }
    }

    switch (inst.info.opcode) {
        case Opcode::MOV: {
            if (inst.src_operands.size() >= 1 && inst.dst_operands.size() >= 1) {
                uint32_t src = 0;
                if (inst.src_operands[0].type == maxwell::OperandType::REGISTER) {
                    src = load_register(ctx, inst.src_operands[0].reg);
                } else if (inst.src_operands[0].type == maxwell::OperandType::IMMEDIATE) {
                    src = ctx.builder.constant_int(ctx.int32_type, inst.src_operands[0].immediate_value);
                }
                store_register(ctx, inst.dst_operands[0].reg, src);
            }
            break;
        }

        case Opcode::IADD: {
            if (inst.src_operands.size() >= 2 && inst.dst_operands.size() >= 1) {
                uint32_t src0 = load_register(ctx, inst.src_operands[0].reg);
                uint32_t src1 = load_register(ctx, inst.src_operands[1].reg);
                uint32_t result = ctx.builder.iadd(ctx.int32_type, src0, src1);
                store_register(ctx, inst.dst_operands[0].reg, result);
            }
            break;
        }

        case Opcode::ISUB: {
            if (inst.src_operands.size() >= 2 && inst.dst_operands.size() >= 1) {
                uint32_t src0 = load_register(ctx, inst.src_operands[0].reg);
                uint32_t src1 = load_register(ctx, inst.src_operands[1].reg);
                uint32_t result = ctx.builder.isub(ctx.int32_type, src0, src1);
                store_register(ctx, inst.dst_operands[0].reg, result);
            }
            break;
        }

        case Opcode::IMUL: {
            if (inst.src_operands.size() >= 2 && inst.dst_operands.size() >= 1) {
                uint32_t src0 = load_register(ctx, inst.src_operands[0].reg);
                uint32_t src1 = load_register(ctx, inst.src_operands[1].reg);
                uint32_t result = ctx.builder.imul(ctx.int32_type, src0, src1);
                store_register(ctx, inst.dst_operands[0].reg, result);
            }
            break;
        }

        case Opcode::FADD: {
            if (inst.src_operands.size() >= 2 && inst.dst_operands.size() >= 1) {
                uint32_t src0 = load_register(ctx, inst.src_operands[0].reg);
                uint32_t src1 = load_register(ctx, inst.src_operands[1].reg);
                uint32_t result = ctx.builder.fadd(ctx.float32_type, src0, src1);
                store_register(ctx, inst.dst_operands[0].reg, result);
            }
            break;
        }

        case Opcode::FMUL: {
            if (inst.src_operands.size() >= 2 && inst.dst_operands.size() >= 1) {
                uint32_t src0 = load_register(ctx, inst.src_operands[0].reg);
                uint32_t src1 = load_register(ctx, inst.src_operands[1].reg);
                uint32_t result = ctx.builder.fmul(ctx.float32_type, src0, src1);
                store_register(ctx, inst.dst_operands[0].reg, result);
            }
            break;
        }

        case Opcode::F2I: {
            if (inst.src_operands.size() >= 1 && inst.dst_operands.size() >= 1) {
                uint32_t src = load_register(ctx, inst.src_operands[0].reg);
                uint32_t result = ctx.builder.convert_f_to_s(ctx.int32_type, src);
                store_register(ctx, inst.dst_operands[0].reg, result);
            }
            break;
        }

        case Opcode::I2F: {
            if (inst.src_operands.size() >= 1 && inst.dst_operands.size() >= 1) {
                uint32_t src = load_register(ctx, inst.src_operands[0].reg);
                uint32_t result = ctx.builder.convert_s_to_f(ctx.float32_type, src);
                store_register(ctx, inst.dst_operands[0].reg, result);
            }
            break;
        }

        case Opcode::LDG: {
            if (inst.src_operands.size() >= 1 && inst.dst_operands.size() >= 1) {
                // Load from global memory (storage buffer)
                // This requires setting up descriptor sets and access chains
                // For now, emit a placeholder load
                uint32_t addr = load_register(ctx, inst.src_operands[0].reg);
                // Would need proper buffer access chain here
                store_register(ctx, inst.dst_operands[0].reg, ctx.builder.load(ctx.int32_type, addr));
            }
            break;
        }

        case Opcode::STG: {
            if (inst.src_operands.size() >= 2) {
                // Store to global memory
                uint32_t addr = load_register(ctx, inst.src_operands[0].reg);
                uint32_t value = load_register(ctx, inst.src_operands[1].reg);
                ctx.builder.store(addr, value);
            }
            break;
        }

        case Opcode::TEX:
        case Opcode::TXF: {
            // Texture operations - would need image/sampler setup
            if (inst.dst_operands.size() >= 1) {
                // Placeholder: return zero
                uint32_t zero = ctx.builder.constant_int(ctx.int32_type, 0);
                store_register(ctx, inst.dst_operands[0].reg, zero);
            }
            break;
        }

        case Opcode::BRA: {
            // Branch - would need control flow graph construction
            // For now, just emit a branch to a label
            if (inst.control_flow_target != 0) {
                auto it = ctx.label_map.find(inst.control_flow_target);
                if (it != ctx.label_map.end()) {
                    ctx.builder.branch(it->second);
                }
            }
            break;
        }

        case Opcode::BRX: {
            // Indirect branch - complex, skip for now
            break;
        }

        case Opcode::EXIT:
        case Opcode::RET: {
            ctx.builder.return_inst();
            break;
        }

        case Opcode::BAR: {
            // Barrier
            ctx.builder.control_barrier(
                1,  // Workgroup execution scope
                1,  // Workgroup memory scope
                0   // Memory semantics
            );
            break;
        }

        case Opcode::NOP:
        default: {
            // Unknown or unsupported instruction
            break;
        }
    }

    return true;
}

std::optional<SpvModule> translate_shader(
    const std::vector<maxwell::DecodedInstruction>& instructions,
    NvnShaderStage stage,
    const ShaderTranslationOptions& options) {

    TranslationContext ctx;
    ctx.stage = stage;

    // Initialize builder
    ctx.builder.begin_module();

    // Declare capabilities
    ctx.builder.capability(0);  // Shader
    if (stage == NvnShaderStage::Compute) {
        ctx.builder.capability(10);  // Kernel (for compute)
    }

    // Memory model
    ctx.builder.memory_model(0, 0);  // Logical, GLSL450

    // Create basic types
    ctx.void_type = ctx.builder.type_void();
    ctx.bool_type = ctx.builder.type_bool();
    ctx.int32_type = ctx.builder.type_int(32, 1);
    ctx.uint32_type = ctx.builder.type_int(32, 0);
    ctx.float32_type = ctx.builder.type_float(32);
    ctx.vec4_type = ctx.builder.type_vector(ctx.float32_type, 4);

    // Entry point
    std::string entry_name = "main";
    ctx.entry_point_id = ctx.builder.next_id();
    ctx.builder.function_begin(ctx.void_type, ctx.builder.type_function(ctx.void_type, {}));
    ctx.builder.name(ctx.entry_point_id, entry_name);

    // Execution mode
    ExecutionModel exec_model = ExecutionModel::Fragment;
    if (stage == NvnShaderStage::Vertex) exec_model = ExecutionModel::Vertex;
    else if (stage == NvnShaderStage::Compute) exec_model = ExecutionModel::GLCompute;
    else if (stage == NvnShaderStage::Geometry) exec_model = ExecutionModel::Geometry;
    else if (stage == NvnShaderStage::TessControl) exec_model = ExecutionModel::TessellationControl;
    else if (stage == NvnShaderStage::TessEval) exec_model = ExecutionModel::TessellationEvaluation;

    std::vector<uint32_t> interface_ids;  // Would be filled with input/output variables
    ctx.builder.entry_point(exec_model, ctx.entry_point_id, entry_name, interface_ids);

    if (stage == NvnShaderStage::Compute) {
        ctx.builder.execution_mode(ctx.entry_point_id, 52,  // LocalSize
                                   {ctx.local_size_x, ctx.local_size_y, ctx.local_size_z});
    }

    // Create entry block
    uint32_t entry_label = ctx.builder.label();

    // Translate each instruction
    for (const auto& inst : instructions) {
        translate_instruction(ctx, inst);
    }

    // End function
    ctx.builder.return_inst();
    ctx.builder.function_end();

    // Finalize
    SpvModule module = ctx.builder.end_module();
    return module;
}

} // namespace kirbyrecomp::shader::spirv