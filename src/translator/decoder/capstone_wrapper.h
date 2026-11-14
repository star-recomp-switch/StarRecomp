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

// ARM64 register set (Capstone 5.x register values)
// Note: Capstone 5.x has different register values than 4.x
enum class Arm64Register : uint16_t {
    Invalid = ARM64_REG_INVALID,
    // Special registers (Capstone 5.x values)
    FFR   = ARM64_REG_FFR,
    FP    = ARM64_REG_FP,
    LR    = ARM64_REG_LR,
    NZCV  = ARM64_REG_NZCV,
    SP    = ARM64_REG_SP,
    VG    = ARM64_REG_VG,
    WSP   = ARM64_REG_WSP,
    WZR   = ARM64_REG_WZR,
    XZR   = ARM64_REG_XZR,
    ZA    = ARM64_REG_ZA,
    // B registers (SME)
    B0  = ARM64_REG_B0,  B1  = ARM64_REG_B1,  B2  = ARM64_REG_B2,  B3  = ARM64_REG_B3,
    B4  = ARM64_REG_B4,  B5  = ARM64_REG_B5,  B6  = ARM64_REG_B6,  B7  = ARM64_REG_B7,
    B8  = ARM64_REG_B8,  B9  = ARM64_REG_B9,  B10 = ARM64_REG_B10, B11 = ARM64_REG_B11,
    B12 = ARM64_REG_B12, B13 = ARM64_REG_B13, B14 = ARM64_REG_B14, B15 = ARM64_REG_B15,
    B16 = ARM64_REG_B16, B17 = ARM64_REG_B17, B18 = ARM64_REG_B18, B19 = ARM64_REG_B19,
    B20 = ARM64_REG_B20, B21 = ARM64_REG_B21, B22 = ARM64_REG_B22, B23 = ARM64_REG_B23,
    B24 = ARM64_REG_B24, B25 = ARM64_REG_B25, B26 = ARM64_REG_B26, B27 = ARM64_REG_B27,
    B28 = ARM64_REG_B28, B29 = ARM64_REG_B29, B30 = ARM64_REG_B30, B31 = ARM64_REG_B31,
    // D registers (FP64/SIMD64)
    D0  = ARM64_REG_D0,  D1  = ARM64_REG_D1,  D2  = ARM64_REG_D2,  D3  = ARM64_REG_D3,
    D4  = ARM64_REG_D4,  D5  = ARM64_REG_D5,  D6  = ARM64_REG_D6,  D7  = ARM64_REG_D7,
    D8  = ARM64_REG_D8,  D9  = ARM64_REG_D9,  D10 = ARM64_REG_D10, D11 = ARM64_REG_D11,
    D12 = ARM64_REG_D12, D13 = ARM64_REG_D13, D14 = ARM64_REG_D14, D15 = ARM64_REG_D15,
    D16 = ARM64_REG_D16, D17 = ARM64_REG_D17, D18 = ARM64_REG_D18, D19 = ARM64_REG_D19,
    D20 = ARM64_REG_D20, D21 = ARM64_REG_D21, D22 = ARM64_REG_D22, D23 = ARM64_REG_D23,
    D24 = ARM64_REG_D24, D25 = ARM64_REG_D25, D26 = ARM64_REG_D26, D27 = ARM64_REG_D27,
    D28 = ARM64_REG_D28, D29 = ARM64_REG_D29, D30 = ARM64_REG_D30, D31 = ARM64_REG_D31,
    // H registers (FP16)
    H0  = ARM64_REG_H0,  H1  = ARM64_REG_H1,  H2  = ARM64_REG_H2,  H3  = ARM64_REG_H3,
    H4  = ARM64_REG_H4,  H5  = ARM64_REG_H5,  H6  = ARM64_REG_H6,  H7  = ARM64_REG_H7,
    H8  = ARM64_REG_H8,  H9  = ARM64_REG_H9,  H10 = ARM64_REG_H10, H11 = ARM64_REG_H11,
    H12 = ARM64_REG_H12, H13 = ARM64_REG_H13, H14 = ARM64_REG_H14, H15 = ARM64_REG_H15,
    H16 = ARM64_REG_H16, H17 = ARM64_REG_H17, H18 = ARM64_REG_H18, H19 = ARM64_REG_H19,
    H20 = ARM64_REG_H20, H21 = ARM64_REG_H21, H22 = ARM64_REG_H22, H23 = ARM64_REG_H23,
    H24 = ARM64_REG_H24, H25 = ARM64_REG_H25, H26 = ARM64_REG_H26, H27 = ARM64_REG_H27,
    H28 = ARM64_REG_H28, H29 = ARM64_REG_H29, H30 = ARM64_REG_H30, H31 = ARM64_REG_H31,
    // P registers (predicate)
    P0  = ARM64_REG_P0,  P1  = ARM64_REG_P1,  P2  = ARM64_REG_P2,  P3  = ARM64_REG_P3,
    P4  = ARM64_REG_P4,  P5  = ARM64_REG_P5,  P6  = ARM64_REG_P6,  P7  = ARM64_REG_P7,
    P8  = ARM64_REG_P8,  P9  = ARM64_REG_P9,  P10 = ARM64_REG_P10, P11 = ARM64_REG_P11,
    P12 = ARM64_REG_P12, P13 = ARM64_REG_P13, P14 = ARM64_REG_P14, P15 = ARM64_REG_P15,
    // Q registers (SIMD128)
    Q0  = ARM64_REG_Q0,  Q1  = ARM64_REG_Q1,  Q2  = ARM64_REG_Q2,  Q3  = ARM64_REG_Q3,
    Q4  = ARM64_REG_Q4,  Q5  = ARM64_REG_Q5,  Q6  = ARM64_REG_Q6,  Q7  = ARM64_REG_Q7,
    Q8  = ARM64_REG_Q8,  Q9  = ARM64_REG_Q9,  Q10 = ARM64_REG_Q10, Q11 = ARM64_REG_Q11,
    Q12 = ARM64_REG_Q12, Q13 = ARM64_REG_Q13, Q14 = ARM64_REG_Q14, Q15 = ARM64_REG_Q15,
    Q16 = ARM64_REG_Q16, Q17 = ARM64_REG_Q17, Q18 = ARM64_REG_Q18, Q19 = ARM64_REG_Q19,
    Q20 = ARM64_REG_Q20, Q21 = ARM64_REG_Q21, Q22 = ARM64_REG_Q22, Q23 = ARM64_REG_Q23,
    Q24 = ARM64_REG_Q24, Q25 = ARM64_REG_Q25, Q26 = ARM64_REG_Q26, Q27 = ARM64_REG_Q27,
    Q28 = ARM64_REG_Q28, Q29 = ARM64_REG_Q29, Q30 = ARM64_REG_Q30, Q31 = ARM64_REG_Q31,
    // S registers (FP32)
    S0  = ARM64_REG_S0,  S1  = ARM64_REG_S1,  S2  = ARM64_REG_S2,  S3  = ARM64_REG_S3,
    S4  = ARM64_REG_S4,  S5  = ARM64_REG_S5,  S6  = ARM64_REG_S6,  S7  = ARM64_REG_S7,
    S8  = ARM64_REG_S8,  S9  = ARM64_REG_S9,  S10 = ARM64_REG_S10, S11 = ARM64_REG_S11,
    S12 = ARM64_REG_S12, S13 = ARM64_REG_S13, S14 = ARM64_REG_S14, S15 = ARM64_REG_S15,
    S16 = ARM64_REG_S16, S17 = ARM64_REG_S17, S18 = ARM64_REG_S18, S19 = ARM64_REG_S19,
    S20 = ARM64_REG_S20, S21 = ARM64_REG_S21, S22 = ARM64_REG_S22, S23 = ARM64_REG_S23,
    S24 = ARM64_REG_S24, S25 = ARM64_REG_S25, S26 = ARM64_REG_S26, S27 = ARM64_REG_S27,
    S28 = ARM64_REG_S28, S29 = ARM64_REG_S29, S30 = ARM64_REG_S30, S31 = ARM64_REG_S31,
    // W registers (32-bit GPR)
    W0  = ARM64_REG_W0,  W1  = ARM64_REG_W1,  W2  = ARM64_REG_W2,  W3  = ARM64_REG_W3,
    W4  = ARM64_REG_W4,  W5  = ARM64_REG_W5,  W6  = ARM64_REG_W6,  W7  = ARM64_REG_W7,
    W8  = ARM64_REG_W8,  W9  = ARM64_REG_W9,  W10 = ARM64_REG_W10, W11 = ARM64_REG_W11,
    W12 = ARM64_REG_W12, W13 = ARM64_REG_W13, W14 = ARM64_REG_W14, W15 = ARM64_REG_W15,
    W16 = ARM64_REG_W16, W17 = ARM64_REG_W17, W18 = ARM64_REG_W18, W19 = ARM64_REG_W19,
    W20 = ARM64_REG_W20, W21 = ARM64_REG_W21, W22 = ARM64_REG_W22, W23 = ARM64_REG_W23,
    W24 = ARM64_REG_W24, W25 = ARM64_REG_W25, W26 = ARM64_REG_W26, W27 = ARM64_REG_W27,
    W28 = ARM64_REG_W28, W29 = ARM64_REG_W29, W30 = ARM64_REG_W30,
    // X registers (64-bit GPR)
    X0  = ARM64_REG_X0,  X1  = ARM64_REG_X1,  X2  = ARM64_REG_X2,  X3  = ARM64_REG_X3,
    X4  = ARM64_REG_X4,  X5  = ARM64_REG_X5,  X6  = ARM64_REG_X6,  X7  = ARM64_REG_X7,
    X8  = ARM64_REG_X8,  X9  = ARM64_REG_X9,  X10 = ARM64_REG_X10, X11 = ARM64_REG_X11,
    X12 = ARM64_REG_X12, X13 = ARM64_REG_X13, X14 = ARM64_REG_X14, X15 = ARM64_REG_X15,
    X16 = ARM64_REG_X16, X17 = ARM64_REG_X17, X18 = ARM64_REG_X18, X19 = ARM64_REG_X19,
    X20 = ARM64_REG_X20, X21 = ARM64_REG_X21, X22 = ARM64_REG_X22, X23 = ARM64_REG_X23,
    X24 = ARM64_REG_X24, X25 = ARM64_REG_X25, X26 = ARM64_REG_X26, X27 = ARM64_REG_X27,
    X28 = ARM64_REG_X28,
    // Z registers (SVE)
    Z0  = ARM64_REG_Z0,  Z1  = ARM64_REG_Z1,  Z2  = ARM64_REG_Z2,  Z3  = ARM64_REG_Z3,
    Z4  = ARM64_REG_Z4,  Z5  = ARM64_REG_Z5,  Z6  = ARM64_REG_Z6,  Z7  = ARM64_REG_Z7,
    Z8  = ARM64_REG_Z8,  Z9  = ARM64_REG_Z9,  Z10 = ARM64_REG_Z10, Z11 = ARM64_REG_Z11,
    Z12 = ARM64_REG_Z12, Z13 = ARM64_REG_Z13, Z14 = ARM64_REG_Z14, Z15 = ARM64_REG_Z15,
    Z16 = ARM64_REG_Z16, Z17 = ARM64_REG_Z17, Z18 = ARM64_REG_Z18, Z19 = ARM64_REG_Z19,
    Z20 = ARM64_REG_Z20, Z21 = ARM64_REG_Z21, Z22 = ARM64_REG_Z22, Z23 = ARM64_REG_Z23,
    Z24 = ARM64_REG_Z24, Z25 = ARM64_REG_Z25, Z26 = ARM64_REG_Z26, Z27 = ARM64_REG_Z27,
    Z28 = ARM64_REG_Z28, Z29 = ARM64_REG_Z29, Z30 = ARM64_REG_Z30, Z31 = ARM64_REG_Z31,
    // V registers (alias for Q)
    V0  = ARM64_REG_V0,  V1  = ARM64_REG_V1,  V2  = ARM64_REG_V2,  V3  = ARM64_REG_V3,
    V4  = ARM64_REG_V4,  V5  = ARM64_REG_V5,  V6  = ARM64_REG_V6,  V7  = ARM64_REG_V7,
    V8  = ARM64_REG_V8,  V9  = ARM64_REG_V9,  V10 = ARM64_REG_V10, V11 = ARM64_REG_V11,
    V12 = ARM64_REG_V12, V13 = ARM64_REG_V13, V14 = ARM64_REG_V14, V15 = ARM64_REG_V15,
    V16 = ARM64_REG_V16, V17 = ARM64_REG_V17, V18 = ARM64_REG_V18, V19 = ARM64_REG_V19,
    V20 = ARM64_REG_V20, V21 = ARM64_REG_V21, V22 = ARM64_REG_V22, V23 = ARM64_REG_V23,
    V24 = ARM64_REG_V24, V25 = ARM64_REG_V25, V26 = ARM64_REG_V26, V27 = ARM64_REG_V27,
    V28 = ARM64_REG_V28, V29 = ARM64_REG_V29, V30 = ARM64_REG_V30, V31 = ARM64_REG_V31,
    // Aliases
    X29 = ARM64_REG_FP,  // FP alias
    X30 = ARM64_REG_LR,  // LR alias
    ENDING = ARM64_REG_ENDING
};

