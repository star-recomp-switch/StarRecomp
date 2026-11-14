#include "aarch64_decoder.h"
#include <capstone/arm64.h>
#include <algorithm>
#include <cctype>

namespace kirbyrecomp::translator {

AArch64Decoder::AArch64Decoder() = default;

std::optional<ir::IrInstruction> AArch64Decoder::decode_instruction(
    std::span<const uint8_t> bytes,
    uint64_t address
) {
    auto detail = capstone_.decode_instruction(bytes, address);
    if (!detail.has_value()) {
        return std::nullopt;
    }

    const InstructionDetail& insn = detail.value();
    auto opcode = map_opcode(insn.id);
    if (!opcode.has_value()) {
        // Unrecognized instruction - return invalid
        return ir::IrInstruction{ir::IrOpcode::Invalid};
    }

    // Dispatch to appropriate decoder based on opcode category
    switch (opcode.value()) {
        case ir::IrOpcode::Add:
        case ir::IrOpcode::Sub:
        case ir::IrOpcode::And:
        case ir::IrOpcode::Orr:
        case ir::IrOpcode::Eor:
        case ir::IrOpcode::Lsl:
        case ir::IrOpcode::Lsr:
        case ir::IrOpcode::Asr:
        case ir::IrOpcode::Mul:
        case ir::IrOpcode::Udiv:
        case ir::IrOpcode::Sdiv:
            return decode_alu(insn);

        case ir::IrOpcode::Ldr:
        case ir::IrOpcode::Str:
        case ir::IrOpcode::Ldp:
        case ir::IrOpcode::Stp:
            return decode_load_store(insn);

        case ir::IrOpcode::B:
        case ir::IrOpcode::Bl:
        case ir::IrOpcode::Br:
        case ir::IrOpcode::Blr:
        case ir::IrOpcode::Cbz:
        case ir::IrOpcode::Cbnz:
        case ir::IrOpcode::Tbz:
        case ir::IrOpcode::Tbnz:
            return decode_branch(insn);

        case ir::IrOpcode::Cmp:
        case ir::IrOpcode::Cmn:
        case ir::IrOpcode::Tst:
            return decode_compare(insn);

        case ir::IrOpcode::Mov:
        case ir::IrOpcode::Movk:
        case ir::IrOpcode::Movn:
        case ir::IrOpcode::Movz:
            return decode_move(insn);

        case ir::IrOpcode::Svc:
        case ir::IrOpcode::Hvc:
        case ir::IrOpcode::Smc:
        case ir::IrOpcode::Mrs:
        case ir::IrOpcode::Msr:
            return decode_system(insn);

        default:
            return ir::IrInstruction{ir::IrOpcode::Invalid};
    }
}

std::vector<ir::IrInstruction> AArch64Decoder::decode_basic_block(
    std::span<const uint8_t> bytes,
    uint64_t start_address,
    size_t max_instructions
) {
    std::vector<ir::IrInstruction> instructions;
    instructions.reserve(std::min(max_instructions, bytes.size() / 4));

    uint64_t current_addr = start_address;
    size_t offset = 0;

    while (offset < bytes.size() && instructions.size() < max_instructions) {
        // ARM64 instructions are always 4 bytes
        if (offset + 4 > bytes.size()) {
            break;
        }

        std::span<const uint8_t> insn_bytes(bytes.data() + offset, 4);
        auto insn = decode_instruction(insn_bytes, current_addr);

        if (!insn.has_value()) {
            break;
        }

        instructions.push_back(insn.value());

        // Stop at unconditional branch or return-like instructions
        const auto& decoded = instructions.back();
        if (decoded.opcode == ir::IrOpcode::B ||
            decoded.opcode == ir::IrOpcode::Bl ||
            decoded.opcode == ir::IrOpcode::Br ||
            decoded.opcode == ir::IrOpcode::Blr ||
            decoded.opcode == ir::IrOpcode::Ret) {  // If we add Ret later
            break;
        }

        offset += 4;
        current_addr += 4;
    }

    return instructions;
}

std::string AArch64Decoder::get_last_error() const {
    return capstone_.get_last_error_message();
}

std::optional<ir::IrOpcode> AArch64Decoder::map_opcode(cs_insn_id capstone_id) const {
    using namespace kirbyrecomp::translator;
    // Map Capstone ARM64 instruction IDs to our IR opcodes
    // Reference: Capstone's arm64.h ARM64_INS_* enum
    switch (static_cast<unsigned int>(capstone_id)) {
        // Integer ALU
        case ARM64_INS_ADD:  return ir::IrOpcode::Add;
        case ARM64_INS_SUB:  return ir::IrOpcode::Sub;
        case ARM64_INS_AND:  return ir::IrOpcode::And;
        case ARM64_INS_ORR:  return ir::IrOpcode::Orr;
        case ARM64_INS_EOR:  return ir::IrOpcode::Eor;
        case ARM64_INS_LSL:  return ir::IrOpcode::Lsl;
        case ARM64_INS_LSR:  return ir::IrOpcode::Lsr;
        case ARM64_INS_ASR:  return ir::IrOpcode::Asr;
        case ARM64_INS_MUL:  return ir::IrOpcode::Mul;
        case ARM64_INS_UDIV: return ir::IrOpcode::Udiv;
        case ARM64_INS_SDIV: return ir::IrOpcode::Sdiv;

        // Load/Store (single register)
        case ARM64_INS_LDR:  return ir::IrOpcode::Ldr;
        case ARM64_INS_STR:  return ir::IrOpcode::Str;

        // Load/Store (register pair)
        case ARM64_INS_LDP:  return ir::IrOpcode::Ldp;
        case ARM64_INS_STP:  return ir::IrOpcode::Stp;

        // Branches
        case ARM64_INS_B:    return ir::IrOpcode::B;
        case ARM64_INS_BL:   return ir::IrOpcode::Bl;
        case ARM64_INS_BR:   return ir::IrOpcode::Br;
        case ARM64_INS_BLR:  return ir::IrOpcode::Blr;
        case ARM64_INS_CBZ:  return ir::IrOpcode::Cbz;
        case ARM64_INS_CBNZ: return ir::IrOpcode::Cbnz;
        case ARM64_INS_TBZ:  return ir::IrOpcode::Tbz;
        case ARM64_INS_TBNZ: return ir::IrOpcode::Tbnz;

        // Compare
        case ARM64_INS_CMP:  return ir::IrOpcode::Cmp;
        case ARM64_INS_CMN:  return ir::IrOpcode::Cmn;
        case ARM64_INS_TST:  return ir::IrOpcode::Tst;

        // Move
        case ARM64_INS_MOV:  return ir::IrOpcode::Mov;
        case ARM64_INS_MOVK: return ir::IrOpcode::Movk;
        case ARM64_INS_MOVN: return ir::IrOpcode::Movn;
        case ARM64_INS_MOVZ: return ir::IrOpcode::Movz;

        // System
        case ARM64_INS_SVC:  return ir::IrOpcode::Svc;
        case ARM64_INS_HVC:  return ir::IrOpcode::Hvc;
        case ARM64_INS_SMC:  return ir::IrOpcode::Smc;
        case ARM64_INS_MRS:  return ir::IrOpcode::Mrs;
        case ARM64_INS_MSR:  return ir::IrOpcode::Msr;

        default:
            return std::nullopt;
    }
}

std::optional<uint8_t> AArch64Decoder::reg_to_index(Arm64Register reg, bool& is_64bit) const {
    // Map Capstone ARM64 register enum to register index (0-31)
    // X0-X30, SP are indices 0-31
    // W0-W30, WSP are 32-bit views of same registers
    unsigned int r = static_cast<unsigned int>(reg);

    // 64-bit registers: X0-X30, SP
    if (r >= ARM64_REG_X0 && r <= ARM64_REG_X30) {
        is_64bit = true;
        return static_cast<uint8_t>(r - ARM64_REG_X0);
    }
    if (r == ARM64_REG_SP) {
        is_64bit = true;
        return 31;  // SP is index 31
    }

    // 32-bit registers: W0-W30, WSP
    if (r >= ARM64_REG_W0 && r <= ARM64_REG_W30) {
        is_64bit = false;
        return static_cast<uint8_t>(r - ARM64_REG_W0);
    }
    if (r == ARM64_REG_WSP) {
        is_64bit = false;
        return 31;  // WSP is index 31
    }

    // XZR/WZR are encoded as register 31 in ARM64 but we treat them specially
    // In AArch64, XZR is X31, WZR is W31
    if (r == ARM64_REG_XZR || r == ARM64_REG_WZR) {
        is_64bit = (r == ARM64_REG_XZR);
        return 31;  // ZR uses index 31
    }

    return std::nullopt;
}

std::optional<ir::IrOperand> AArch64Decoder::extract_register(const Arm64Operand& op) const {
    bool is_64bit = true;
    auto idx = reg_to_index(op.reg, is_64bit);
    if (!idx.has_value()) {
        return std::nullopt;
    }
    return ir::IrOperand::Reg(idx.value(), is_64bit);
}

std::optional<ir::IrOperand> AArch64Decoder::extract_immediate(const Arm64Operand& op) const {
    // Determine bit width from the instruction context if possible
    // For now, assume 64-bit for immediates in ALU ops, but could be smaller
    // The actual bit width is encoded in the instruction
    uint8_t bit_width = 64;
    bool is_signed = false;

    // For logical immedials (MOVK/MOVN/MOVZ), the immediate is 16-bit
    // For ADD/SUB immediate, it's 12-bit
    // For now, default to 64-bit

    return ir::IrOperand::Imm(static_cast<uint64_t>(op.imm), bit_width, is_signed);
}

std::optional<ir::IrOperand> AArch64Decoder::extract_memory(const Arm64Operand& op, uint8_t access_size) const {
    bool base_is_64bit = true;
    auto base_idx = reg_to_index(op.mem.base, base_is_64bit);
    if (!base_idx.has_value()) {
        return std::nullopt;
    }

    std::optional<uint8_t> index_idx = std::nullopt;
    if (op.mem.index != Arm64Register::Invalid) {
        bool index_is_64bit = true;
        index_idx = reg_to_index(op.mem.index, index_is_64bit);
    }

    // Capstone displacement is signed
    int64_t disp = op.mem.disp;

    // Determine if this is a sign-extending load (e.g., LDAR, LDARH, etc.)
    // For now, default to false
    bool is_signed = false;

    return ir::IrOperand::Mem(base_idx.value(), disp, access_size, index_idx, is_signed);
}

std::optional<ir::IrOperand> AArch64Decoder::extract_label(const Arm64Operand& op, uint64_t pc) const {
    if (op.type == Arm64OperandType::Immediate) {
        // Branch target is PC-relative offset encoded as immediate
        // For B/BL: 26-bit signed offset << 2
        // For CBZ/CBNZ/TBZ/TBNZ: 19-bit signed offset << 2
        // Capstone already resolves this to absolute address in op.imm
        uint64_t target_addr = static_cast<uint64_t>(op.imm);
        std::string label_name = "loc_" + std::to_string(target_addr);
        return ir::IrOperand::Label(label_name, target_addr);
    }
    return std::nullopt;
}

uint8_t AArch64Decoder::get_memory_access_size(const InstructionDetail& detail) const {
    // Determine access size from mnemonic suffix or register size
    const std::string& mnemonic = detail.mnemonic;

    // Check for size suffixes in mnemonic: b, h, w, d (8, 16, 32, 64 bits)
    // Also check for vector sizes: q (128-bit)
    if (mnemonic.size() >= 2) {
        char suffix = mnemonic[mnemonic.size() - 1];
        switch (suffix) {
            case 'b': return 1;   // byte (8-bit)
            case 'h': return 2;   // halfword (16-bit)
            case 'w': return 4;   // word (32-bit)
            case 'd': return 8;   // doubleword (64-bit)
            case 'q': return 16;  // quadword (128-bit)
        }
    }

    // Infer from register type (first operand for load, second for store)
    if (!detail.operands.empty()) {
        const auto& op = detail.operands[0];
        if (op.type == Arm64OperandType::Register) {
            bool is_64bit = true;
            auto idx = reg_to_index(op.reg, is_64bit);
            if (idx.has_value()) {
                return is_64bit ? 8 : 4;
            }
        }
    }

    // Default to 64-bit
    return 8;
}

bool AArch64Decoder::is_sp(Arm64Register reg) {
    return reg == Arm64Register::SP || reg == Arm64Register::WSP;
}

bool AArch64Decoder::is_zr(Arm64Register reg) {
    return reg == Arm64Register::XZR || reg == Arm64Register::WZR;
}

std::optional<ir::IrInstruction> AArch64Decoder::decode_alu(const InstructionDetail& detail) const {
    auto opcode = map_opcode(detail.id);
    if (!opcode.has_value()) {
        return std::nullopt;
    }

    ir::IrInstruction insn(opcode.value());
    insn.address = detail.address;
    insn.flags_affected = detail.update_flags;

    // ALU instructions typically have: Rd, Rn, Rm (or Rd, Rn, #imm)
    // Operand order in Capstone: dest, src1, src2/src3
    if (detail.operands.size() >= 2) {
        // Destination register
        auto dst = extract_register(detail.operands[0]);
        if (dst.has_value()) {
            insn.operands.push_back(dst.value());
            if (dst.value().AsRegister()) {
                insn.result_register = dst.value().AsRegister()->index;
            }
        }

        // Source register 1
        auto src1 = extract_register(detail.operands[1]);
        if (src1.has_value()) {
            insn.operands.push_back(src1.value());
        }

        // Source register 2 or immediate
        if (detail.operands.size() >= 3) {
            const auto& op3 = detail.operands[2];
            if (op3.type == Arm64OperandType::Register) {
                auto src2 = extract_register(op3);
                if (src2.has_value()) {
                    insn.operands.push_back(src2.value());
                }
            } else if (op3.type == Arm64OperandType::Immediate) {
                auto imm = extract_immediate(op3);
                if (imm.has_value()) {
                    insn.operands.push_back(imm.value());
                }
            }
        }
    }

    return insn;
}

std::optional<ir::IrInstruction> AArch64Decoder::decode_load_store(const InstructionDetail& detail) const {
    auto opcode = map_opcode(detail.id);
    if (!opcode.has_value()) {
        return std::nullopt;
    }

    ir::IrInstruction insn(opcode.value());
    insn.address = detail.address;

    uint8_t access_size = get_memory_access_size(detail);

    // Load/Store single: Rt, [Rn, #offset] or Rt, [Rn, Rm]
    // Load/Store pair: Rt1, Rt2, [Rn, #offset]
    if (detail.operands.size() >= 2) {
        if (opcode == ir::IrOpcode::Ldp || opcode == ir::IrOpcode::Stp) {
            // Pair: 3 register operands (Rt1, Rt2, Rn) + optional offset
            if (detail.operands.size() >= 3) {
                // First register (Rt1)
                auto rt1 = extract_register(detail.operands[0]);
                if (rt1.has_value()) {
                    insn.operands.push_back(rt1.value());
                    if (rt1.value().AsRegister()) {
                        insn.result_register = rt1.value().AsRegister()->index;
                    }
                }

                // Second register (Rt2)
                auto rt2 = extract_register(detail.operands[1]);
                if (rt2.has_value()) {
                    insn.operands.push_back(rt2.value());
                }

                // Base register (Rn) - memory operand
                const auto& mem_op = detail.operands[2];
                if (mem_op.type == Arm64OperandType::Memory) {
                    auto mem = extract_memory(mem_op, access_size);
                    if (mem.has_value()) {
                        insn.operands.push_back(mem.value());
                    }
                } else if (mem_op.type == Arm64OperandType::Register) {
                    // Register base with no offset
                    auto base = extract_register(mem_op);
                    if (base.has_value()) {
                        // Create memory operand with zero offset
                        insn.operands.push_back(ir::IrOperand::Mem(
                            base.value().AsRegister()->index, 0, access_size
                        ));
                    }
                }

                // Optional immediate offset (4th operand)
                if (detail.operands.size() >= 4) {
                    const auto& off_op = detail.operands[3];
                    if (off_op.type == Arm64OperandType::Immediate) {
                        // The offset is already included in the memory operand from Capstone
                        // Capstone puts the full [base, offset] in the memory operand
                    }
                }
            }
        } else {
            // Single register LDR/STR
            // Operand 0: Rt (destination for load, source for store)
            auto rt = extract_register(detail.operands[0]);
            if (rt.has_value()) {
                insn.operands.push_back(rt.value());
                if (opcode == ir::IrOpcode::Ldr && rt.value().AsRegister()) {
                    insn.result_register = rt.value().AsRegister()->index;
                }
            }

            // Operand 1: Memory operand [Rn, #offset]
            if (detail.operands.size() >= 2) {
                const auto& mem_op = detail.operands[1];
                if (mem_op.type == Arm64OperandType::Memory) {
                    auto mem = extract_memory(mem_op, access_size);
                    if (mem.has_value()) {
                        insn.operands.push_back(mem.value());
                    }
                } else if (mem_op.type == Arm64OperandType::Register) {
                    // Register indirect with no offset
                    auto base = extract_register(mem_op);
                    if (base.has_value()) {
                        insn.operands.push_back(ir::IrOperand::Mem(
                            base.value().AsRegister()->index, 0, access_size
                        ));
                    }
                }
            }
        }
    }

    return insn;
}

std::optional<ir::IrInstruction> AArch64Decoder::decode_branch(const InstructionDetail& detail) const {
    auto opcode = map_opcode(detail.id);
    if (!opcode.has_value()) {
        return std::nullopt;
    }

    ir::IrInstruction insn(opcode.value());
    insn.address = detail.address;

    switch (opcode.value()) {
        case ir::IrOpcode::B:
        case ir::IrOpcode::Bl: {
            // Unconditional branch: single label operand
            if (!detail.operands.empty()) {
                auto label = extract_label(detail.operands[0], detail.address);
                if (label.has_value()) {
                    insn.operands.push_back(label.value());
                }
            }
            break;
        }

        case ir::IrOpcode::Br:
        case ir::IrOpcode::Blr: {
            // Register branch: single register operand
            if (!detail.operands.empty()) {
                auto reg = extract_register(detail.operands[0]);
                if (reg.has_value()) {
                    insn.operands.push_back(reg.value());
                }
            }
            break;
        }

        case ir::IrOpcode::Cbz:
        case ir::IrOpcode::Cbnz: {
            // Compare and branch on zero/non-zero: Rn, label
            if (detail.operands.size() >= 2) {
                auto reg = extract_register(detail.operands[0]);
                if (reg.has_value()) {
                    insn.operands.push_back(reg.value());
                }
                auto label = extract_label(detail.operands[1], detail.address);
                if (label.has_value()) {
                    insn.operands.push_back(label.value());
                }
            }
            break;
        }

        case ir::IrOpcode::Tbz:
        case ir::IrOpcode::Tbnz: {
            // Test bit and branch: Rn, #bit, label
            if (detail.operands.size() >= 3) {
                auto reg = extract_register(detail.operands[0]);
                if (reg.has_value()) {
                    insn.operands.push_back(reg.value());
                }
                // Bit position immediate
                if (detail.operands[1].type == Arm64OperandType::Immediate) {
                    auto bit = extract_immediate(detail.operands[1]);
                    if (bit.has_value()) {
                        insn.operands.push_back(bit.value());
                    }
                }
                auto label = extract_label(detail.operands[2], detail.address);
                if (label.has_value()) {
                    insn.operands.push_back(label.value());
                }
            }
            break;
        }

        default:
            break;
    }

    return insn;
}

std::optional<ir::IrInstruction> AArch64Decoder::decode_compare(const InstructionDetail& detail) const {
    auto opcode = map_opcode(detail.id);
    if (!opcode.has_value()) {
        return std::nullopt;
    }

    ir::IrInstruction insn(opcode.value());
    insn.address = detail.address;
    insn.flags_affected = true;  // Compare instructions always affect flags

    // Compare instructions: Rn, Rm (or Rn, #imm)
    if (detail.operands.size() >= 2) {
        auto rn = extract_register(detail.operands[0]);
        if (rn.has_value()) {
            insn.operands.push_back(rn.value());
        }

        const auto& op2 = detail.operands[1];
        if (op2.type == Arm64OperandType::Register) {
            auto rm = extract_register(op2);
            if (rm.has_value()) {
                insn.operands.push_back(rm.value());
            }
        } else if (op2.type == Arm64OperandType::Immediate) {
            auto imm = extract_immediate(op2);
            if (imm.has_value()) {
                insn.operands.push_back(imm.value());
            }
        }
    }

    return insn;
}

std::optional<ir::IrInstruction> AArch64Decoder::decode_move(const InstructionDetail& detail) const {
    auto opcode = map_opcode(detail.id);
    if (!opcode.has_value()) {
        return std::nullopt;
    }

    ir::IrInstruction insn(opcode.value());
    insn.address = detail.address;

    // MOV: Rd, Rn (or Rd, #imm)
    // MOVK/MOVN/MOVZ: Rd, #imm, LSL #shift
    if (!detail.operands.empty()) {
        // Destination register
        auto rd = extract_register(detail.operands[0]);
        if (rd.has_value()) {
            insn.operands.push_back(rd.value());
            if (rd.value().AsRegister()) {
                insn.result_register = rd.value().AsRegister()->index;
            }
        }

        // Source: register or immediate
        if (detail.operands.size() >= 2) {
            const auto& op2 = detail.operands[1];
            if (op2.type == Arm64OperandType::Register) {
                auto rn = extract_register(op2);
                if (rn.has_value()) {
                    insn.operands.push_back(rn.value());
                }
            } else if (op2.type == Arm64OperandType::Immediate) {
                auto imm = extract_immediate(op2);
                if (imm.has_value()) {
                    insn.operands.push_back(imm.value());
                }
            }
        }

        // Shift amount for MOVK/MOVN/MOVZ (3rd operand)
        if (detail.operands.size() >= 3) {
            const auto& op3 = detail.operands[2];
            if (op3.type == Arm64OperandType::Immediate) {
                auto shift = extract_immediate(op3);
                if (shift.has_value()) {
                    insn.operands.push_back(shift.value());
                }
            }
        }
    }

    return insn;
}

std::optional<ir::IrInstruction> AArch64Decoder::decode_system(const InstructionDetail& detail) const {
    auto opcode = map_opcode(detail.id);
    if (!opcode.has_value()) {
        return std::nullopt;
    }

    ir::IrInstruction insn(opcode.value());
    insn.address = detail.address;

    switch (opcode.value()) {
        case ir::IrOpcode::Svc:
        case ir::IrOpcode::Hvc:
        case ir::IrOpcode::Smc: {
            // Exception generation: #imm
            if (!detail.operands.empty() && detail.operands[0].type == Arm64OperandType::Immediate) {
                auto imm = extract_immediate(detail.operands[0]);
                if (imm.has_value()) {
                    insn.operands.push_back(imm.value());
                }
            }
            break;
        }

        case ir::IrOpcode::Mrs: {
            // MRS: Rd, sysreg
            if (detail.operands.size() >= 2) {
                auto rd = extract_register(detail.operands[0]);
                if (rd.has_value()) {
                    insn.operands.push_back(rd.value());
                    if (rd.value().AsRegister()) {
                        insn.result_register = rd.value().AsRegister()->index;
                    }
                }
                // System register operand - treat as immediate for now
                if (detail.operands[1].type == Arm64OperandType::Immediate) {
                    auto sysreg = extract_immediate(detail.operands[1]);
                    if (sysreg.has_value()) {
                        insn.operands.push_back(sysreg.value());
                    }
                }
            }
            break;
        }

        case ir::IrOpcode::Msr: {
            // MSR: sysreg, Rn
            if (detail.operands.size() >= 2) {
                // System register (treated as immediate)
                if (detail.operands[0].type == Arm64OperandType::Immediate) {
                    auto sysreg = extract_immediate(detail.operands[0]);
                    if (sysreg.has_value()) {
                        insn.operands.push_back(sysreg.value());
                    }
                }
                // Source register
                auto rn = extract_register(detail.operands[1]);
                if (rn.has_value()) {
                    insn.operands.push_back(rn.value());
                }
            }
            break;
        }

        default:
            break;
    }

    return insn;
}

} // namespace kirbyrecomp::translator