// Maxwell ISA Instruction Decoder Implementation
// Based on envytools/envyas and NVIDIA Maxwell documentation

#include "kirbyrecomp/shader/maxwell_decoder.h"

#include <cstring>
#include <sstream>
#include <iomanip>

namespace kirbyrecomp::shader::maxwell {

// ============================================================================
// Instruction Info Table
// ============================================================================

namespace {
const InstructionInfo kInstructionInfoTable[] = {
    // Data movement
    {MaxwellOpcode::MOV,   "MOV",   InstFormat::FMT_R,    1, 1, true, false, false, "Move register to register"},
    {MaxwellOpcode::SELP,  "SELP",  InstFormat::FMT_R,    2, 1, true, false, false, "Select based on predicate"},
    {MaxwellOpcode::SHF,   "SHF",   InstFormat::FMT_R,    3, 1, true, false, false, "Shift and fold"},
    {MaxwellOpcode::PRMT,  "PRMT",  InstFormat::FMT_R,    3, 1, true, false, false, "Permute bytes"},

    // Integer arithmetic
    {MaxwellOpcode::IADD,  "IADD",  InstFormat::FMT_R,    2, 1, true, true,  false, "Integer add"},
    {MaxwellOpcode::IADD3, "IADD3", InstFormat::FMT_R,    3, 1, true, true,  false, "Integer add 3-operand"},
    {MaxwellOpcode::IMAD,  "IMAD",  InstFormat::FMT_R,    3, 1, true, true,  false, "Integer multiply-add"},
    {MaxwellOpcode::IMADSP,"IMADSP",InstFormat::FMT_R,    3, 1, true, true,  false, "Integer multiply-add (special)"},
    {MaxwellOpcode::IMUL,  "IMUL",  InstFormat::FMT_R,    2, 1, true, true,  false, "Integer multiply"},
    {MaxwellOpcode::IMULH, "IMULH", InstFormat::FMT_R,    2, 1, true, true,  false, "Integer multiply high"},
    {MaxwellOpcode::IDIV,  "IDIV",  InstFormat::FMT_R,    2, 1, true, true,  false, "Integer divide"},
    {MaxwellOpcode::IREM,  "IREM",  InstFormat::FMT_R,    2, 1, true, true,  false, "Integer remainder"},
    {MaxwellOpcode::ISETP, "ISETP", InstFormat::FMT_R,    2, 0, true, true,  false, "Integer compare and set predicate"},
    {MaxwellOpcode::ISAD,  "ISAD",  InstFormat::FMT_R,    3, 1, true, true,  false, "Integer sum of absolute differences"},

    // Bitwise
    {MaxwellOpcode::AND,   "AND",   InstFormat::FMT_R,    2, 1, true, false, false, "Bitwise AND"},
    {MaxwellOpcode::OR,    "OR",    InstFormat::FMT_R,    2, 1, true, false, false, "Bitwise OR"},
    {MaxwellOpcode::XOR,   "XOR",   InstFormat::FMT_R,    2, 1, true, false, false, "Bitwise XOR"},
    {MaxwellOpcode::NOT,   "NOT",   InstFormat::FMT_R,    1, 1, true, false, false, "Bitwise NOT"},
    {MaxwellOpcode::BFI,   "BFI",   InstFormat::FMT_R,    3, 1, true, false, false, "Bit field insert"},
    {MaxwellOpcode::BFE,   "BFE",   InstFormat::FMT_R,    2, 1, true, false, false, "Bit field extract"},
    {MaxwellOpcode::SHL,   "SHL",   InstFormat::FMT_R,    2, 1, true, false, false, "Shift left"},
    {MaxwellOpcode::SHR,   "SHR",   InstFormat::FMT_R,    2, 1, true, false, false, "Shift right"},
    {MaxwellOpcode::SHF_R, "SHF_R", InstFormat::FMT_R,    3, 1, true, false, false, "Shift and fold reverse"},

    // Floating point
    {MaxwellOpcode::FADD,  "FADD",  InstFormat::FMT_R,    2, 1, true, true,  false, "Float add"},
    {MaxwellOpcode::FMUL,  "FMUL",  InstFormat::FMT_R,    2, 1, true, true,  false, "Float multiply"},
    {MaxwellOpcode::FMAD,  "FMAD",  InstFormat::FMT_R,    3, 1, true, true,  false, "Float multiply-add"},
    {MaxwellOpcode::FFMA,  "FFMA",  InstFormat::FMT_R,    3, 1, true, true,  false, "Float fused multiply-add"},
    {MaxwellOpcode::FDIV,  "FDIV",  InstFormat::FMT_R,    2, 1, true, true,  false, "Float divide"},
    {MaxwellOpcode::FSETP, "FSETP", InstFormat::FMT_R,    2, 0, true, true,  false, "Float compare and set predicate"},
    {MaxwellOpcode::FMIN,  "FMIN",  InstFormat::FMT_R,    2, 1, true, true,  false, "Float minimum"},
    {MaxwellOpcode::FMAX,  "FMAX",  InstFormat::FMT_R,    2, 1, true, true,  false, "Float maximum"},
    {MaxwellOpcode::FSQRT, "FSQRT", InstFormat::FMT_R,    1, 1, true, true,  false, "Float square root"},
    {MaxwellOpcode::FRCP,  "FRCP",  InstFormat::FMT_R,    1, 1, true, true,  false, "Float reciprocal"},
    {MaxwellOpcode::FRSQ,  "FRSQ",  InstFormat::FMT_R,    1, 1, true, true,  false, "Float reciprocal square root"},
    {MaxwellOpcode::FSIN,  "FSIN",  InstFormat::FMT_R,    1, 1, true, true,  false, "Float sine"},
    {MaxwellOpcode::FCOS,  "FCOS",  InstFormat::FMT_R,    1, 1, true, true,  false, "Float cosine"},
    {MaxwellOpcode::F2F,   "F2F",   InstFormat::FMT_R,    1, 1, true, false, false, "Float to float conversion"},
    {MaxwellOpcode::F2I,   "F2I",   InstFormat::FMT_R,    1, 1, true, false, false, "Float to int conversion"},
    {MaxwellOpcode::I2F,   "I2F",   InstFormat::FMT_R,    1, 1, true, false, false, "Int to float conversion"},
    {MaxwellOpcode::F2I_RZ,"F2I_RZ",InstFormat::FMT_R,    1, 1, true, false, false, "Float to int (round toward zero)"},
    {MaxwellOpcode::F2I_RN,"F2I_RN",InstFormat::FMT_R,    1, 1, true, false, false, "Float to int (round nearest)"},
    {MaxwellOpcode::F2I_RU,"F2I_RU",InstFormat::FMT_R,    1, 1, true, false, false, "Float to int (round up)"},
    {MaxwellOpcode::F2I_RD,"F2I_RD",InstFormat::FMT_R,    1, 1, true, false, false, "Float to int (round down)"},

    // Comparison/Selection
    {MaxwellOpcode::SEL,   "SEL",   InstFormat::FMT_R,    3, 1, true, false, false, "Select"},
    {MaxwellOpcode::SLCT,  "SLCT",  InstFormat::FMT_R,    3, 1, true, false, false, "Select with predicate"},
    {MaxwellOpcode::CMP,   "CMP",   InstFormat::FMT_R,    2, 0, true, true,  false, "Compare"},

    // Memory
    {MaxwellOpcode::LD,    "LD",    InstFormat::FMT_LDST, 1, 1, true, false, false, "Load local/shared"},
    {MaxwellOpcode::ST,    "ST",    InstFormat::FMT_LDST, 2, 0, true, false, false, "Store local/shared"},
    {MaxwellOpcode::LDG,   "LDG",   InstFormat::FMT_LDST, 1, 1, true, false, false, "Load global"},
    {MaxwellOpcode::STG,   "STG",   InstFormat::FMT_LDST, 2, 0, true, false, false, "Store global"},
    {MaxwellOpcode::LDS,   "LDS",   InstFormat::FMT_LDST, 1, 1, true, false, false, "Load shared"},
    {MaxwellOpcode::STS,   "STS",   InstFormat::FMT_LDST, 2, 0, true, false, false, "Store shared"},
    {MaxwellOpcode::LDU,   "LDU",   InstFormat::FMT_LDST, 1, 1, true, false, false, "Load uniform/constant"},
    {MaxwellOpcode::ATOM,  "ATOM",  InstFormat::FMT_LDST, 2, 1, true, false, false, "Atomic operation"},
    {MaxwellOpcode::RED,   "RED",   InstFormat::FMT_LDST, 2, 1, true, false, false, "Reduction operation"},

    // Texture
    {MaxwellOpcode::TEX,   "TEX",   InstFormat::FMT_TEX,  3, 1, true, false, false, "Texture sample"},
    {MaxwellOpcode::TXD,   "TXD",   InstFormat::FMT_TEX,  4, 1, true, false, false, "Texture sample with derivatives"},
    {MaxwellOpcode::TXL,   "TXL",   InstFormat::FMT_TEX,  3, 1, true, false, false, "Texture sample LOD"},
    {MaxwellOpcode::TXQ,   "TXQ",   InstFormat::FMT_TEX,  2, 1, true, false, false, "Texture query"},
    {MaxwellOpcode::TXF,   "TXF",   InstFormat::FMT_TEX,  2, 1, true, false, false, "Texture fetch"},
    {MaxwellOpcode::TXB,   "TXB",   InstFormat::FMT_TEX,  3, 1, true, false, false, "Texture sample with bias"},
    {MaxwellOpcode::TEXS,  "TEXS",  InstFormat::FMT_TEX,  2, 1, true, false, false, "Texture sample (sparse)"},

    // Control flow
    {MaxwellOpcode::BRA,   "BRA",   InstFormat::FMT_CTRL, 0, 0, true, false, false, "Branch"},
    {MaxwellOpcode::BRX,   "BRX",   InstFormat::FMT_CTRL, 1, 0, true, false, false, "Branch indirect"},
    {MaxwellOpcode::JMP,   "JMP",   InstFormat::FMT_CTRL, 0, 0, false, false, false, "Jump"},
    {MaxwellOpcode::CALL,  "CALL",  InstFormat::FMT_CTRL, 0, 0, true, false, false, "Call"},
    {MaxwellOpcode::RET,   "RET",   InstFormat::FMT_CTRL, 0, 0, false, false, false, "Return"},
    {MaxwellOpcode::EXIT,  "EXIT",  InstFormat::FMT_CTRL, 0, 0, false, false, false, "Exit thread"},
    {MaxwellOpcode::BRK,   "BRK",   InstFormat::FMT_CTRL, 0, 0, true, false, false, "Break"},
    {MaxwellOpcode::CONT,  "CONT",  InstFormat::FMT_CTRL, 0, 0, true, false, false, "Continue"},
    {MaxwellOpcode::PBRK,  "PBRK",  InstFormat::FMT_CTRL, 0, 0, true, false, false, "Pre-break"},
    {MaxwellOpcode::PCONT, "PCONT", InstFormat::FMT_CTRL, 0, 0, true, false, false, "Pre-continue"},

    // Synchronization
    {MaxwellOpcode::BAR,   "BAR",   InstFormat::FMT_SYNC, 1, 0, false, false, false, "Barrier"},
    {MaxwellOpcode::SYNC,  "SYNC",  InstFormat::FMT_SYNC, 0, 0, false, false, false, "Synchronize"},
    {MaxwellOpcode::MEMBAR,"MEMBAR",InstFormat::FMT_SYNC, 0, 0, false, false, false, "Memory barrier"},

    // Special
    {MaxwellOpcode::NOP,   "NOP",   InstFormat::FMT_MISC, 0, 0, false, false, false, "No operation"},
    {MaxwellOpcode::SSY,   "SSY",   InstFormat::FMT_MISC, 0, 0, false, false, false, "Set synchronization"},
    {MaxwellOpcode::S2R,   "S2R",   InstFormat::FMT_MISC, 0, 1, false, false, false, "Special register read"},
    {MaxwellOpcode::R2S,   "R2S",   InstFormat::FMT_MISC, 1, 0, false, false, false, "Special register write"},
    {MaxwellOpcode::VOTE,  "VOTE",  InstFormat::FMT_MISC, 1, 1, true, false, false, "Warp vote"},
    {MaxwellOpcode::LEPC,  "LEPC",  InstFormat::FMT_MISC, 0, 1, false, false, false, "Load EPC"},
};

const size_t kInstructionCount = sizeof(kInstructionInfoTable) / sizeof(kInstructionInfoTable[0]);
} // namespace

const InstructionInfo& get_instruction_info(MaxwellOpcode opcode) {
    for (size_t i = 0; i < kInstructionCount; ++i) {
        if (kInstructionInfoTable[i].opcode == opcode) {
            return kInstructionInfoTable[i];
        }
    }
    static const InstructionInfo kUnknown = {
        MaxwellOpcode::NOP, "UNKNOWN", InstFormat::FMT_MISC, 0, 0, false, false, false, "Unknown opcode"
    };
    return kUnknown;
}

// ============================================================================
// Register Implementation
// ============================================================================

std::string Register::to_string() const {
    std::stringstream ss;
    switch (file) {
        case RegisterFile::GPR:     ss << "R"; break;
        case RegisterFile::PRED:    ss << "P"; break;
        case RegisterFile::CCR:     ss << "CC"; break;
        case RegisterFile::SR:      ss << "SR"; break;
        case RegisterFile::UR:      ss << "UR"; break;
        case RegisterFile::CR:      ss << "c"; break;
        case RegisterFile::TR:      ss << "T"; break;
        case RegisterFile::SAMPLER: ss << "S"; break;
        case RegisterFile::ADDR:    ss << "A"; break;
        default:                    ss << "?"; break;
    }
    ss << index;
    return ss.str();
}

bool Register::operator==(const Register& other) const {
    return file == other.file && index == other.index;
}

// ============================================================================
// Operand Implementation
// ============================================================================

std::string Operand::to_string() const {
    std::stringstream ss;
    switch (type) {
        case OperandType::REGISTER:
            ss << reg.to_string();
            if (swizzle != 0xE4) {  // Not .xyzw
                ss << ".";
                const char* comp[] = {"x", "y", "z", "w"};
                for (int i = 0; i < 4; ++i) {
                    if (swizzle & (1 << i)) ss << comp[i];
                }
            }
            if (negate) ss << " (neg)";
            if (absolute) ss << " (abs)";
            break;
        case OperandType::IMMEDIATE:
            ss << immediate_value;
            break;
        case OperandType::CONSTANT:
            ss << "c[" << constant_index << "]";
            break;
        case OperandType::ADDRESS:
            ss << "[addr]";
            break;
    }
    return ss.str();
}

// ============================================================================
// DecodedInstruction Implementation
// ============================================================================

std::string DecodedInstruction::to_string() const {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(8) << address << ":  "
       << std::setw(16) << raw[0] << (is_dual_issue ? " " + std::to_string(raw[1]) : "") << "  "
       << info.mnemonic;

    if (predicate.index != 7) {
        ss << " @" << predicate.to_string() << (predicate_neg ? "!" : "");
    }
    if (cc_update) ss << " CC";

    if (!src_operands.empty()) {
        ss << " ";
        for (size_t i = 0; i < src_operands.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << src_operands[i].to_string();
        }
    }
    if (!dst_operands.empty()) {
        ss << " -> ";
        for (size_t i = 0; i < dst_operands.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << dst_operands[i].to_string();
        }
    }

    return ss.str();
}

// ============================================================================
// Decoder Implementation
// ============================================================================

uint64_t Decoder::extract_bits(uint64_t value, int shift, int width) {
    if (width == 64) return value >> shift;
    uint64_t mask = (width == 64) ? ~0ULL : ((1ULL << width) - 1);
    return (value >> shift) & mask;
}

std::optional<DecodedInstruction> Decoder::decode_instruction(
    uint64_t address,
    const uint8_t* bytes,
    size_t available_bytes) {

    last_error_.clear();

    if (available_bytes < 8) {
        last_error_ = "Insufficient bytes for instruction (need 8, got " + std::to_string(available_bytes) + ")";
        return std::nullopt;
    }

    DecodedInstruction inst;
    inst.address = address;
    inst.raw[0] = *reinterpret_cast<const uint64_t*>(bytes);

    // Check for dual-issue (bit 63 set indicates dual-issue pair)
    bool is_dual = (inst.raw[0] >> 63) & 1;
    inst.is_dual_issue = is_dual;

    if (is_dual) {
        if (available_bytes < 16) {
            last_error_ = "Dual-issue instruction needs 16 bytes";
            return std::nullopt;
        }
        inst.raw[1] = *reinterpret_cast<const uint64_t*>(bytes + 8);
    }

    // Extract opcode - check for extended opcode (opcode 0xFF indicates extended)
    // In Maxwell, extended opcodes use 0xFF as primary opcode
    uint16_t opcode_val = 0;
    bool is_extended = false;

    uint8_t primary_opcode = static_cast<uint8_t>(inst.raw[0] & 0xFF);

    if (primary_opcode == 0xFF) {
        // Extended opcode: bits [15:8] contain the extended opcode
        is_extended = true;
        opcode_val = static_cast<uint16_t>((inst.raw[0] >> 8) & 0xFF);
    } else {
        // Standard 8-bit opcode
        opcode_val = primary_opcode;
    }

    inst.info = get_instruction_info(static_cast<MaxwellOpcode>(opcode_val));
    inst.info.is_extended = is_extended;

    // Extract predicate (bits [10:8])
    inst.predicate = {RegisterFile::PRED, static_cast<uint16_t>((inst.raw[0] >> 8) & 0x7)};
    inst.predicate_neg = (inst.raw[0] >> 11) & 1;
    inst.cc_update = (inst.raw[0] >> 12) & 1;

    // Decode based on format
    bool success = false;
    switch (inst.info.format) {
        case InstFormat::FMT_R:
            success = decode_fmt_r(inst.raw, inst);
            break;
        case InstFormat::FMT_I:
            success = decode_fmt_i(inst.raw, inst);
            break;
        case InstFormat::FMT_C:
            success = decode_fmt_c(inst.raw, inst);
            break;
        case InstFormat::FMT_LDST:
            success = decode_fmt_ldst(inst.raw, inst);
            break;
        case InstFormat::FMT_TEX:
            success = decode_fmt_tex(inst.raw, inst);
            break;
        case InstFormat::FMT_CTRL:
            success = decode_fmt_ctrl(inst.raw, inst);
            break;
        case InstFormat::FMT_SYNC:
            success = decode_fmt_sync(inst.raw, inst);
            break;
        case InstFormat::FMT_MISC:
            success = decode_fmt_misc(inst.raw, inst);
            break;
    }

    if (!success) {
        last_error_ = "Failed to decode instruction format";
        return std::nullopt;
    }

    // If dual-issue, decode the second instruction as well
    if (is_dual) {
        // The second instruction is at address + 8
        // We'll return the first instruction, and the caller can call decode_instruction again
        // for the second instruction at address + 8 with the same raw[1] word
        // For now, we just store the second raw word for the caller to use
    }

    return inst;
}

// Decode the second instruction of a dual-issue pair
// The caller must provide the second raw word (inst.raw[1]) and the instruction format
std::optional<DecodedInstruction> Decoder::decode_dual_issue_second(
    uint64_t address,
    uint64_t second_raw_word) {

    last_error_.clear();

    DecodedInstruction inst;
    inst.address = address;
    inst.raw[0] = second_raw_word;
    inst.is_dual_issue = true;

    // Extract opcode (bits [7:0])
    uint8_t opcode_val = static_cast<uint8_t>(second_raw_word & 0xFF);
    inst.info = get_instruction_info(static_cast<MaxwellOpcode>(opcode_val));

    // Extract predicate (bits [10:8])
    inst.predicate = {RegisterFile::PRED, static_cast<uint16_t>((second_raw_word >> 8) & 0x7)};
    inst.predicate_neg = (second_raw_word >> 11) & 1;
    inst.cc_update = (second_raw_word >> 12) & 1;

    // Decode based on format
    bool success = false;
    switch (inst.info.format) {
        case InstFormat::FMT_R:
            success = decode_fmt_r(&second_raw_word, inst);
            break;
        case InstFormat::FMT_I:
            success = decode_fmt_i(&second_raw_word, inst);
            break;
        case InstFormat::FMT_C:
            success = decode_fmt_c(&second_raw_word, inst);
            break;
        case InstFormat::FMT_LDST:
            success = decode_fmt_ldst(&second_raw_word, inst);
            break;
        case InstFormat::FMT_TEX:
            success = decode_fmt_tex(&second_raw_word, inst);
            break;
        case InstFormat::FMT_CTRL:
            success = decode_fmt_ctrl(&second_raw_word, inst);
            break;
        case InstFormat::FMT_SYNC:
            success = decode_fmt_sync(&second_raw_word, inst);
            break;
        case InstFormat::FMT_MISC:
            success = decode_fmt_misc(&second_raw_word, inst);
            break;
    }

    if (!success) {
        last_error_ = "Failed to decode second instruction of dual-issue pair";
        return std::nullopt;
    }

    inst.is_dual_issue = true;
    return inst;
}

std::vector<DecodedInstruction> Decoder::decode_instructions(
    const uint8_t* bytes,
    size_t size,
    uint64_t base_address) {

    std::vector<DecodedInstruction> instructions;
    size_t offset = 0;

    while (offset + 8 <= size) {
        auto inst_opt = decode_instruction(base_address + offset, bytes + offset, size - offset);
        if (!inst_opt) {
            break;
        }
        instructions.push_back(*inst_opt);

        // If dual-issue, decode the second instruction as well
        if (inst_opt->is_dual_issue) {
            // Decode the second instruction using the stored raw[1]
            auto second_inst_opt = decode_dual_issue_second(
                base_address + offset + 8, inst_opt->raw[1]);
            if (second_inst_opt) {
                instructions.push_back(*second_inst_opt);
            }
        }

        offset += inst_opt->is_dual_issue ? 16 : 8;
    }

    return instructions;
}

// ============================================================================
// Format-Specific Decoders (Simplified Implementations)
// ============================================================================

bool Decoder::decode_fmt_r(const uint64_t* raw, DecodedInstruction& inst) {
    // R-format: opcode [7:0], pred [10:8], cc [12], dst [25:13], src0 [38:26], src1 [51:39], src2 [63:52]
    // This is a simplified decode - real format is more complex

    uint64_t word = raw[0];

    // Destination register (bits [25:13])
    auto dst_reg = decode_register(word, 13, 13);
    if (!dst_reg) return false;
    inst.dst_operands.push_back(Operand{OperandType::REGISTER, *dst_reg, 0, 0, 0xE4, false, false});

    // Source register 0 (bits [38:26])
    auto src0_reg = decode_register(word, 26, 13);
    if (src0_reg) {
        inst.src_operands.push_back(Operand{OperandType::REGISTER, *src0_reg, 0, 0, 0xE4, false, false});
    }

    // Source register 1 (bits [51:39])
    auto src1_reg = decode_register(word, 39, 13);
    if (src1_reg) {
        inst.src_operands.push_back(Operand{OperandType::REGISTER, *src1_reg, 0, 0, 0xE4, false, false});
    }

    // Source register 2 (bits [63:52]) - for 3-operand instructions
    auto src2_reg = decode_register(word, 52, 11);
    if (src2_reg) {
        inst.src_operands.push_back(Operand{OperandType::REGISTER, *src2_reg, 0, 0, 0xE4, false, false});
    }

    return true;
}

bool Decoder::decode_fmt_i(const uint64_t* raw, DecodedInstruction& inst) {
    // I-format: register-immediate
    // Supports 13-bit, 32-bit, and 64-bit immediates
    uint64_t word = raw[0];

    // Destination
    auto dst_reg = decode_register(word, 13, 13);
    if (!dst_reg) return false;
    inst.dst_operands.push_back(Operand{OperandType::REGISTER, *dst_reg, 0, 0, 0xE4, false, false});

    // Source register
    auto src_reg = decode_register(word, 26, 13);
    if (src_reg) {
        inst.src_operands.push_back(Operand{OperandType::REGISTER, *src_reg, 0, 0, 0xE4, false, false});
    }

    // Immediate encoding (Maxwell ISA):
    // 13-bit immediate: bits [51:39] (sign-extended to 32/64 bits)
    // 32-bit immediate: uses extended format with bit 62 set, value in bits [63:32]
    // 64-bit immediate: uses dual-issue second word (full 64-bit in raw[1])

    // Extract 13-bit immediate (bits [51:39])
    int32_t imm13 = static_cast<int32_t>(extract_bits(word, 39, 13)) << 19 >> 19;

    // Check for 32-bit immediate (bit 62 indicates 32-bit immediate format)
    bool is_32bit_imm = (word >> 62) & 1;

    // Check for 64-bit immediate (dual-issue pair with raw[1] containing the immediate)
    bool is_64bit_imm = false;
    // In Maxwell, 64-bit immediates are encoded as dual-issue pairs
    // The first instruction has a special opcode indicating 64-bit immediate follows

    if (is_32bit_imm && raw[1] != 0) {
        // 32-bit immediate in bits [63:32] of the first word
        uint32_t imm32 = static_cast<uint32_t>(word >> 32);
        inst.src_operands.push_back(Operand{
            OperandType::IMMEDIATE,
            {RegisterFile::GPR, 0},
            static_cast<int64_t>(static_cast<int32_t>(imm32)),
            0, 0, false, false
        });
    } else if (inst.info.opcode == MaxwellOpcode::MOV && (inst.raw[0] & 0xFF) == 0xFF) {
        // Extended opcode MOV with 64-bit immediate in dual-issue second word
        // This is a special case for 64-bit immediates
        if (raw[1] != 0) {
            uint64_t imm64 = raw[1];
            inst.src_operands.push_back(Operand{
                OperandType::IMMEDIATE,
                {RegisterFile::GPR, 0},
                static_cast<int64_t>(imm64),
                0, 0, false, false
            });
        } else {
            // Standard 13-bit sign-extended immediate
            inst.src_operands.push_back(Operand{
                OperandType::IMMEDIATE,
                {RegisterFile::GPR, 0},
                static_cast<int64_t>(imm13),
                0, 0, false, false
            });
        }
    } else {
        // Standard 13-bit sign-extended immediate
        inst.src_operands.push_back(Operand{
            OperandType::IMMEDIATE,
            {RegisterFile::GPR, 0},
            static_cast<int64_t>(imm13),
            0, 0, false, false
        });
    }

    return true;
}

bool Decoder::decode_fmt_c(const uint64_t* raw, DecodedInstruction& inst) {
    // C-format: register-constant
    uint64_t word = raw[0];

    auto dst_reg = decode_register(word, 13, 13);
    if (!dst_reg) return false;
    inst.dst_operands.push_back(Operand{OperandType::REGISTER, *dst_reg, 0, 0, 0xE4, false, false});

    auto src_reg = decode_register(word, 26, 13);
    if (src_reg) {
        inst.src_operands.push_back(Operand{OperandType::REGISTER, *src_reg, 0, 0, 0xE4, false, false});
    }

    // Constant buffer index (bits [51:39])
    uint32_t cbuf_idx = static_cast<uint32_t>(extract_bits(word, 39, 13));
    inst.src_operands.push_back(Operand{OperandType::CONSTANT, {RegisterFile::GPR, 0}, 0, cbuf_idx, 0, false, false});

    return true;
}

bool Decoder::decode_fmt_ldst(const uint64_t* raw, DecodedInstruction& inst) {
    // Load/store format
    uint64_t word = raw[0];

    // Destination (for loads) or source (for stores)
    auto reg = decode_register(word, 13, 13);
    if (!reg) return false;

    if (inst.info.opcode == MaxwellOpcode::ST || inst.info.opcode == MaxwellOpcode::STG ||
        inst.info.opcode == MaxwellOpcode::STS) {
        inst.src_operands.push_back(Operand{OperandType::REGISTER, *reg, 0, 0, 0xE4, false, false});
    } else {
        inst.dst_operands.push_back(Operand{OperandType::REGISTER, *reg, 0, 0, 0xE4, false, false});
    }

    // Address register
    auto addr_reg = decode_register(word, 26, 13);
    if (addr_reg) {
        if (inst.info.opcode == MaxwellOpcode::ST || inst.info.opcode == MaxwellOpcode::STG ||
            inst.info.opcode == MaxwellOpcode::STS) {
            inst.src_operands.push_back(Operand{OperandType::ADDRESS, *addr_reg, 0, 0, 0, false, false});
        } else {
            inst.src_operands.push_back(Operand{OperandType::ADDRESS, *addr_reg, 0, 0, 0, false, false});
        }
    }

    // Offset (immediate)
    int32_t offset = static_cast<int32_t>(extract_bits(word, 39, 13)) << 19 >> 19;
    inst.src_operands.push_back(Operand{OperandType::IMMEDIATE, {RegisterFile::GPR, 0}, offset, 0, 0, false, false});

    // Address register arithmetic: check for ADD/SUB on address registers
    // These are encoded as R-format with ADDR register file
    if (inst.info.opcode == MaxwellOpcode::IADD) {
        // Check if destination or sources are ADDR registers
        for (auto& op : inst.dst_operands) {
            if (op.type == OperandType::REGISTER && op.reg.file == RegisterFile::ADDR) {
                op.type = OperandType::ADDRESS;
            }
        }
        for (auto& op : inst.src_operands) {
            if (op.type == OperandType::REGISTER && op.reg.file == RegisterFile::ADDR) {
                op.type = OperandType::ADDRESS;
            }
        }
    }

    return true;
}

bool Decoder::decode_fmt_tex(const uint64_t* raw, DecodedInstruction& inst) {
    // Texture format
    uint64_t word = raw[0];

    // Destination
    auto dst_reg = decode_register(word, 13, 13);
    if (!dst_reg) return false;
    inst.dst_operands.push_back(Operand{OperandType::REGISTER, *dst_reg, 0, 0, 0xE4, false, false});

    // Texture register
    auto tex_reg = decode_register(word, 26, 13);
    if (tex_reg) {
        inst.src_operands.push_back(Operand{OperandType::REGISTER, *tex_reg, 0, 0, 0xE4, false, false});
    }

    // Sampler register
    auto samp_reg = decode_register(word, 39, 13);
    if (samp_reg) {
        inst.src_operands.push_back(Operand{OperandType::REGISTER, *samp_reg, 0, 0, 0xE4, false, false});
    }

    // Coordinates register
    auto coord_reg = decode_register(word, 52, 11);
    if (coord_reg) {
        inst.src_operands.push_back(Operand{OperandType::REGISTER, *coord_reg, 0, 0, 0xE4, false, false});
    }

    return true;
}

bool Decoder::decode_fmt_ctrl(const uint64_t* raw, DecodedInstruction& inst) {
    // Control flow format
    uint64_t word = raw[0];

    // Target address (bits [39:13] - 27 bits, word-aligned)
    uint32_t target = static_cast<uint32_t>(extract_bits(word, 13, 27)) * 4;
    inst.control_flow_target = target;

    return true;
}

bool Decoder::decode_fmt_sync(const uint64_t* raw, DecodedInstruction& inst) {
    // Synchronization format - typically no operands or simple barrier ID
    uint64_t word = raw[0];

    // Barrier ID (bits [25:13])
    uint32_t barrier_id = static_cast<uint32_t>(extract_bits(word, 13, 13));
    if (barrier_id != 0) {
        inst.src_operands.push_back(Operand{OperandType::IMMEDIATE, {RegisterFile::GPR, 0}, barrier_id, 0, 0, false, false});
    }

    return true;
}

bool Decoder::decode_fmt_misc(const uint64_t* raw, DecodedInstruction& inst) {
    // Miscellaneous format - varies by opcode
    // For now, just handle special register operands for S2R/R2S
    if (inst.info.opcode == MaxwellOpcode::S2R || inst.info.opcode == MaxwellOpcode::R2S) {
        uint64_t word = raw[0];

        auto reg = decode_register(word, 13, 13);
        if (!reg) return false;

        // Special register index (bits [25:13])
        uint32_t sr_index = static_cast<uint32_t>(extract_bits(word, 13, 13));
        Register sr_reg = {RegisterFile::SR, static_cast<uint16_t>(sr_index)};

        if (inst.info.opcode == MaxwellOpcode::S2R) {
            inst.dst_operands.push_back(Operand{OperandType::REGISTER, *reg, 0, 0, 0xE4, false, false});
            inst.src_operands.push_back(Operand{OperandType::REGISTER, sr_reg, 0, 0, 0xE4, false, false});
        } else {
            inst.src_operands.push_back(Operand{OperandType::REGISTER, *reg, 0, 0, 0xE4, false, false});
            inst.dst_operands.push_back(Operand{OperandType::REGISTER, sr_reg, 0, 0, 0xE4, false, false});
        }
    }

    return true;
}

// ============================================================================
// Operand Decoder Helpers
// ============================================================================

// ============================================================================
// Improved Register Decoding with Predicate Logic Support
// ============================================================================

std::optional<Register> Decoder::decode_register(uint64_t bits, int shift, int width) {
    uint64_t reg_bits = extract_bits(bits, shift, width);

    // Register encoding (Maxwell):
    // Bits [11:8] = register file (4 bits)
    // Bits [7:0]  = register index (8 bits for GPR, 3 bits for predicate)
    // For ADDR registers, bits [11:0] can be 12-bit index

    uint32_t file_bits = static_cast<uint32_t>(extract_bits(reg_bits, 8, 4));
    uint32_t index_bits = static_cast<uint32_t>(reg_bits & 0xFF);

    RegisterFile file = RegisterFile::GPR;
    switch (file_bits) {
        case 0: file = RegisterFile::GPR; break;
        case 1: file = RegisterFile::PRED; break;
        case 2: file = RegisterFile::CCR; break;
        case 3: file = RegisterFile::SR; break;
        case 4: file = RegisterFile::UR; break;
        case 5: file = RegisterFile::CR; break;
        case 6: file = RegisterFile::TR; break;
        case 7: file = RegisterFile::SAMPLER; break;
        case 8: file = RegisterFile::ADDR; break;
        default: file = RegisterFile::GPR; break;
    }

    // For predicate registers, index is only 3 bits (P0-P7)
    if (file == RegisterFile::PRED) {
        index_bits &= 0x7;
    }
    // For address registers, use 12-bit index (bits [11:0])
    else if (file == RegisterFile::ADDR) {
        index_bits = static_cast<uint32_t>(reg_bits & 0xFFF);
    }

    return Register{file, static_cast<uint16_t>(index_bits)};
}

// ============================================================================
// Predicate Logic Helpers
// ============================================================================

// Convert predicate register to SPIR-V condition
uint32_t Decoder::predicate_to_spv_condition(uint8_t pred_reg, bool negated) const {
    // In SPIR-V, we use OpSelect with predicate
    // pred_reg 0-7 maps to P0-P7
    // negated: true means use ~predicate
    (void)pred_reg; (void)negated;
    return 0; // Placeholder - actual implementation would map to SPIR-V select
}

} // namespace kirbyrecomp::shader::maxwell