// ARM64 system register (for SYS operands) - Capstone 5.x values from arm64_sysreg enum
enum class Arm64SysReg : uint16_t {
    Invalid = 0,
    // Common system registers (Capstone 5.x values)
    SCTLR_EL1 = ARM64_SYSREG_SCTLR_EL1,
    ACTLR_EL1 = ARM64_SYSREG_ACTLR_EL1,
    CPACR_EL1 = ARM64_SYSREG_CPACR_EL1,
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

    // Note: reg_sub was removed in Capstone 5.x - register pair info is encoded differently
};

// Full instruction detail
struct InstructionDetail {
    unsigned int id;                    // Capstone instruction ID (arm64_insn enum)
    std::string mnemonic;               // Instruction mnemonic (e.g., "add", "ldr")
    std::string op_str;                 // Operand string as printed by Capstone
    std::vector<Arm64Operand> operands; // Parsed operands
    std::vector<uint8_t> bytes;         // Raw instruction bytes
    uint16_t size;                      // Instruction size in bytes (always 4 for ARM64)
    uint64_t address;                   // Address of this instruction

    // ARM64-specific details
    uint8_t cc;                       // Condition code (ARM64_CC_*)
    bool update_flags;                // Whether instruction updates flags (S suffix)
    bool writeback;                   // Whether instruction has writeback
    // Note: insn_id was removed in Capstone 5.x
};

} // namespace kirbyrecomp::translator

#endif // KIRBYRECOMP_TRANSLATOR_DECODER_CAPSTONE_WRAPPER_H