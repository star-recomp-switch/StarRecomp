#ifndef KIRBYRECOMP_TRANSLATOR_DECODER_CAPSTONE_WRAPPER_H
#define KIRBYRECOMP_TRANSLATOR_DECODER_CAPSTONE_WRAPPER_H

#include <capstone/capstone.h>
#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <span>

namespace kirbyrecomp::translator {

// Forward declaration
struct InstructionDetail;

class CapstoneWrapper {
public:
    CapstoneWrapper();
    ~CapstoneWrapper();

    // Non-copyable, non-movable (owns Capstone handle)
    CapstoneWrapper(const CapstoneWrapper&) = delete;
    CapstoneWrapper& operator=(const CapstoneWrapper&) = delete;
    CapstoneWrapper(CapstoneWrapper&&) = delete;
    CapstoneWrapper& operator=(CapstoneWrapper&&) = delete;

    // Decode a single instruction from bytes at the given address
    // Returns nullopt on failure (with error set in last_error_)
    std::optional<InstructionDetail> decode_instruction(
        std::span<const uint8_t> bytes,
        uint64_t address
    );

    // Get the last error code from Capstone
    cs_err get_last_error() const noexcept { return last_error_; }

    // Get human-readable error message for the last error
    std::string get_last_error_message() const;

private:
    csh handle_;
    cs_err last_error_;
    bool initialized_;
};

// ARM64 operand types as used by Capstone
enum class Arm64OperandType {
    Invalid = 0,
    Register = ARM64_OP_REG,
    Immediate = ARM64_OP_IMM,
    Memory = ARM64_OP_MEM,
    FloatingPoint = ARM64_OP_FP,
    SystemRegister = ARM64_OP_SYS
};

// ARM64 register set (subset of Capstone's ARM64 registers)
enum class Arm64Register : uint16_t {
    Invalid = ARM64_REG_INVALID,
    // General purpose registers
    X0  = ARM64_REG_X0,  X1  = ARM64_REG_X1,  X2  = ARM64_REG_X2,  X3  = ARM64_REG_X3,
    X4  = ARM64_REG_X4,  X5  = ARM64_REG_X5,  X6  = ARM64_REG_X6,  X7  = ARM64_REG_X7,
    X8  = ARM64_REG_X8,  X9  = ARM64_REG_X9,  X10 = ARM64_REG_X10, X11 = ARM64_REG_X11,
    X12 = ARM64_REG_X12, X13 = ARM64_REG_X13, X14 = ARM64_REG_X14, X15 = ARM64_REG_X15,
    X16 = ARM64_REG_X16, X17 = ARM64_REG_X17, X18 = ARM64_REG_X18, X19 = ARM64_REG_X19,
    X20 = ARM64_REG_X20, X21 = ARM64_REG_X21, X22 = ARM64_REG_X22, X23 = ARM64_REG_X23,
    X24 = ARM64_REG_X24, X25 = ARM64_REG_X25, X26 = ARM64_REG_X26, X27 = ARM64_REG_X27,
    X28 = ARM64_REG_X28, X29 = ARM64_REG_X29, X30 = ARM64_REG_X30, SP  = ARM64_REG_SP,
    // 32-bit views
    W0  = ARM64_REG_W0,  W1  = ARM64_REG_W1,  W2  = ARM64_REG_W2,  W3  = ARM64_REG_W3,
    W4  = ARM64_REG_W4,  W5  = ARM64_REG_W5,  W6  = ARM64_REG_W6,  W7  = ARM64_REG_W7,
    W8  = ARM64_REG_W8,  W9  = ARM64_REG_W9,  W10 = ARM64_REG_W10, W11 = ARM64_REG_W11,
    W12 = ARM64_REG_W12, W13 = ARM64_REG_W13, W14 = ARM64_REG_W14, W15 = ARM64_REG_W15,
    W16 = ARM64_REG_W16, W17 = ARM64_REG_W17, W18 = ARM64_REG_W18, W19 = ARM64_REG_W19,
    W20 = ARM64_REG_W20, W21 = ARM64_REG_W21, W22 = ARM64_REG_W22, W23 = ARM64_REG_W23,
    W24 = ARM64_REG_W24, W25 = ARM64_REG_W25, W26 = ARM64_REG_W26, W27 = ARM64_REG_W27,
    W28 = ARM64_REG_W28, W29 = ARM64_REG_W29, W30 = ARM64_REG_W30, WSP = ARM64_REG_WSP,
    // SIMD/NEON registers
    Q0  = ARM64_REG_Q0,  Q1  = ARM64_REG_Q1,  Q2  = ARM64_REG_Q2,  Q3  = ARM64_REG_Q3,
    Q4  = ARM64_REG_Q4,  Q5  = ARM64_REG_Q5,  Q6  = ARM64_REG_Q6,  Q7  = ARM64_REG_Q7,
    Q8  = ARM64_REG_Q8,  Q9  = ARM64_REG_Q9,  Q10 = ARM64_REG_Q10, Q11 = ARM64_REG_Q11,
    Q12 = ARM64_REG_Q12, Q13 = ARM64_REG_Q13, Q14 = ARM64_REG_Q14, Q15 = ARM64_REG_Q15,
    Q16 = ARM64_REG_Q16, Q17 = ARM64_REG_Q17, Q18 = ARM64_REG_Q18, Q19 = ARM64_REG_Q19,
    Q20 = ARM64_REG_Q20, Q21 = ARM64_REG_Q21, Q22 = ARM64_REG_Q22, Q23 = ARM64_REG_Q23,
    Q24 = ARM64_REG_Q24, Q25 = ARM64_REG_Q25, Q26 = ARM64_REG_Q26, Q27 = ARM64_REG_Q27,
    Q28 = ARM64_REG_Q28, Q29 = ARM64_REG_Q29, Q30 = ARM64_REG_Q30, Q31 = ARM64_REG_Q31,
    // Vector registers (same as Q but 128-bit view)
    V0  = ARM64_REG_V0,  V1  = ARM64_REG_V1,  V2  = ARM64_REG_V2,  V3  = ARM64_REG_V3,
    V4  = ARM64_REG_V4,  V5  = ARM64_REG_V5,  V6  = ARM64_REG_V6,  V7  = ARM64_REG_V7,
    V8  = ARM64_REG_V8,  V9  = ARM64_REG_V9,  V10 = ARM64_REG_V10, V11 = ARM64_REG_V11,
    V12 = ARM64_REG_V12, V13 = ARM64_REG_V13, V14 = ARM64_REG_V14, V15 = ARM64_REG_V15,
    V16 = ARM64_REG_V16, V17 = ARM64_REG_V17, V18 = ARM64_REG_V18, V19 = ARM64_REG_V19,
    V20 = ARM64_REG_V20, V21 = ARM64_REG_V21, V22 = ARM64_REG_V22, V23 = ARM64_REG_V23,
    V24 = ARM64_REG_V24, V25 = ARM64_REG_V25, V26 = ARM64_REG_V26, V27 = ARM64_REG_V27,
    V28 = ARM64_REG_V28, V29 = ARM64_REG_V29, V30 = ARM64_REG_V30, V31 = ARM64_REG_V31,
    // System registers
    NZCV = ARM64_REG_NZCV,
    // PSTATE fields
    SPSEL = ARM64_REG_SPSEL,
    DAIF  = ARM64_REG_DAIF,
    // End marker
    ENDING = ARM64_REG_ENDING
};

// ARM64 system register (for SYS operands)
enum class Arm64SysReg : uint16_t {
    Invalid = 0,
    // Common system registers
    SCTLR_EL1 = 0x1800,
    ACTLR_EL1 = 0x1801,
    CPACR_EL1 = 0x1802,
    // ... (Capstone defines many more in arm64.h)
};

// Operand detail for ARM64
struct Arm64Operand {
    Arm64OperandType type;
    uint32_t vas;           // Vector arrangement specifier
    uint32_t vector_index;  // Vector index for vector operands
    uint8_t shift_type;     // Shift type (ARM64_SFT_*)
    uint8_t shift_value;    // Shift value
    uint32_t ext;           // Extension type (ARM64_EXT_*)

