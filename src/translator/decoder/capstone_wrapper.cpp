#include "capstone_wrapper.h"
#include <capstone/arm64.h>
#include <algorithm>
#include <cstring>

namespace kirbyrecomp::translator {

CapstoneWrapper::CapstoneWrapper()
    : handle_(0)
    , last_error_(CS_ERR_OK)
    , initialized_(false)
{
    // Initialize Capstone for ARM64 little-endian
    cs_err err = cs_open(CS_ARCH_ARM64, CS_MODE_LITTLE_ENDIAN, &handle_);
    if (err != CS_ERR_OK) {
        last_error_ = err;
        handle_ = 0;
        initialized_ = false;
        return;
    }

    // Enable detail mode to get full operand information
    err = cs_option(handle_, CS_OPT_DETAIL, CS_OPT_ON);
    if (err != CS_ERR_OK) {
        last_error_ = err;
        cs_close(&handle_);
        handle_ = 0;
        initialized_ = false;
        return;
    }

    // Set syntax to default (Intel-style for ARM is standard)
    // For ARM64, the default is fine - no special syntax option needed

    initialized_ = true;
    last_error_ = CS_ERR_OK;
}

CapstoneWrapper::~CapstoneWrapper() {
    if (handle_ != 0) {
        cs_close(&handle_);
        handle_ = 0;
    }
    initialized_ = false;
}

std::optional<InstructionDetail> CapstoneWrapper::decode_instruction(
    std::span<const uint8_t> bytes,
    uint64_t address
) {
    if (!initialized_ || handle_ == 0) {
        last_error_ = CS_ERR_HANDLE;
        return std::nullopt;
    }

    if (bytes.empty()) {
        last_error_ = CS_ERR_MEM;
        return std::nullopt;
    }

    // ARM64 instructions are always 4 bytes
    // Capstone can decode from a buffer, we'll give it up to 16 bytes to be safe
    const size_t max_bytes = std::min(bytes.size(), static_cast<size_t>(16));

    cs_insn* insn = nullptr;
    size_t count = cs_disasm(
        handle_,
        bytes.data(),
        max_bytes,
        address,
        1,  // Only decode 1 instruction
        &insn
    );

    last_error_ = cs_errno(handle_);

    if (count == 0 || insn == nullptr) {
        if (insn) {
            cs_free(insn, count);
        }
        return std::nullopt;
    }

    // We only requested 1 instruction
    const cs_insn& decoded = insn[0];

    InstructionDetail detail;
    detail.id = decoded.id;  // arm64_insn enum in Capstone 5.x
    detail.mnemonic = (decoded.mnemonic[0] != '\0') ? decoded.mnemonic : "";
    detail.op_str = (decoded.op_str[0] != '\0') ? decoded.op_str : "";
    detail.size = static_cast<uint16_t>(decoded.size);
    detail.address = decoded.address;

    // Copy raw bytes
    detail.bytes.assign(bytes.data(), bytes.data() + detail.size);

    // Parse ARM64-specific detail if available
    if (decoded.detail != nullptr) {
        const cs_arm64& arm64 = decoded.detail->arm64;

        detail.cc = arm64.cc;
        detail.update_flags = arm64.update_flags;
        detail.writeback = arm64.writeback;
        // Note: insn_id was removed in Capstone 5.x

        // Parse operands
        detail.operands.reserve(arm64.op_count);
        for (uint8_t i = 0; i < arm64.op_count; ++i) {
            const cs_arm64_op& op = arm64.operands[i];
            Arm64Operand parsed_op;
            parsed_op.type = static_cast<Arm64OperandType>(op.type);
            parsed_op.vas = op.vas;
            parsed_op.vector_index = op.vector_index;
            parsed_op.shift_type = op.shift.type;
            parsed_op.shift_value = op.shift.value;
            parsed_op.ext = op.ext;
            parsed_op.reg = static_cast<Arm64Register>(op.reg);
            parsed_op.imm = op.imm;
            parsed_op.mem.base = static_cast<Arm64Register>(op.mem.base);
            parsed_op.mem.index = static_cast<Arm64Register>(op.mem.index);
            parsed_op.mem.disp = op.mem.disp;
            parsed_op.fp = op.fp;
            parsed_op.sys = static_cast<Arm64SysReg>(op.sys);
            // Note: reg_sub was removed in Capstone 5.x

            detail.operands.push_back(parsed_op);
        }
    }

    cs_free(insn, count);
    return detail;
}

std::string CapstoneWrapper::get_last_error_message() const {
    if (last_error_ == CS_ERR_OK) {
        return "No error";
    }

    // cs_strerror is available in Capstone 4.0+
    const char* msg = cs_strerror(last_error_);
    return msg ? msg : "Unknown Capstone error";
}

} // namespace kirbyrecomp::translator