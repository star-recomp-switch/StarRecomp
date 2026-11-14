#pragma once

#include <string>
#include <optional>
#include <vector>
#include <cstdint>
#include <cstring>

#include <nlohmann/json.hpp>

#include "../ir.h"

namespace kirbyrecomp::serialize {

//=============================================================================
// JSON Serialization Options
//=============================================================================

struct JsonSerializerOptions {
    bool pretty_print = true;           // Pretty-print with indentation
    int indent = 2;                     // Indentation spaces (when pretty_print=true)
    bool include_opcode_names = true;   // Include opcode string names alongside enum values
    bool use_register_names = true;     // Use X0-X30, SP, PC, V0-V31, W0-W30 names instead of indices
    bool include_immediate_types = true; // Include type info for immediates (u64, i64, f32, f64)
    bool include_addresses = true;      // Include original guest addresses
    bool include_metadata = true;       // Include metadata like flags_affected, stack_size, etc.
};

//=============================================================================
// Register Name Helpers
//=============================================================================

// Get register name for AArch64 general-purpose registers
// Returns names like "X0"-"X30", "SP", "XZR" for 64-bit
// Returns names like "W0"-"W30", "WSP", "WZR" for 32-bit
// Note: PC is not a general-purpose register in AArch64; X30 is LR (Link Register)
std::string GetRegisterName(uint8_t index, bool is_64bit);

// Get register name for AArch64 vector/SIMD registers
// Returns names like "V0"-"V31", "Q0"-"Q31", "D0"-"D31", "S0"-"S31", "H0"-"H31", "B0"-"B31"
std::string GetVectorRegisterName(uint8_t index, uint8_t bit_width);

// Check if a register index represents SP (stack pointer) or ZR (zero register)
bool IsSpecialRegister(uint8_t index, bool is_64bit);

//=============================================================================
// Immediate Value Helpers
//=============================================================================

// Immediate value representation with type information
struct ImmediateValue {
    uint64_t value = 0;
    enum class Type : uint8_t {
        U64,
        I64,
        F32,
        F64,
        Unknown
    } type = Type::Unknown;

    // Factory methods
    static ImmediateValue MakeU64(uint64_t v) { return {v, Type::U64}; }
    static ImmediateValue MakeI64(int64_t v) { return {static_cast<uint64_t>(v), Type::I64}; }
    static ImmediateValue MakeF32(float v) {
        uint32_t bits;
        std::memcpy(&bits, &v, sizeof(float));
        return {bits, Type::F32};
    }
    static ImmediateValue MakeF64(double v) {
        uint64_t bits;
        std::memcpy(&bits, &v, sizeof(double));
        return {bits, Type::F64};
    }

    // Convert back to native types
    std::optional<uint64_t> AsU64() const { return (type == Type::U64) ? std::optional<uint64_t>(value) : std::nullopt; }
    std::optional<int64_t> AsI64() const { return (type == Type::I64) ? std::optional<int64_t>(static_cast<int64_t>(value)) : std::nullopt; }
    std::optional<float> AsF32() const {
        if (type != Type::F32) return std::nullopt;
        float v; std::memcpy(&v, &value, sizeof(float)); return v;
    }
    std::optional<double> AsF64() const {
        if (type != Type::F64) return std::nullopt;
        double v; std::memcpy(&v, &value, sizeof(double)); return v;
    }
};

// Infer immediate type from IrImmediate
ImmediateValue::Type InferImmediateType(const ir::IrImmediate& imm);

// Convert IrImmediate to typed ImmediateValue
ImmediateValue ConvertImmediate(const ir::IrImmediate& imm);

// Convert typed ImmediateValue back to IrImmediate
ir::IrImmediate ConvertImmediate(const ImmediateValue& imm);

//=============================================================================
// Main Serialization Functions
//=============================================================================

// Serialize IrInstruction to JSON
nlohmann::json to_json(const ir::IrInstruction& insn, const JsonSerializerOptions& opts = {});

// Serialize IrFunction to JSON
nlohmann::json to_json(const ir::IrFunction& func, const JsonSerializerOptions& opts = {});

// Serialize IrModule to JSON
nlohmann::json to_json(const ir::IrModule& mod, const JsonSerializerOptions& opts = {});

// Serialize IrModule to JSON string
std::string SerializeToJson(const ir::IrModule& mod, const JsonSerializerOptions& opts = {});

// Serialize IrModule to file
bool SerializeToFile(const ir::IrModule& mod, const std::string& filepath, const JsonSerializerOptions& opts = {});

//=============================================================================
// Deserialization Functions (Round-trip)
//=============================================================================

// Deserialize IrInstruction from JSON
ir::IrInstruction from_json_instruction(const nlohmann::json& j, const JsonSerializerOptions& opts = {});

// Deserialize IrFunction from JSON
ir::IrFunction from_json_function(const nlohmann::json& j, const JsonSerializerOptions& opts = {});

// Deserialize IrModule from JSON
ir::IrModule from_json_module(const nlohmann::json& j, const JsonSerializerOptions& opts = {});

// Deserialize IrModule from JSON string
ir::IrModule DeserializeFromJson(const std::string& json_str, const JsonSerializerOptions& opts = {});

// Deserialize IrModule from file
std::optional<ir::IrModule> DeserializeFromFile(const std::string& filepath, const JsonSerializerOptions& opts = {});

//=============================================================================
// Round-trip Testing Helpers
//=============================================================================

// Test round-trip serialization for IrInstruction
bool TestRoundTrip(const ir::IrInstruction& insn, const JsonSerializerOptions& opts = {});

// Test round-trip serialization for IrFunction
bool TestRoundTrip(const ir::IrFunction& func, const JsonSerializerOptions& opts = {});

// Test round-trip serialization for IrModule
bool TestRoundTrip(const ir::IrModule& mod, const JsonSerializerOptions& opts = {});

//=============================================================================
// Utility Functions
//=============================================================================

// Validate JSON structure for IrModule
bool ValidateJson(const nlohmann::json& j, std::string* error_out = nullptr);

} // namespace kirbyrecomp::serialize