    // Union-like storage for different operand types
    Arm64Register reg;      // For REG operands
    int64_t imm;            // For IMM operands
    struct {
        Arm64Register base;     // Base register
        Arm64Register index;    // Index register
        int64_t disp;           // Displacement
    } mem;                    // For MEM operands
    double fp;                // For FP operands
    Arm64SysReg sys;          // For SYS operands

    // Subtracted registers (for register pairs like LDP/STP)
    Arm64Register reg_sub;
};

// Full instruction detail
struct InstructionDetail {
    cs_insn_id id;                    // Capstone instruction ID (ARM64_INS_*)
    std::string mnemonic;             // Instruction mnemonic (e.g., "add", "ldr")
    std::string op_str;               // Operand string as printed by Capstone
    std::vector<Arm64Operand> operands; // Parsed operands
    std::vector<uint8_t> bytes;       // Raw instruction bytes
    uint16_t size;                    // Instruction size in bytes (always 4 for ARM64)
    uint64_t address;                 // Address of this instruction

    // ARM64-specific details
    uint8_t cc;                       // Condition code (ARM64_CC_*)
    bool update_flags;                // Whether instruction updates flags (S suffix)
    bool writeback;                   // Whether instruction has writeback
    uint32_t insn_id;                 // Raw instruction encoding
};

} // namespace kirbyrecomp::translator

#endif // KIRBYRECOMP_TRANSLATOR_DECODER_CAPSTONE_WRAPPER_H