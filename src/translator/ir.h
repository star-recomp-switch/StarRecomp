#pragma once

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <cstdint>
#include <fstream>

#include <nlohmann/json.hpp>

namespace kirbyrecomp::ir {

//=============================================================================
// IR Operand Types
//=============================================================================

enum class IrOperandType : uint8_t {
    Register,
    Immediate,
    Memory,
    Label
};

struct IrRegister {
    uint8_t index;  // 0-30 for general purpose, 31 for SP/ZR
    bool is_64bit;  // true = X register (64-bit), false = W register (32-bit)
};

struct IrImmediate {
    uint64_t value;
    bool is_signed;
    uint8_t bit_width;  // 8, 16, 32, 64
};

struct IrMemory {
    uint8_t base_register;    // base register index
    std::optional<uint8_t> index_register;  // optional index register
    int64_t offset;           // signed offset
    uint8_t size;             // access size in bytes (1, 2, 4, 8, 16)
    bool is_signed;           // for sign-extending loads
};

struct IrLabel {
    std::string name;
    uint64_t address;  // resolved address (0 if unresolved)
};

using IrOperandValue = std::variant<IrRegister, IrImmediate, IrMemory, IrLabel>;

struct IrOperand {
    IrOperandType type;
    IrOperandValue value;

    // Constructors for each operand type
    IrOperand() : type(IrOperandType::Register), value(IrRegister{0, true}) {}

    explicit IrOperand(const IrRegister& reg)
        : type(IrOperandType::Register), value(reg) {}

    explicit IrOperand(const IrImmediate& imm)
        : type(IrOperandType::Immediate), value(imm) {}

    explicit IrOperand(const IrMemory& mem)
        : type(IrOperandType::Memory), value(mem) {}

    explicit IrOperand(const IrLabel& label)
        : type(IrOperandType::Label), value(label) {}

    // Convenience factory methods
    static IrOperand Reg(uint8_t index, bool is_64bit = true) {
        return IrOperand(IrRegister{index, is_64bit});
    }

    static IrOperand Imm(uint64_t value, uint8_t bit_width = 64, bool is_signed = false) {
        return IrOperand(IrImmediate{value, is_signed, bit_width});
    }

    static IrOperand Mem(uint8_t base, int64_t offset, uint8_t size,
                         std::optional<uint8_t> index = std::nullopt, bool is_signed = false) {
        return IrOperand(IrMemory{base, index, offset, size, is_signed});
    }

    static IrOperand Label(const std::string& name, uint64_t address = 0) {
        return IrOperand(IrLabel{name, address});
    }

    // Type-safe accessors
    const IrRegister* AsRegister() const {
        return std::get_if<IrRegister>(&value);
    }

    const IrImmediate* AsImmediate() const {
        return std::get_if<IrImmediate>(&value);
    }

    const IrMemory* AsMemory() const {
        return std::get_if<IrMemory>(&value);
    }

    const IrLabel* AsLabel() const {
        return std::get_if<IrLabel>(&value);
    }
};

//=============================================================================
// IR Opcodes (subset of SwitchRecomp AArch64)
//=============================================================================

enum class IrOpcode : uint16_t {
    // Invalid/placeholder
    Invalid = 0,

    // Integer ALU
    Add,    // add rd, rn, rm
    Sub,    // sub rd, rn, rm
    And,    // and rd, rn, rm
    Orr,    // orr rd, rn, rm
    Eor,    // eor rd, rn, rm
    Lsl,    // lsl rd, rn, #shift
    Lsr,    // lsr rd, rn, #shift
    Asr,    // asr rd, rn, #shift
    Mul,    // mul rd, rn, rm
    Udiv,   // udiv rd, rn, rm
    Sdiv,   // sdiv rd, rn, rm

    // Load/Store
    Ldr,    // ldr rt, [rn, #offset]
    Str,    // str rt, [rn, #offset]
    Ldp,    // ldp rt1, rt2, [rn, #offset]
    Stp,    // stp rt1, rt2, [rn, #offset]

    // Branches
    B,      // b label
    Bl,     // bl label
    Br,     // br xn
    Blr,    // blr xn
    Cbz,    // cbz wn, label
    Cbnz,   // cbnz wn, label
    Tbz,    // tbz wn, #bit, label
    Tbnz,   // tbnz wn, #bit, label

