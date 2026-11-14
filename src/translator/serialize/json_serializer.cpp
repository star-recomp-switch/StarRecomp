#include "json_serializer.h"

#include <sstream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <optional>
#include <cstdint>

namespace kirbyrecomp::serialize {

//=============================================================================
// Register Name Helpers
//=============================================================================

std::string GetRegisterName(uint8_t index, bool is_64bit) {
    // AArch64 general-purpose registers
    // 64-bit: X0-X30, SP/XZR (31)
    // 32-bit: W0-W30, WSP/WZR (31)
    // Note: PC is not a general-purpose register in AArch64; it's accessed via special instructions.
    // X30 is the Link Register (LR). Some contexts may refer to PC separately.
    if (index > 31) {
        std::ostringstream oss;
        oss << (is_64bit ? "X" : "W") << index;
        return oss.str();
    }

    if (index == 31) {
        // SP (stack pointer) or ZR (zero register) depending on context
        // In most contexts, index 31 with is_64bit=true is SP, and with is_64bit=false is WSP
        // But ZR/WZR is also encoded as 31 - we can't distinguish without more context
        // Default to SP/WSP as that's more common for operand registers
        return is_64bit ? "SP" : "WSP";
    }

    if (index == 30) {
        // X30/W30 is the Link Register (LR), sometimes called LR
        // In some debugging contexts, it's useful to show "LR" instead of "X30"/"W30"
        std::ostringstream oss;
        oss << (is_64bit ? "X" : "W") << static_cast<int>(index);
        return oss.str();  // Could return "LR"/"WLR" if preferred
    }

    // X0-X29 / W0-W29
    std::ostringstream oss;
    oss << (is_64bit ? "X" : "W") << static_cast<int>(index);
    return oss.str();
}

std::string GetVectorRegisterName(uint8_t index, uint8_t bit_width) {
    // AArch64 vector/SIMD registers (V0-V31)
    // Different views: Q (128-bit), D (64-bit), S (32-bit), H (16-bit), B (8-bit)
    if (index > 31) {
        std::ostringstream oss;
        oss << "V" << index;
        return oss.str();
    }

    const char* prefix = "V";
    switch (bit_width) {
        case 128: prefix = "Q"; break;
        case 64:  prefix = "D"; break;
        case 32:  prefix = "S"; break;
        case 16:  prefix = "H"; break;
        case 8:   prefix = "B"; break;
        default:  prefix = "V"; break;
    }

    std::ostringstream oss;
    oss << prefix << static_cast<int>(index);
    return oss.str();
}

bool IsSpecialRegister(uint8_t index, bool is_64bit) {
    // Index 31 is either SP/ZR (64-bit) or WSP/WZR (32-bit)
    return index == 31;
}

//=============================================================================
// Immediate Value Helpers
//=============================================================================

ImmediateValue::Type InferImmediateType(const ir::IrImmediate& imm) {
    // Infer type based on bit_width and is_signed
    switch (imm.bit_width) {
        case 8:
        case 16:
        case 32:
            return imm.is_signed ? ImmediateValue::Type::I64 : ImmediateValue::Type::U64;
        case 64:
            return imm.is_signed ? ImmediateValue::Type::I64 : ImmediateValue::Type::U64;
        default:
            return ImmediateValue::Type::Unknown;
    }
}

ImmediateValue ConvertImmediate(const ir::IrImmediate& imm) {
    ImmediateValue result;
    result.value = imm.value;
    result.type = InferImmediateType(imm);
    return result;
}

ir::IrImmediate ConvertImmediate(const ImmediateValue& imm) {
    ir::IrImmediate result;
    result.value = imm.value;

    switch (imm.type) {
        case ImmediateValue::Type::U64:
            result.is_signed = false;
            result.bit_width = 64;
            break;
        case ImmediateValue::Type::I64:
            result.is_signed = true;
            result.bit_width = 64;
            break;
        case ImmediateValue::Type::F32:
            result.is_signed = true; // FP values are treated as signed
            result.bit_width = 32;
            break;
        case ImmediateValue::Type::F64:
            result.is_signed = true;
            result.bit_width = 64;
            break;
        default:
            result.is_signed = false;
            result.bit_width = 64;
            break;
    }
    return result;
}

//=============================================================================
// Operand Serialization
//=============================================================================

static nlohmann::json SerializeOperand(const ir::IrOperand& op, const JsonSerializerOptions& opts) {
    nlohmann::json j;
    j["type"] = static_cast<uint8_t>(op.type);

    switch (op.type) {
        case ir::IrOperandType::Register: {
            const auto* reg = op.AsRegister();
            if (reg) {
                nlohmann::json reg_json;
                reg_json["index"] = reg->index;
                reg_json["is_64bit"] = reg->is_64bit;
                if (opts.use_register_names) {
                    reg_json["name"] = GetRegisterName(reg->index, reg->is_64bit);
                }
                j["value"] = reg_json;
            }
            break;
        }
        case ir::IrOperandType::Immediate: {
            const auto* imm = op.AsImmediate();
            if (imm) {
                nlohmann::json imm_json;
                imm_json["value"] = imm->value;
                imm_json["is_signed"] = imm->is_signed;
                imm_json["bit_width"] = imm->bit_width;
                if (opts.include_immediate_types) {
                    auto typed = ConvertImmediate(*imm);
                    switch (typed.type) {
                        case ImmediateValue::Type::U64: imm_json["value_type"] = "u64"; break;
                        case ImmediateValue::Type::I64: imm_json["value_type"] = "i64"; break;
                        case ImmediateValue::Type::F32: imm_json["value_type"] = "f32"; break;
                        case ImmediateValue::Type::F64: imm_json["value_type"] = "f64"; break;
                        default: imm_json["value_type"] = "unknown"; break;
                    }
                    // Also include typed value for convenience
                    if (typed.type == ImmediateValue::Type::F32) {
                        if (auto f = typed.AsF32()) imm_json["float_value"] = *f;
                    } else if (typed.type == ImmediateValue::Type::F64) {
                        if (auto d = typed.AsF64()) imm_json["double_value"] = *d;
                    } else if (typed.type == ImmediateValue::Type::I64) {
                        if (auto i = typed.AsI64()) imm_json["signed_value"] = *i;
                    } else {
                        if (auto u = typed.AsU64()) imm_json["unsigned_value"] = *u;
                    }
                }
                j["value"] = imm_json;
            }
            break;
        }
        case ir::IrOperandType::Memory: {
            const auto* mem = op.AsMemory();
            if (mem) {
                nlohmann::json mem_json;
                mem_json["base_register"] = mem->base_register;
                mem_json["offset"] = mem->offset;
                mem_json["size"] = mem->size;
                mem_json["is_signed"] = mem->is_signed;
                if (opts.use_register_names) {
                    mem_json["base_register_name"] = GetRegisterName(mem->base_register, true);
                }
                if (mem->index_register.has_value()) {
                    mem_json["index_register"] = mem->index_register.value();
                    if (opts.use_register_names) {
                        mem_json["index_register_name"] = GetRegisterName(mem->index_register.value(), true);
                    }
                }
                j["value"] = mem_json;
            }
            break;
        }
        case ir::IrOperandType::Label: {
            const auto* label = op.AsLabel();
            if (label) {
                nlohmann::json label_json;
                label_json["name"] = label->name;
                label_json["address"] = label->address;
                j["value"] = label_json;
            }
            break;
        }
    }

    return j;
}

static ir::IrOperand DeserializeOperand(const nlohmann::json& j) {
    ir::IrOperand op;
    uint8_t type_val;
    j.at("type").get_to(type_val);
    op.type = static_cast<ir::IrOperandType>(type_val);

    const auto& value = j.at("value");

    switch (op.type) {
        case ir::IrOperandType::Register: {
            ir::IrRegister reg;
            value.at("index").get_to(reg.index);
            value.at("is_64bit").get_to(reg.is_64bit);
            op = ir::IrOperand(reg);
            break;
        }
        case ir::IrOperandType::Immediate: {
            ir::IrImmediate imm;
            value.at("value").get_to(imm.value);
            value.at("is_signed").get_to(imm.is_signed);
            value.at("bit_width").get_to(imm.bit_width);
            op = ir::IrOperand(imm);
            break;
        }
        case ir::IrOperandType::Memory: {
            ir::IrMemory mem;
            value.at("base_register").get_to(mem.base_register);
            value.at("offset").get_to(mem.offset);
            value.at("size").get_to(mem.size);
            value.at("is_signed").get_to(mem.is_signed);
            if (value.contains("index_register")) {
                mem.index_register = value.at("index_register").get<uint8_t>();
            }
            op = ir::IrOperand(mem);
            break;
        }
        case ir::IrOperandType::Label: {
            ir::IrLabel label;
            value.at("name").get_to(label.name);
            value.at("address").get_to(label.address);
            op = ir::IrOperand(label);
            break;
        }
    }

    return op;
}

//=============================================================================
// Instruction Serialization
//=============================================================================

nlohmann::json to_json(const ir::IrInstruction& insn, const JsonSerializerOptions& opts) {
    nlohmann::json j;
    j["opcode"] = static_cast<uint16_t>(insn.opcode);

    if (opts.include_opcode_names) {
        j["opcode_name"] = ir::IrOpcodeToString(insn.opcode);
    }

    // Serialize operands
    nlohmann::json operands_json = nlohmann::json::array();
    for (const auto& operand : insn.operands) {
        operands_json.push_back(SerializeOperand(operand, opts));
    }
    j["operands"] = operands_json;

    if (opts.include_metadata) {
        if (insn.result_register.has_value()) {
            nlohmann::json reg_json;
            reg_json["index"] = insn.result_register.value();
            if (opts.use_register_names) {
                reg_json["name"] = GetRegisterName(insn.result_register.value(), true);
            }
            j["result_register"] = reg_json;
        }
        j["flags_affected"] = insn.flags_affected;
    }

    if (opts.include_addresses) {
        j["address"] = insn.address;
    }

    return j;
}

ir::IrInstruction from_json_instruction(const nlohmann::json& j, const JsonSerializerOptions&) {
    ir::IrInstruction insn;

    uint16_t opcode_val;
    j.at("opcode").get_to(opcode_val);
    insn.opcode = static_cast<ir::IrOpcode>(opcode_val);

    // Deserialize operands
    if (j.contains("operands")) {
        for (const auto& op_json : j.at("operands")) {
            insn.operands.push_back(DeserializeOperand(op_json));
        }
    }

    if (j.contains("result_register")) {
        const auto& reg_json = j.at("result_register");
        if (reg_json.is_number()) {
            insn.result_register = reg_json.get<uint8_t>();
        } else if (reg_json.is_object() && reg_json.contains("index")) {
            insn.result_register = reg_json.at("index").get<uint8_t>();
        }
    }

    if (j.contains("flags_affected")) {
        j.at("flags_affected").get_to(insn.flags_affected);
    }

    if (j.contains("address")) {
        j.at("address").get_to(insn.address);
    }

    return insn;
}

//=============================================================================
// Function Serialization
//=============================================================================

nlohmann::json to_json(const ir::IrFunction& func, const JsonSerializerOptions& opts) {
    nlohmann::json j;
    j["name"] = func.name;

    if (opts.include_addresses) {
        j["address"] = func.address;
    }

    // Serialize instructions
    nlohmann::json instructions_json = nlohmann::json::array();
    for (const auto& insn : func.instructions) {
        instructions_json.push_back(to_json(insn, opts));
    }
    j["instructions"] = instructions_json;

    if (opts.include_metadata) {
        // CFG edges
        nlohmann::json edges_json = nlohmann::json::array();
        for (const auto& edge : func.cfg_edges) {
            nlohmann::json edge_json;
            edge_json["from_block"] = edge.from_block;
            edge_json["to_block"] = edge.to_block;
            edge_json["type"] = static_cast<uint8_t>(edge.type);
            edges_json.push_back(edge_json);
        }
        j["cfg_edges"] = edges_json;

        j["block_starts"] = func.block_starts;
        j["stack_size"] = func.stack_size;
        j["is_thumb"] = func.is_thumb;
        j["imported_symbols"] = func.imported_symbols;
    }

    return j;
}

ir::IrFunction from_json_function(const nlohmann::json& j, const JsonSerializerOptions& opts) {
    ir::IrFunction func;

    j.at("name").get_to(func.name);

    if (j.contains("address")) {
        j.at("address").get_to(func.address);
    }

    // Deserialize instructions
    if (j.contains("instructions")) {
        for (const auto& insn_json : j.at("instructions")) {
            func.instructions.push_back(from_json_instruction(insn_json, opts));
        }
    }

    if (j.contains("cfg_edges")) {
        for (const auto& edge_json : j.at("cfg_edges")) {
            ir::IrCfgEdge edge;
            edge_json.at("from_block").get_to(edge.from_block);
            edge_json.at("to_block").get_to(edge.to_block);
            uint8_t type_val;
            edge_json.at("type").get_to(type_val);
            edge.type = static_cast<ir::IrCfgEdge::Type>(type_val);
            func.cfg_edges.push_back(edge);
        }
    }

    if (j.contains("block_starts")) {
        j.at("block_starts").get_to(func.block_starts);
    }
    if (j.contains("stack_size")) {
        j.at("stack_size").get_to(func.stack_size);
    }
    if (j.contains("is_thumb")) {
        j.at("is_thumb").get_to(func.is_thumb);
    }
    if (j.contains("imported_symbols")) {
        j.at("imported_symbols").get_to(func.imported_symbols);
    }

    return func;
}

//=============================================================================
// Global Serialization
//=============================================================================

static nlohmann::json SerializeGlobal(const ir::IrGlobal& global, const JsonSerializerOptions& opts) {
    nlohmann::json j;
    j["name"] = global.name;

    if (opts.include_addresses) {
        j["address"] = global.address;
    }

    if (opts.include_metadata) {
        j["size"] = global.size;
        j["is_readonly"] = global.is_readonly;
        j["section"] = global.section;
        // Note: data is serialized as array of integers (byte values)
        j["data"] = global.data;
    }

    return j;
}

static ir::IrGlobal DeserializeGlobal(const nlohmann::json& j) {
    ir::IrGlobal global;
    j.at("name").get_to(global.name);

    if (j.contains("address")) {
        j.at("address").get_to(global.address);
    }
    if (j.contains("size")) {
        j.at("size").get_to(global.size);
    }
    if (j.contains("is_readonly")) {
        j.at("is_readonly").get_to(global.is_readonly);
    }
    if (j.contains("section")) {
        j.at("section").get_to(global.section);
    }
    if (j.contains("data")) {
        j.at("data").get_to(global.data);
    }

    return global;
}

//=============================================================================
// Module Serialization
//=============================================================================

nlohmann::json to_json(const ir::IrModule& mod, const JsonSerializerOptions& opts) {
    nlohmann::json j;
    j["name"] = mod.name;

    if (opts.include_addresses) {
        j["base_address"] = mod.base_address;
    }

    if (opts.include_metadata) {
        j["architecture"] = mod.architecture;
        j["endianness"] = mod.endianness;
    }

    // Serialize functions
    nlohmann::json functions_json = nlohmann::json::array();
    for (const auto& func : mod.functions) {
        functions_json.push_back(to_json(func, opts));
    }
    j["functions"] = functions_json;

    // Serialize globals
    nlohmann::json globals_json = nlohmann::json::array();
    for (const auto& global : mod.globals) {
        globals_json.push_back(SerializeGlobal(global, opts));
    }
    j["globals"] = globals_json;

    return j;
}

ir::IrModule from_json_module(const nlohmann::json& j, const JsonSerializerOptions& opts) {
    ir::IrModule mod;

    j.at("name").get_to(mod.name);

    if (j.contains("base_address")) {
        j.at("base_address").get_to(mod.base_address);
    }
    if (j.contains("architecture")) {
        j.at("architecture").get_to(mod.architecture);
    }
    if (j.contains("endianness")) {
        j.at("endianness").get_to(mod.endianness);
    }

    // Deserialize functions
    if (j.contains("functions")) {
        for (const auto& func_json : j.at("functions")) {
            mod.functions.push_back(from_json_function(func_json, opts));
        }
    }

    // Deserialize globals
    if (j.contains("globals")) {
        for (const auto& global_json : j.at("globals")) {
            mod.globals.push_back(DeserializeGlobal(global_json));
        }
    }

    return mod;
}

//=============================================================================
// High-level Serialization API
//=============================================================================

std::string SerializeToJson(const ir::IrModule& mod, const JsonSerializerOptions& opts) {
    nlohmann::json j = to_json(mod, opts);
    return j.dump(opts.pretty_print ? opts.indent : -1);
}

bool SerializeToFile(const ir::IrModule& mod, const std::string& filepath, const JsonSerializerOptions& opts) {
    try {
        std::ofstream file(filepath);
        if (!file) return false;
        file << SerializeToJson(mod, opts);
        return true;
    } catch (...) {
        return false;
    }
}

ir::IrModule DeserializeFromJson(const std::string& json_str, const JsonSerializerOptions& opts) {
    nlohmann::json j = nlohmann::json::parse(json_str);
    return from_json_module(j, opts);
}

std::optional<ir::IrModule> DeserializeFromFile(const std::string& filepath, const JsonSerializerOptions& opts) {
    try {
        std::ifstream file(filepath);
        if (!file) return std::nullopt;
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        return DeserializeFromJson(content, opts);
    } catch (...) {
        return std::nullopt;
    }
}

//=============================================================================
// Round-trip Testing
//=============================================================================

bool TestRoundTrip(const ir::IrInstruction& insn, const JsonSerializerOptions& opts) {
    try {
        auto json = to_json(insn, opts);
        auto restored = from_json_instruction(json, opts);

        // Compare key fields
        if (insn.opcode != restored.opcode) return false;
        if (insn.flags_affected != restored.flags_affected) return false;
        if (insn.address != restored.address) return false;
        if (insn.result_register != restored.result_register) return false;
        if (insn.operands.size() != restored.operands.size()) return false;

        for (size_t i = 0; i < insn.operands.size(); ++i) {
            const auto& a = insn.operands[i];
            const auto& b = restored.operands[i];
            if (a.type != b.type) return false;

            // Compare operand values based on type
            switch (a.type) {
                case ir::IrOperandType::Register: {
                    const auto* ra = a.AsRegister();
                    const auto* rb = b.AsRegister();
                    if (!ra || !rb) return false;
                    if (ra->index != rb->index || ra->is_64bit != rb->is_64bit) return false;
                    break;
                }
                case ir::IrOperandType::Immediate: {
                    const auto* ia = a.AsImmediate();
                    const auto* ib = b.AsImmediate();
                    if (!ia || !ib) return false;
                    if (ia->value != ib->value || ia->is_signed != ib->is_signed || ia->bit_width != ib->bit_width) return false;
                    break;
                }
                case ir::IrOperandType::Memory: {
                    const auto* ma = a.AsMemory();
                    const auto* mb = b.AsMemory();
                    if (!ma || !mb) return false;
                    if (ma->base_register != mb->base_register || ma->offset != mb->offset ||
                        ma->size != mb->size || ma->is_signed != mb->is_signed ||
                        ma->index_register != mb->index_register) return false;
                    break;
                }
                case ir::IrOperandType::Label: {
                    const auto* la = a.AsLabel();
                    const auto* lb = b.AsLabel();
                    if (!la || !lb) return false;
                    if (la->name != lb->name || la->address != lb->address) return false;
                    break;
                }
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool TestRoundTrip(const ir::IrFunction& func, const JsonSerializerOptions& opts) {
    try {
        auto json = to_json(func, opts);
        auto restored = from_json_function(json, opts);

        if (func.name != restored.name) return false;
        if (func.address != restored.address) return false;
        if (func.stack_size != restored.stack_size) return false;
        if (func.is_thumb != restored.is_thumb) return false;
        if (func.imported_symbols != restored.imported_symbols) return false;
        if (func.block_starts != restored.block_starts) return false;
        if (func.cfg_edges.size() != restored.cfg_edges.size()) return false;
        if (func.instructions.size() != restored.instructions.size()) return false;

        for (size_t i = 0; i < func.cfg_edges.size(); ++i) {
            const auto& a = func.cfg_edges[i];
            const auto& b = restored.cfg_edges[i];
            if (a.from_block != b.from_block || a.to_block != b.to_block || a.type != b.type) return false;
        }

        for (size_t i = 0; i < func.instructions.size(); ++i) {
            if (!TestRoundTrip(func.instructions[i], opts)) return false;
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool TestRoundTrip(const ir::IrModule& mod, const JsonSerializerOptions& opts) {
    try {
        auto json = to_json(mod, opts);
        auto restored = from_json_module(json, opts);

        if (mod.name != restored.name) return false;
        if (mod.base_address != restored.base_address) return false;
        if (mod.architecture != restored.architecture) return false;
        if (mod.endianness != restored.endianness) return false;
        if (mod.functions.size() != restored.functions.size()) return false;
        if (mod.globals.size() != restored.globals.size()) return false;

        for (size_t i = 0; i < mod.functions.size(); ++i) {
            if (!TestRoundTrip(mod.functions[i], opts)) return false;
        }

        for (size_t i = 0; i < mod.globals.size(); ++i) {
            const auto& a = mod.globals[i];
            const auto& b = restored.globals[i];
            if (a.name != b.name || a.address != b.address || a.size != b.size ||
                a.is_readonly != b.is_readonly || a.section != b.section || a.data != b.data) {
                return false;
            }
        }

        return true;
    } catch (...) {
        return false;
    }
}

//=============================================================================
// Validation
//=============================================================================

bool ValidateJson(const nlohmann::json& j, std::string* error_out) {
    try {
        // Check required top-level fields
        if (!j.contains("name") || !j["name"].is_string()) {
            if (error_out) *error_out = "Missing or invalid 'name' field";
            return false;
        }

        // Check functions array
        if (j.contains("functions")) {
            if (!j["functions"].is_array()) {
                if (error_out) *error_out = "'functions' must be an array";
                return false;
            }
            for (size_t i = 0; i < j["functions"].size(); ++i) {
                const auto& func = j["functions"][i];
                if (!func.contains("name") || !func["name"].is_string()) {
                    if (error_out) *error_out = "Function at index " + std::to_string(i) + " missing 'name'";
                    return false;
                }
                if (!func.contains("instructions") || !func["instructions"].is_array()) {
                    if (error_out) *error_out = "Function '" + func["name"].get<std::string>() + "' missing 'instructions' array";
                    return false;
                }
            }
        }

        // Check globals array
        if (j.contains("globals")) {
            if (!j["globals"].is_array()) {
                if (error_out) *error_out = "'globals' must be an array";
                return false;
            }
        }

        return true;
    } catch (const std::exception& e) {
        if (error_out) *error_out = "JSON validation error: " + std::string(e.what());
        return false;
    }
}

} // namespace kirbyrecomp::serialize