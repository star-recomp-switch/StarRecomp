# Golden Fixture Tests

This directory contains the golden fixture test infrastructure for KirbyRecomp.

## Overview

Golden fixture tests verify the complete compilation pipeline:
1. **Decode** ARM64 instruction bytes → IR
2. **Generate** C code from IR
3. **Compile** with MSVC/Clang
4. **Execute** and verify results against expected state

## Directory Structure

```
tests/golden/
├── CMakeLists.txt          # CMake integration
├── schema.json             # JSON schema for fixture files
├── test_runner.cpp         # Test runner (GoogleTest)
├── fixtures/               # Fixture files (30+ JSON files)
└── README.md               # This file
```

## Fixture Format

Each fixture is a JSON file with the following structure:

```json
{
  "name": "unique_test_name",
  "description": "Human-readable description",
  "bytes": [0x00, 0x00, 0x01, 0x8b],  // ARM64 instruction bytes (little-endian)
  "initial_state": {
    "x": ["0x0", "0x10", "0x20"],     // X0-X30 registers
    "sp": "0x8000",                   // Stack pointer
    "pc": "0x1000",                   // Program counter
    "pstate": "0x0"                   // NZCV flags
  },
  "expected_results": {
    "x": ["0x30", "0x10", "0x20"],    // Expected register values
    "pc": "0x1004",                   // Expected PC
    "pstate": "0x80000000"            // Expected flags (N=1)
  },
  "expected_ir": {
    "opcode": "add",                  // Expected IR opcode
    "operands_count": 3,              // Expected operand count
    "flags_affected": false           // Whether flags are affected
  },
  "tags": ["alu", "add"]              // Test categorization tags
}
```

## Coverage

The 30 fixture files cover:

### ALU Instructions
- **ADD**: `add_reg`, `add_imm`, `add_overflow`, `add_carry`, `add_negative_flag`
- **SUB**: `sub_reg`, `sub_borrow`
- **Logic**: `and_reg`, `and_flags`, `orr_reg`, `eor_reg`
- **Shifts**: `lsl_imm`, `lsl_shift_64`, `lsr_imm`, `lsr_shift_64`, `asr_imm`, `asr_negative`, `asr_shift_64`
- **Mul/Div**: `mul_reg`, `udiv_reg`, `udiv_by_zero`, `sdiv_reg`, `sdiv_by_zero`
- **Compare**: `cmp_reg`, `cmp_equal`

### Memory Instructions
- **LDR**: `ldr_imm`
- **STR**: `str_imm`

### Branch Instructions
- **B**: `b_uncond`
- **BL**: `bl_call`
- **CBZ**: `cbz_taken`, `cbz_not_taken`

### Move Instructions
- **MOV/MOVZ**: `mov_imm`
- **MOVK**: `movk_build`

### System Instructions
- **SVC**: `svc_call`

### Edge Cases Covered
- Shift amount ≥ 64 (masked to 0 per AArch64)
- Division by zero (returns 0 per AArch64)
- Overflow detection (V flag)
- Carry/borrow (C flag)
- Negative flag (N flag)
- Zero flag (Z flag)
- Sign extension in ASR
- 32-bit vs 64-bit operations

## Running Tests

### Via CMake/CTest

```bash
# Configure with tests enabled
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build

# Run all golden tests
ctest --output-on-failure -R golden

# Run a specific fixture
ctest --output-on-failure -R golden_add_reg
```

### Via GoogleTest directly

```bash
# Run all golden fixtures
./build/bin/kirbyrecomp_golden_tests --gtest_filter="GoldenFixtures.*"

# Run a specific fixture
./build/bin/kirbyrecomp_golden_tests --gtest_filter="GoldenFixtures.DiscoverFixtures/add_reg*"
```

## Adding New Fixtures

1. Create a new `.json` file in `fixtures/`
2. Follow the schema in `schema.json`
3. Add appropriate tags for categorization
4. Run tests to verify

## Reference Implementation

Expected results are verified against the `numeric.h` reference implementation in `include/kirbyrecomp/numeric.h`, which provides bit-exact, UB-free AArch64 numeric semantics.

## Tags

Tests are tagged for easy filtering:
- `alu` - Arithmetic/logic unit instructions
- `memory` - Load/store instructions
- `branch` - Branch instructions
- `move` - Move immediate instructions
- `system` - System instructions
- `shift` - Shift operations
- `division` - Division operations
- `compare` - Compare operations
- `flags` - Flag-setting instructions
- `edge-case` - Edge case tests
- `immediate` - Immediate operand tests
- `32bit` - 32-bit (W register) operations
- `signed` - Signed operations
- `carry` - Carry flag tests
- `overflow` - Overflow flag tests
- `zero-flag` - Zero flag tests
- `negative-flag` - Negative flag tests
- `shift-ge-64` - Shift ≥ 64 tests
- `divide-by-zero` - Division by zero tests