    // Compare
    Cmp,    // cmp rn, rm
    Cmn,    // cmn rn, rm
    Tst,    // tst rn, rm

    // Move
    Mov,    // mov rd, rn
    Movk,   // movk rd, #imm, lsl #shift
    Movn,   // movn rd, #imm, lsl #shift
    Movz,   // movz rd, #imm, lsl #shift

    // System
    Svc,    // svc #imm
    Hvc,    // hvc #imm
    Smc,    // smc #imm
    Mrs,    // mrs rd, sysreg
    Msr,    // msr sysreg, rn

    // Control flow
    Return, // ret
};

// Get string name for opcode (for debugging/serialization)
inline const char* IrOpcodeToString(IrOpcode opcode) {
    switch (opcode) {
        case IrOpcode::Invalid: return "invalid";
        case IrOpcode::Add: return "add";
        case IrOpcode::Sub: return "sub";
        case IrOpcode::And: return "and";
        case IrOpcode::Orr: return "orr";
        case IrOpcode::Eor: return "eor";
        case IrOpcode::Lsl: return "lsl";
        case IrOpcode::Lsr: return "lsr";
        case IrOpcode::Asr: return "asr";
        case IrOpcode::Mul: return "mul";
        case IrOpcode::Udiv: return "udiv";
        case IrOpcode::Sdiv: return "sdiv";
        case IrOpcode::Ldr: return "ldr";
        case IrOpcode::Str: return "str";
        case IrOpcode::Ldp: return "ldp";
        case IrOpcode::Stp: return "stp";
        case IrOpcode::B: return "b";
        case IrOpcode::Bl: return "bl";
        case IrOpcode::Br: return "br";
        case IrOpcode::Blr: return "blr";
        case IrOpcode::Cbz: return "cbz";
        case IrOpcode::Cbnz: return "cbnz";
        case IrOpcode::Tbz: return "tbz";
        case IrOpcode::Tbnz: return "tbnz";
        case IrOpcode::Cmp: return "cmp";
        case IrOpcode::Cmn: return "cmn";
        case IrOpcode::Tst: return "tst";
        case IrOpcode::Mov: return "mov";
        case IrOpcode::Movk: return "movk";
        case IrOpcode::Movn: return "movn";
        case IrOpcode::Movz: return "movz";
        case IrOpcode::Svc: return "svc";
        case IrOpcode::Hvc: return "hvc";
        case IrOpcode::Smc: return "smc";
        case IrOpcode::Mrs: return "mrs";
        case IrOpcode::Msr: return "msr";
        case IrOpcode::Return: return "ret";
    }
    return "unknown";
}

//=============================================================================
// IR Instruction
//=============================================================================

struct IrInstruction {
    IrOpcode opcode = IrOpcode::Invalid;
    std::vector<IrOperand> operands;
    std::optional<uint8_t> result_register;  // destination register index (if any)
    bool flags_affected = false;              // whether instruction updates NZCV flags
    uint64_t address = 0;                     // original guest address

    IrInstruction() = default;

    explicit IrInstruction(IrOpcode op) : opcode(op) {}

