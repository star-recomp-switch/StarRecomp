#pragma once

#include "kirbyrecomp/numeric.h"
#include "kirbyrecomp/types.h"
#include "kirbyrecomp/ir.h"

#include <string>
#include <vector>
#include <ostream>

namespace kr::codegen {

// Forward declaration of runtime types used in generated code
struct SrCpuState;
struct SrMemory;

// Code generation options
struct CodegenOptions {
    std::string module_name = "guest";
    bool emit_comments = true;
    bool use_numeric_helpers = true;  // Use sr::numeric helpers for bit-exact semantics
};

// Register mapping for generated code
enum class RegisterKind : uint8_t {
    GPR,        // X0-X30 general purpose registers
    SP,         // Stack pointer
    PC,         // Program counter
    ZR          // Zero register (always reads as 0)
};

// Result of code generation
struct CodegenResult {
    std::string source_code;
    std::vector<std::string> function_names;
    bool success = true;
    std::string error_message;
};

// Generate C code for a single IR function
// Emits a function with signature: void func_name(SrCpuState* cpu, SrMemory* mem)
CodegenResult generate_c(const kirbyrecomp::ir::IrFunction& func,
                         std::ostream& output,
                         const CodegenOptions& options = {});

// Generate C code for an entire IR module
CodegenResult generate_c(const kirbyrecomp::ir::IrModule& module,
                         std::ostream& output,
                         const CodegenOptions& options = {});

// Generate C code for an IR function and return as string
CodegenResult generate_c(const kirbyrecomp::ir::IrFunction& func,
                         const CodegenOptions& options = {});

// Generate C code for an IR module and return as string
CodegenResult generate_c(const kirbyrecomp::ir::IrModule& module,
                         const CodegenOptions& options = {});

} // namespace kr::codegen