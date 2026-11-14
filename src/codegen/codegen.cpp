#include "codegen.h"

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace kr::codegen {

namespace {

using sr::common::u32;
using sr::common::u64;
using sr::common::s64;
using sr::numeric::FlagsResult;

// Fast unsigned 64-bit to decimal conversion
int FastU64ToDec(u64 v, char* buf) {
    if (v == 0) { buf[0] = '0'; return 1; }
    char tmp[21];
    int n = 0;
    while (v > 0) {
        tmp[n++] = static_cast<char>('0' + (v % 10));
        v /= 10;
    }
    for (int i = 0; i < n; ++i) buf[i] = tmp[n - 1 - i];
    return n;
}

void AppendDec(std::string& out, u64 v) {
    char buf[21];
    int n = FastU64ToDec(v, buf);
    out.append(buf, static_cast<size_t>(n));
}

// Append a u64 in lowercase hex
void AppendHex(std::string& out, u64 v) {
    if (v == 0) { out.push_back('0'); return; }
    char tmp[17];
    int n = 0;
    while (v > 0) {
        int d = static_cast<int>(v & 0xF);
        tmp[n++] = static_cast<char>(d < 10 ? '0' + d : 'a' + (d - 10));
        v >>= 4;
    }
    for (int i = 0; i < n; ++i) out.push_back(tmp[n - 1 - i]);
}

// Get register name for X0-X30, SP, PC, ZR
std::string GetRegisterName(uint8_t index, bool is_64bit) {
    if (index == 31) {
        return is_64bit ? "sp" : "wsp";
    }
    if (index == 30) {
        return is_64bit ? "lr" : "w30";
    }
    if (is_64bit) {
        return "x" + std::to_string(index);
    }
    return "w" + std::to_string(index);
}

// Get CPU register access expression
std::string GetCpuRegisterAccess(const kirbyrecomp::ir::IrRegister& reg) {
    if (reg.index == 31) { // SP/ZR
        return "cpu->sp";
    }
    if (reg.index == 30) { // LR
        return "cpu->x[30]";
    }
    if (reg.index >= 31) {
        return "0"; // ZR
    }
    return "cpu->x[" + std::to_string(reg.index) + "]";
}

// Get CPU register access for 32-bit (W register)
std::string GetCpuWRegisterAccess(const kirbyrecomp::ir::IrRegister& reg) {
    if (reg.index == 31) {
        return "(uint32_t)cpu->sp";
    }
    if (reg.index == 30) {
        return "(uint32_t)cpu->x[30]";
    }
    if (reg.index >= 31) {
        return "0";
    }
    return "(uint32_t)cpu->x[" + std::to_string(reg.index) + "]";
}

// Stringify an operand to a C expression
std::string StringifyOperand(const kirbyrecomp::ir::IrOperand& op, bool is32) {
    if (op.type == kirbyrecomp::ir::IrOperandType::Register) {
        const auto* reg = op.AsRegister();
        if (!reg) return "0";
        return is32 ? GetCpuWRegisterAccess(*reg) : GetCpuRegisterAccess(*reg);
    }
    if (op.type == kirbyrecomp::ir::IrOperandType::Immediate) {
        const auto* imm = op.AsImmediate();
        if (!imm) return "0";
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%lluULL", static_cast<unsigned long long>(imm->value));
        return buf;
    }
    if (op.type == kirbyrecomp::ir::IrOperandType::Memory) {
        const auto* mem = op.AsMemory();
        if (!mem) return "0";
        std::string base = GetCpuRegisterAccess({mem->base_register, true});
        std::string expr = "sr_ldr64(cpu, mem, " + base;
        if (mem->offset != 0) {
            expr += " + " + std::to_string(mem->offset);
        }
        expr += ")";
        return expr;
    }
    if (op.type == kirbyrecomp::ir::IrOperandType::Label) {
        const auto* label = op.AsLabel();
        if (!label) return "0";
        return label->name;
    }
    return "0";
}

// Emit a comment if enabled
void EmitComment(std::ostringstream& out, const std::string& comment, bool enabled) {
    if (enabled && !comment.empty()) {
        out << "    // " << comment << "\n";
    }
}

// Emit NZCV flag computation for integer operations
std::string EmitFlagsUpdate(const std::string& result_expr, bool is32, bool set_carry = false, bool carry_val = false) {
    std::ostringstream os;
    if (is32) {
        os << "cpu->pstate = sr::numeric::LogicFlags(" << result_expr << ", true);";
    } else {
        os << "cpu->pstate = sr::numeric::LogicFlags(" << result_expr << ", false);";
    }
    return os.str();
}

// Emit ADDS/SUBS with flags
std::string EmitAddSubWithFlags(const std::string& a, const std::string& b, bool is_sub, bool is32) {
    std::ostringstream os;
    if (is_sub) {
        os << "sr::numeric::SubWithFlags(" << a << ", " << b << ", " << (is32 ? "true" : "false") << ")";
    } else {
        os << "sr::numeric::AddWithFlags(" << a << ", " << b << ", " << (is32 ? "true" : "false") << ")";
    }
    return os.str();
}

// Emit MOV immediate family (MOV, MOVK, MOVN, MOVZ)
std::string EmitMovImmediate(const kirbyrecomp::ir::IrInstruction& insn,
                             const std::string& dst, bool is32) {
    std::ostringstream os;
    if (insn.operands.size() < 2) return "0";

    const auto* imm = insn.operands[1].AsImmediate();
    if (!imm) return "0";

    u64 imm_val = imm->value;
    int shift = 0;

    // Check for shift operand
    if (insn.operands.size() >= 3) {
        const auto* shift_op = insn.operands[2].AsImmediate();
        if (shift_op) shift = static_cast<int>(shift_op->value);
    }

    u64 shifted_val = imm_val << shift;
    if (is32) {
        shifted_val &= 0xFFFFFFFFULL;
    }

    switch (insn.opcode) {
        case kirbyrecomp::ir::IrOpcode::Mov:
            os << dst << " = " << shifted_val << "ULL;";
            break;
        case kirbyrecomp::ir::IrOpcode::Movz:
            os << dst << " = " << shifted_val << "ULL;";
            break;
        case kirbyrecomp::ir::IrOpcode::Movk:
            if (is32) {
                u32 mask = 0xFFFFU << shift;
                os << dst << " = (" << dst << " & ~" << mask << "ULL) | " << shifted_val << "ULL;";
            } else {
                u64 mask = 0xFFFFULL << shift;
                os << dst << " = (" << dst << " & ~" << mask << "ULL) | " << shifted_val << "ULL;";
            }
            break;
        case kirbyrecomp::ir::IrOpcode::Movn:
            if (is32) {
                os << dst << " = ~" << shifted_val << "ULL;";
            } else {
                os << dst << " = ~" << shifted_val << "ULL;";
            }
            break;
        default:
            os << dst << " = " << shifted_val << "ULL;";
            break;
    }
    return os.str();
}

// Generate code for a single instruction
void GenerateInstruction(const kirbyrecomp::ir::IrInstruction& insn,
                         std::ostringstream& out,
                         bool use_helpers,
                         bool emit_comments) {
    using IrOpcode = kirbyrecomp::ir::IrOpcode;

    if (emit_comments) {
        out << "    // " << kirbyrecomp::ir::IrOpcodeToString(insn.opcode);
        if (insn.address != 0) {
            out << " @ 0x" << std::hex << insn.address << std::dec;
        }
        out << "\n";
    }

    bool is32 = false;
    std::string dst_expr;

    if (insn.result_register.has_value()) {
        uint8_t rd = insn.result_register.value();
        kirbyrecomp::ir::IrRegister reg{rd, true};
        dst_expr = GetCpuRegisterAccess(reg);
        // Check if destination is a W register (32-bit)
        // For now, we check the opcode or operand types
    }

    const auto* op0 = insn.operands.size() > 0 ? &insn.operands[0] : nullptr;
    const auto* op1 = insn.operands.size() > 1 ? &insn.operands[1] : nullptr;
    const auto* op2 = insn.operands.size() > 2 ? &insn.operands[2] : nullptr; // op2 - used in some cases below
    (void)op2; // suppress unused warning

    switch (insn.opcode) {
        case IrOpcode::Add:
        case IrOpcode::Sub: {
            bool is_sub = (insn.opcode == IrOpcode::Sub);
            if (op0 && op1) {
                std::string a = StringifyOperand(*op0, is32);
                std::string b = StringifyOperand(*op1, is32);

                if (use_helpers && insn.flags_affected) {
                    std::string flags_call = EmitAddSubWithFlags(a, b, is_sub, is32);
                    out << "    {\n";
                    out << "        auto __res = " << flags_call << ";\n";
                    out << "        " << dst_expr << " = __res.value;\n";
                    out << "        cpu->pstate = __res.nzcv;\n";
                    out << "    }\n";
                } else if (use_helpers) {
                    if (is_sub) {
                        out << "    " << dst_expr << " = sr::numeric::Sext32(sr::numeric::SubWithFlags("
                            << a << ", " << b << ", " << (is32 ? "true" : "false") << ").value);\n";
                    } else {
                        out << "    " << dst_expr << " = sr::numeric::Sext32(sr::numeric::AddWithFlags("
                            << a << ", " << b << ", " << (is32 ? "true" : "false") << ").value);\n";
                    }
                } else {
                    out << "    " << dst_expr << " = " << a << (is_sub ? " - " : " + ") << b << ";\n";
                }
            }
            break;
        }
        case IrOpcode::And:
        case IrOpcode::Orr:
        case IrOpcode::Eor: {
            if (op0 && op1) {
                std::string a = StringifyOperand(*op0, is32);
                std::string b = StringifyOperand(*op1, is32);

                if (use_helpers) {
                    const char* fn_name = (insn.opcode == IrOpcode::And) ? "sr_and64" :
                                          (insn.opcode == IrOpcode::Orr) ? "sr_orr64" : "sr_eor64";
                    if (is32) {
                        fn_name = (insn.opcode == IrOpcode::And) ? "sr_and32" :
                                  (insn.opcode == IrOpcode::Orr) ? "sr_orr32" : "sr_eor32";
                    }

                    if (insn.flags_affected) {
                        out << "    {\n";
                        out << "        u64 __res = sr::numeric::" << fn_name << "(" << a << ", " << b << ");\n";
                        out << "        " << dst_expr << " = __res;\n";
                        out << "        cpu->pstate = sr::numeric::LogicFlags(__res, " << (is32 ? "true" : "false") << ");\n";
                        out << "    }\n";
                    } else {
                        out << "    " << dst_expr << " = sr::numeric::" << fn_name << "(" << a << ", " << b << ");\n";
                    }
                } else {
                    const char* op_str = (insn.opcode == IrOpcode::And) ? " & " :
                                         (insn.opcode == IrOpcode::Orr) ? " | " : " ^ ";
                    out << "    " << dst_expr << " = " << a << op_str << b << ";\n";
                }
            }
            break;
        }
        case IrOpcode::Lsl:
        case IrOpcode::Lsr:
        case IrOpcode::Asr: {
            if (op0 && op1) {
                std::string a = StringifyOperand(*op0, is32);
                std::string b = StringifyOperand(*op1, is32);

                if (use_helpers) {
                    const char* fn_name = (insn.opcode == IrOpcode::Lsl) ? "sr_lsl64" :
                                          (insn.opcode == IrOpcode::Lsr) ? "sr_lsr64" : "sr_asr64";
                    if (is32) {
                        fn_name = (insn.opcode == IrOpcode::Lsl) ? "sr_lsl32" :
                                  (insn.opcode == IrOpcode::Lsr) ? "sr_lsr32" : "sr_asr32";
                    }
                    out << "    " << dst_expr << " = sr::numeric::" << fn_name << "(" << a << ", " << b << ");\n";
                } else {
                    const char* op_str = (insn.opcode == IrOpcode::Lsl) ? " << " :
                                         (insn.opcode == IrOpcode::Lsr) ? " >> " : " >> ";
                    out << "    " << dst_expr << " = " << a << op_str << b << ";\n";
                }
            }
            break;
        }
        case IrOpcode::Mul: {
            if (op0 && op1) {
                std::string a = StringifyOperand(*op0, is32);
                std::string b = StringifyOperand(*op1, is32);

                if (use_helpers) {
                    out << "    " << dst_expr << " = sr::numeric::sr_mul64(" << a << ", " << b << ");\n";
                } else {
                    out << "    " << dst_expr << " = " << a << " * " << b << ";\n";
                }
            }
            break;
        }
        case IrOpcode::Udiv: {
            if (op0 && op1) {
                std::string a = StringifyOperand(*op0, is32);
                std::string b = StringifyOperand(*op1, is32);

                if (use_helpers) {
                    out << "    " << dst_expr << " = sr::numeric::sr_udiv64(" << a << ", " << b << ");\n";
                } else {
                    out << "    if (" << b << " != 0) " << dst_expr << " = " << a << " / " << b << "; else " << dst_expr << " = 0;\n";
                }
            }
            break;
        }
        case IrOpcode::Sdiv: {
            if (op0 && op1) {
                std::string a = StringifyOperand(*op0, is32);
                std::string b = StringifyOperand(*op1, is32);

                if (use_helpers) {
                    out << "    " << dst_expr << " = sr::numeric::sr_sdiv64(" << a << ", " << b << ");\n";
                } else {
                    out << "    if (" << b << " != 0) " << dst_expr << " = (int64_t)" << a << " / (int64_t)" << b << "; else " << dst_expr << " = 0;\n";
                }
            }
            break;
        }
        case IrOpcode::Cmp:
        case IrOpcode::Cmn:
        case IrOpcode::Tst: {
            if (op0 && op1) {
                std::string a = StringifyOperand(*op0, is32);
                std::string b = StringifyOperand(*op1, is32);

                if (use_helpers) {
                    const char* fn_name = (insn.opcode == IrOpcode::Cmp) ? "sr_cmp64" :
                                          (insn.opcode == IrOpcode::Cmn) ? "sr_cmn64" : "sr_tst64";
                    if (is32) {
                        fn_name = (insn.opcode == IrOpcode::Cmp) ? "sr_cmp32" :
                                  (insn.opcode == IrOpcode::Cmn) ? "sr_cmn32" : "sr_tst32";
                    }
                    out << "    {\n";
                    out << "        auto __res = sr::numeric::" << fn_name << "(" << a << ", " << b << ");\n";
                    out << "        cpu->pstate = __res.nzcv;\n";
                    out << "    }\n";
                } else {
                    // Fallback inline flag computation
                    out << "    {\n";
                    out << "        u64 __a = " << a << ";\n";
                    out << "        u64 __b = " << b << ";\n";
                    out << "        u64 __r = __a " << (insn.opcode == IrOpcode::Cmp ? "-" : "+") << " __b;\n";
                    out << "        cpu->pstate = ((__r >> 63) & 1) << 31 | ((__r == 0) << 30) | ((__a >= __b) << 29) | (0 << 28);\n";
                    out << "    }\n";
                }
            }
            break;
        }
        case IrOpcode::Mov:
        case IrOpcode::Movk:
        case IrOpcode::Movn:
        case IrOpcode::Movz: {
            if (op0) {
                std::string mov_code = EmitMovImmediate(insn, dst_expr, is32);
                out << "    " << mov_code << "\n";
            }
            break;
        }
        case IrOpcode::Ldr: {
            if (op0 && op1) {
                // op0 is destination register, op1 is memory operand
                const auto* mem = op1->AsMemory();
                if (mem) {
                    std::string base = GetCpuRegisterAccess({mem->base_register, true});
                    std::string offset_str = mem->offset != 0 ? " + " + std::to_string(mem->offset) : "";

                    if (use_helpers) {
                        out << "    " << dst_expr << " = sr_ldr64(cpu, mem, " << base << offset_str << ");\n";
                    } else {
                        out << "    " << dst_expr << " = *(uint64_t*)sr_rt_guest_to_host(cpu, " << base << offset_str << ", 8, 0);\n";
                    }
                }
            }
            break;
        }
        case IrOpcode::Str: {
            if (op0 && op1) {
                // op0 is memory operand, op1 is source register
                const auto* mem = op0->AsMemory();
                if (mem) {
                    std::string base = GetCpuRegisterAccess({mem->base_register, true});
                    std::string src = StringifyOperand(*op1, is32);
                    std::string offset_str = mem->offset != 0 ? " + " + std::to_string(mem->offset) : "";

                    if (use_helpers) {
                        out << "    sr_str64(cpu, mem, " << base << offset_str << ", " << src << ");\n";
                    } else {
                        out << "    *(uint64_t*)sr_rt_guest_to_host(cpu, " << base << offset_str << ", 8, 1) = " << src << ";\n";
                    }
                }
            }
            break;
        }
        case IrOpcode::B: {
            if (op0) {
                const auto* label = op0->AsLabel();
                if (label) {
                    out << "    goto " << label->name << ";\n";
                } else {
                    out << "    goto bb_" << op0->AsImmediate()->value << ";\n";
                }
            }
            break;
        }
        case IrOpcode::Bl: {
            if (op0) {
                const auto* label = op0->AsLabel();
                if (label) {
                    out << "    " << label->name << "(cpu, mem);\n";
                } else {
                    out << "    sr_rt_call(cpu, " << StringifyOperand(*op0, false) << ");\n";
                }
            }
            break;
        }
        case IrOpcode::Br: {
            if (op0) {
                out << "    sr_rt_call(cpu, " << StringifyOperand(*op0, false) << ");\n";
            }
            break;
        }
        case IrOpcode::Blr: {
            if (op0) {
                out << "    sr_rt_call(cpu, " << StringifyOperand(*op0, false) << ");\n";
            }
            break;
        }
        case IrOpcode::Cbz:
        case IrOpcode::Cbnz: {
            if (op0 && op1) {
                std::string reg_expr = StringifyOperand(*op0, true);
                const auto* label = op1->AsLabel();
                std::string label_name = label ? label->name : "bb_" + std::to_string(op1->AsImmediate()->value);

                if (insn.opcode == IrOpcode::Cbz) {
                    out << "    if (" << reg_expr << " == 0) goto " << label_name << ";\n";
                } else {
                    out << "    if (" << reg_expr << " != 0) goto " << label_name << ";\n";
                }
            }
            break;
        }
        case IrOpcode::Tbz:
        case IrOpcode::Tbnz: {
            if (insn.operands.size() >= 3) {
                std::string reg_expr = StringifyOperand(insn.operands[0], true);
                const auto* bit_op = insn.operands[1].AsImmediate();
                const auto* label = insn.operands[2].AsLabel();
                std::string label_name = label ? label->name : "bb_" + std::to_string(insn.operands[2].AsImmediate()->value);
                u64 bit = bit_op ? bit_op->value : 0;

                if (insn.opcode == IrOpcode::Tbz) {
                    out << "    if (((" << reg_expr << " >> " << bit << ") & 1) == 0) goto " << label_name << ";\n";
                } else {
                    out << "    if (((" << reg_expr << " >> " << bit << ") & 1) != 0) goto " << label_name << ";\n";
                }
            }
            break;
        }
        case IrOpcode::Return: {
            out << "    return;\n";
            break;
        }
        case IrOpcode::Svc: {
            if (op0) {
                out << "    sr_rt_handle_svc(cpu, " << StringifyOperand(*op0, false) << ");\n";
            }
            break;
        }
        default: {
            out << "    // Unimplemented: " << kirbyrecomp::ir::IrOpcodeToString(insn.opcode) << "\n";
            break;
        }
    }
}

// Generate function prologue
void GeneratePrologue(const kirbyrecomp::ir::IrFunction& func,
                      std::ostringstream& out,
                      const CodegenOptions& options) {
    std::string func_name = func.name.empty() ? "fn_" + std::to_string(func.address) : func.name;

    out << "void " << func_name << "(SrCpuState* cpu, SrMemory* mem) {\n";

    if (options.emit_comments) {
        out << "    // Function: " << func_name << " @ 0x" << std::hex << func.address << std::dec << "\n";
        out << "    // Stack size: " << func.stack_size << " bytes\n";
    }

    // Declare temporaries for SSA values if needed
    // For now, we use direct register access
}

// Generate function epilogue
void GenerateEpilogue(std::ostringstream& out) {
    out << "}\n\n";
}

// Generate labels for basic blocks
void GenerateBlockLabels(const kirbyrecomp::ir::IrFunction& func,
                         std::ostringstream& out) {
    for (size_t i = 0; i < func.block_starts.size(); ++i) {
        out << "bb_" << i << ":\n";
    }
}

} // anonymous namespace

CodegenResult generate_c(const kirbyrecomp::ir::IrFunction& func,
                         std::ostream& output,
                         const CodegenOptions& options) {
    CodegenResult result;
    std::ostringstream out;

    GeneratePrologue(func, out, options);

    // Generate basic block labels
    for (size_t i = 0; i < func.block_starts.size(); ++i) {
        out << "bb_" << i << ":\n";
    }

    // Generate instructions
    for (const auto& insn : func.instructions) {
        GenerateInstruction(insn, out, options.use_numeric_helpers, options.emit_comments);
    }

    GenerateEpilogue(out);

    result.source_code = out.str();
    result.function_names.push_back(func.name.empty() ? "fn_" + std::to_string(func.address) : func.name);
    result.success = true;

    output << result.source_code;
    return result;
}

CodegenResult generate_c(const kirbyrecomp::ir::IrModule& module,
                         std::ostream& output,
                         const CodegenOptions& options) {
    CodegenResult result;
    std::ostringstream out;

    // Emit includes - runtime.h provides SrCpuState, SrMemory, and helper declarations
    out << "#include <stdint.h>\n";
    out << "#include <stdbool.h>\n\n";
    out << "#include \"kirbyrecomp/numeric.h\"\n";
    out << "#include \"kirbyrecomp/runtime.h\"\n\n";

    // Generate each function
    for (const auto& func : module.functions) {
        CodegenResult func_result = generate_c(func, out, options);
        if (!func_result.success) {
            result.success = false;
            result.error_message = func_result.error_message;
            return result;
        }
        result.function_names.insert(result.function_names.end(),
                                     func_result.function_names.begin(),
                                     func_result.function_names.end());
    }

    result.source_code = out.str();
    result.success = true;

    output << result.source_code;
    return result;
}

CodegenResult generate_c(const kirbyrecomp::ir::IrFunction& func,
                         const CodegenOptions& options) {
    std::ostringstream out;
    CodegenResult result = generate_c(func, out, options);
    return result;
}

CodegenResult generate_c(const kirbyrecomp::ir::IrModule& module,
                         const CodegenOptions& options) {
    std::ostringstream out;
    CodegenResult result = generate_c(module, out, options);
    return result;
}

} // namespace kr::codegen