    IrInstruction(IrOpcode op, std::vector<IrOperand> ops,
                  std::optional<uint8_t> result = std::nullopt,
                  bool flags = false, uint64_t addr = 0)
        : opcode(op), operands(std::move(ops)), result_register(result),
          flags_affected(flags), address(addr) {}
};

//=============================================================================
// IR Function
//=============================================================================

struct IrCfgEdge {
    uint32_t from_block;
    uint32_t to_block;
    enum class Type : uint8_t {
        Fallthrough,
        Branch,
        CondBranchTrue,
        CondBranchFalse,
        Call,
        Return,
        Indirect
    } type = Type::Fallthrough;
};

struct IrFunction {
    std::string name;
    uint64_t address = 0;                     // guest entry address
    std::vector<IrInstruction> instructions;  // linear instruction list
    std::vector<IrCfgEdge> cfg_edges;         // control flow edges
    std::vector<uint32_t> block_starts;       // instruction indices where blocks start
    uint32_t stack_size = 0;                  // required stack frame size
    bool is_thumb = false;                    // thumb mode (for ARM32 compat)
    std::vector<std::string> imported_symbols; // external symbols referenced
};

//=============================================================================
// IR Module
//=============================================================================

struct IrGlobal {
    std::string name;
    uint64_t address = 0;
    uint64_t size = 0;
    bool is_readonly = false;
    std::vector<uint8_t> data;  // initial data (if any)
    std::string section;        // section name (e.g., ".text", ".data", ".rodata")
};

struct IrModule {
    std::string name;
    std::vector<IrFunction> functions;
    std::vector<IrGlobal> globals;
    uint64_t base_address = 0;       // module load address
    std::string architecture = "aarch64";
    std::string endianness = "little";
};

//=============================================================================
// JSON Serialization (nlohmann/json)
//=============================================================================

// IrRegister
inline void to_json(nlohmann::json& j, const IrRegister& r) {
    j = nlohmann::json{
        {"index", r.index},
        {"is_64bit", r.is_64bit}
    };
}

inline void from_json(const nlohmann::json& j, IrRegister& r) {
    j.at("index").get_to(r.index);
    j.at("is_64bit").get_to(r.is_64bit);
}

// IrImmediate
inline void to_json(nlohmann::json& j, const IrImmediate& i) {
    j = nlohmann::json{
        {"value", i.value},
        {"is_signed", i.is_signed},
        {"bit_width", i.bit_width}
    };
}

inline void from_json(const nlohmann::json& j, IrImmediate& i) {
    j.at("value").get_to(i.value);
    j.at("is_signed").get_to(i.is_signed);
    j.at("bit_width").get_to(i.bit_width);
}

// IrMemory
inline void to_json(nlohmann::json& j, const IrMemory& m) {
    j = nlohmann::json{
        {"base_register", m.base_register},
        {"offset", m.offset},
        {"size", m.size},
        {"is_signed", m.is_signed}
    };
    if (m.index_register.has_value()) {
        j["index_register"] = m.index_register.value();
    }
}

inline void from_json(const nlohmann::json& j, IrMemory& m) {
    j.at("base_register").get_to(m.base_register);
    j.at("offset").get_to(m.offset);
    j.at("size").get_to(m.size);
    j.at("is_signed").get_to(m.is_signed);
    if (j.contains("index_register")) {
        m.index_register = j.at("index_register").get<uint8_t>();
    }
}

// IrLabel
inline void to_json(nlohmann::json& j, const IrLabel& l) {
    j = nlohmann::json{
        {"name", l.name},
        {"address", l.address}
    };
}

inline void from_json(const nlohmann::json& j, IrLabel& l) {
    j.at("name").get_to(l.name);
    j.at("address").get_to(l.address);
}

// IrOperand
inline void to_json(nlohmann::json& j, const IrOperand& op) {
    j = nlohmann::json{
        {"type", static_cast<uint8_t>(op.type)}
    };

    switch (op.type) {
        case IrOperandType::Register:
            j["value"] = std::get<IrRegister>(op.value);
            break;
        case IrOperandType::Immediate:
            j["value"] = std::get<IrImmediate>(op.value);
            break;
        case IrOperandType::Memory:
            j["value"] = std::get<IrMemory>(op.value);
            break;
        case IrOperandType::Label:
            j["value"] = std::get<IrLabel>(op.value);
            break;
    }
}

inline void from_json(const nlohmann::json& j, IrOperand& op) {
    uint8_t type_val;
    j.at("type").get_to(type_val);
    op.type = static_cast<IrOperandType>(type_val);

    switch (op.type) {
        case IrOperandType::Register:
            op.value = j.at("value").get<IrRegister>();
            break;
        case IrOperandType::Immediate:
            op.value = j.at("value").get<IrImmediate>();
            break;
        case IrOperandType::Memory:
            op.value = j.at("value").get<IrMemory>();
            break;
        case IrOperandType::Label:
            op.value = j.at("value").get<IrLabel>();
            break;
    }
}

// IrInstruction
inline void to_json(nlohmann::json& j, const IrInstruction& insn) {
    j = nlohmann::json{
        {"opcode", static_cast<uint16_t>(insn.opcode)},
        {"opcode_name", IrOpcodeToString(insn.opcode)},
        {"operands", insn.operands},
        {"flags_affected", insn.flags_affected},
        {"address", insn.address}
    };
    if (insn.result_register.has_value()) {
        j["result_register"] = insn.result_register.value();
    }
}

inline void from_json(const nlohmann::json& j, IrInstruction& insn) {
    uint16_t opcode_val;
    j.at("opcode").get_to(opcode_val);
    insn.opcode = static_cast<IrOpcode>(opcode_val);
    j.at("operands").get_to(insn.operands);
    j.at("flags_affected").get_to(insn.flags_affected);
    j.at("address").get_to(insn.address);
    if (j.contains("result_register")) {
        insn.result_register = j.at("result_register").get<uint8_t>();
    }
}

// IrCfgEdge
inline void to_json(nlohmann::json& j, const IrCfgEdge& edge) {
    j = nlohmann::json{
        {"from_block", edge.from_block},
        {"to_block", edge.to_block},
        {"type", static_cast<uint8_t>(edge.type)}
    };
}

inline void from_json(const nlohmann::json& j, IrCfgEdge& edge) {
    j.at("from_block").get_to(edge.from_block);
    j.at("to_block").get_to(edge.to_block);
    uint8_t type_val;
    j.at("type").get_to(type_val);
    edge.type = static_cast<IrCfgEdge::Type>(type_val);
}

// IrFunction
inline void to_json(nlohmann::json& j, const IrFunction& func) {
    j = nlohmann::json{
        {"name", func.name},
        {"address", func.address},
        {"instructions", func.instructions},
        {"cfg_edges", func.cfg_edges},
        {"block_starts", func.block_starts},
        {"stack_size", func.stack_size},
        {"is_thumb", func.is_thumb},
        {"imported_symbols", func.imported_symbols}
    };
}

inline void from_json(const nlohmann::json& j, IrFunction& func) {
    j.at("name").get_to(func.name);
    j.at("address").get_to(func.address);
    j.at("instructions").get_to(func.instructions);
    j.at("cfg_edges").get_to(func.cfg_edges);
    j.at("block_starts").get_to(func.block_starts);
    j.at("stack_size").get_to(func.stack_size);
    j.at("is_thumb").get_to(func.is_thumb);
    j.at("imported_symbols").get_to(func.imported_symbols);
}

// IrGlobal
inline void to_json(nlohmann::json& j, const IrGlobal& global) {
    j = nlohmann::json{
        {"name", global.name},
        {"address", global.address},
        {"size", global.size},
        {"is_readonly", global.is_readonly},
        {"data", global.data},
        {"section", global.section}
    };
}

inline void from_json(const nlohmann::json& j, IrGlobal& global) {
    j.at("name").get_to(global.name);
    j.at("address").get_to(global.address);
    j.at("size").get_to(global.size);
    j.at("is_readonly").get_to(global.is_readonly);
    j.at("data").get_to(global.data);
    j.at("section").get_to(global.section);
}

// IrModule
inline void to_json(nlohmann::json& j, const IrModule& mod) {
    j = nlohmann::json{
        {"name", mod.name},
        {"functions", mod.functions},
        {"globals", mod.globals},
        {"base_address", mod.base_address},
        {"architecture", mod.architecture},
        {"endianness", mod.endianness}
    };
}

inline void from_json(const nlohmann::json& j, IrModule& mod) {
    j.at("name").get_to(mod.name);
    j.at("functions").get_to(mod.functions);
    j.at("globals").get_to(mod.globals);
    j.at("base_address").get_to(mod.base_address);
    j.at("architecture").get_to(mod.architecture);
    j.at("endianness").get_to(mod.endianness);
}

//=============================================================================
// Serialization Helpers
//=============================================================================

// Serialize IrModule to JSON string
inline std::string SerializeToJson(const IrModule& module, int indent = 2) {
    nlohmann::json j = module;
    return j.dump(indent);
}

// Deserialize IrModule from JSON string
inline IrModule DeserializeFromJson(const std::string& json_str) {
    nlohmann::json j = nlohmann::json::parse(json_str);
    return j.get<IrModule>();
}

// Serialize to file
inline bool SerializeToFile(const IrModule& module, const std::string& filepath, int indent = 2) {
    try {
        std::ofstream file(filepath);
        if (!file) return false;
        file << SerializeToJson(module, indent);
        return true;
    } catch (...) {
        return false;
    }
}

// Deserialize from file
inline std::optional<IrModule> DeserializeFromFile(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file) return std::nullopt;
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        return DeserializeFromJson(content);
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace kirbyrecomp::ir