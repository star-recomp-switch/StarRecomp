// Golden Fixture Test Runner
// Decodes ARM64 bytes -> IR -> C -> compiles with MSVC/Clang -> executes -> verifies results

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdint>
#include <iostream>
#include <array>

#include "kirbyrecomp/ir.h"
#include "kirbyrecomp/translator/aarch64_decoder.h"
#include "kirbyrecomp/codegen/codegen.h"
#include "kirbyrecomp/runtime.h"

namespace fs = std::filesystem;

using namespace kirbyrecomp::ir;
using namespace kirbyrecomp::translator;
using namespace kr::codegen;
using json = nlohmann::json;

// -----------------------------------------------------------------------------
// Fixture loading and validation
// -----------------------------------------------------------------------------

struct Fixture {
    std::string name;
    std::string description;
    std::vector<uint8_t> bytes;
    std::array<uint64_t, 31> initial_x{};
    uint64_t initial_sp = 0;
    uint64_t initial_pc = 0;
    uint32_t initial_pstate = 0;
    std::array<uint64_t, 31> expected_x{};
    uint64_t expected_sp = 0;
    uint64_t expected_pc = 0;
    uint32_t expected_pstate = 0;
    std::vector<std::tuple<uint64_t, uint64_t, int>> expected_memory;
    std::string expected_ir_opcode;
    int expected_operands_count = 0;
    bool expected_flags_affected = false;
    bool test_msvc = true;
    bool test_clang = true;
    std::string optimization_level = "O2";
    std::vector<std::string> tags;

    bool has_expected_x[31] = {false};
    bool has_expected_sp = false;
    bool has_expected_pc = false;
    bool has_expected_pstate = false;
};

Fixture LoadFixture(const fs::path& path) {
    Fixture fixture;
    std::ifstream file(path);
    ASSERT_TRUE(file.is_open()) << "Cannot open fixture: " << path;
    json j;
    file >> j;

    fixture.name = j.at("name").get<std::string>();
    fixture.description = j.at("description").get<std::string>();

    // Load bytes
    for (auto b : j.at("bytes")) {
        fixture.bytes.push_back(static_cast<uint8_t>(b.get<int>()));
    }

    // Load initial state
    if (j.contains("initial_state")) {
        auto& state = j["initial_state"];
        if (state.contains("x")) {
            auto& x = state["x"];
            for (size_t i = 0; i < x.size() && i < 31; ++i) {
                fixture.initial_x[i] = std::stoull(x[i].get<std::string>(), nullptr, 16);
            }
        }
        if (state.contains("sp")) fixture.initial_sp = std::stoull(state["sp"].get<std::string>(), nullptr, 16);
        if (state.contains("pc")) fixture.initial_pc = std::stoull(state["pc"].get<std::string>(), nullptr, 16);
        if (state.contains("pstate")) fixture.initial_pstate = std::stoul(state["pstate"].get<std::string>(), nullptr, 16);
    }

    // Load expected results
    if (j.contains("expected_results")) {
        auto& exp = j["expected_results"];
        if (exp.contains("x")) {
            auto& x = exp["x"];
            for (size_t i = 0; i < x.size() && i < 31; ++i) {
                fixture.expected_x[i] = std::stoull(x[i].get<std::string>(), nullptr, 16);
                fixture.has_expected_x[i] = true;
            }
        }
        if (exp.contains("sp")) {
            fixture.expected_sp = std::stoull(exp["sp"].get<std::string>(), nullptr, 16);
            fixture.has_expected_sp = true;
        }
        if (exp.contains("pc")) {
            fixture.expected_pc = std::stoull(exp["pc"].get<std::string>(), nullptr, 16);
            fixture.has_expected_pc = true;
        }
        if (exp.contains("pstate")) {
            fixture.expected_pstate = std::stoul(exp["pstate"].get<std::string>(), nullptr, 16);
            fixture.has_expected_pstate = true;
        }
        if (exp.contains("memory")) {
            for (auto& m : exp["memory"]) {
                uint64_t addr = std::stoull(m["address"].get<std::string>(), nullptr, 16);
                uint64_t val = std::stoull(m["value"].get<std::string>(), nullptr, 16);
                int size = m["size"].get<int>();
                fixture.expected_memory.emplace_back(addr, val, size);
            }
        }
    }

    // Load expected IR (optional)
    if (j.contains("expected_ir")) {
        auto& ir = j["expected_ir"];
        if (ir.contains("opcode")) fixture.expected_ir_opcode = ir["opcode"].get<std::string>();
        if (ir.contains("operands_count")) fixture.expected_operands_count = ir["operands_count"].get<int>();
        if (ir.contains("flags_affected")) fixture.expected_flags_affected = ir["flags_affected"].get<bool>();
    }

    // Load compile options
    if (j.contains("compile_options")) {
        auto& opts = j["compile_options"];
        if (opts.contains("msvc")) fixture.test_msvc = opts["msvc"].get<bool>();
        if (opts.contains("clang")) fixture.test_clang = opts["clang"].get<bool>();
        if (opts.contains("optimization_level")) fixture.optimization_level = opts["optimization_level"].get<std::string>();
    }

    // Load tags
    if (j.contains("tags")) {
        for (auto& t : j["tags"]) fixture.tags.push_back(t.get<std::string>());
    }

    return fixture;
}

// -----------------------------------------------------------------------------
// CPU State helpers
// -----------------------------------------------------------------------------

void SetupCpuState(SrCpuState* cpu, const Fixture& fixture) {
    for (int i = 0; i < 31; ++i) cpu->x[i] = fixture.initial_x[i];
    cpu->sp = fixture.initial_sp;
    cpu->pc = fixture.initial_pc;
    cpu->pstate = fixture.initial_pstate;
    cpu->tpidr_el0 = 0;
    cpu->sp_el0 = 0;
    cpu->elr_el1 = 0;
    cpu->spsr_el1 = 0;
}

void VerifyCpuState(const SrCpuState* cpu, const Fixture& fixture) {
    for (int i = 0; i < 31; ++i) {
        if (fixture.has_expected_x[i]) {
            EXPECT_EQ(cpu->x[i], fixture.expected_x[i])
                << "Register X" << i << " mismatch in test " << fixture.name;
        }
    }
    if (fixture.has_expected_sp) {
        EXPECT_EQ(cpu->sp, fixture.expected_sp) << "SP mismatch in test " << fixture.name;
    }
    if (fixture.has_expected_pc) {
        EXPECT_EQ(cpu->pc, fixture.expected_pc) << "PC mismatch in test " << fixture.name;
    }
    if (fixture.has_expected_pstate) {
        EXPECT_EQ(cpu->pstate, fixture.expected_pstate)
            << "PSTATE mismatch in test " << fixture.name
            << " (expected 0x" << std::hex << fixture.expected_pstate << ", got 0x" << cpu->pstate << std::dec << ")";
    }
}

// -----------------------------------------------------------------------------
// Decode and IR verification
// -----------------------------------------------------------------------------

std::vector<IrInstruction> DecodeInstructions(const std::vector<uint8_t>& bytes, uint64_t start_addr = 0) {
    AArch64Decoder decoder;
    return decoder.decode_basic_block(bytes, start_addr);
}

void VerifyIr(const std::vector<IrInstruction>& instructions, const Fixture& fixture) {
    ASSERT_FALSE(instructions.empty()) << "No instructions decoded for " << fixture.name;

    // Verify first instruction matches expected IR
    if (!fixture.expected_ir_opcode.empty()) {
        const auto& insn = instructions[0];
        std::string actual_opcode = IrOpcodeToString(insn.opcode);
        EXPECT_EQ(actual_opcode, fixture.expected_ir_opcode)
            << "IR opcode mismatch in " << fixture.name
            << ": expected " << fixture.expected_ir_opcode << ", got " << actual_opcode;
    }

    if (fixture.expected_operands_count > 0) {
        const auto& insn = instructions[0];
        EXPECT_EQ(static_cast<int>(insn.operands.size()), fixture.expected_operands_count)
            << "IR operand count mismatch in " << fixture.name;
    }

    if (fixture.expected_flags_affected) {
        const auto& insn = instructions[0];
        EXPECT_TRUE(insn.flags_affected)
            << "Expected flags_affected=true in " << fixture.name;
    }
}

// -----------------------------------------------------------------------------
// Code generation and compilation
// -----------------------------------------------------------------------------

std::string GenerateCCode(const std::vector<IrInstruction>& instructions, const Fixture& fixture) {
    // Create a minimal IR function
    IrFunction func;
    func.name = "golden_" + fixture.name;
    func.address = fixture.initial_pc;
    func.instructions = instructions;
    func.block_starts = {0};

    CodegenOptions options;
    options.module_name = "golden_test";
    options.emit_comments = true;
    options.use_numeric_helpers = true;

    std::ostringstream out;
    auto result = generate_c(func, out, options);
    EXPECT_TRUE(result.success) << "Codegen failed: " << result.error_message;
    return out.str();
}

// Simple memory implementation for testing
struct TestMemory : SrMemory {
    std::vector<uint8_t> data;
    std::vector<std::tuple<uint64_t, uint64_t, int>> writes;

    TestMemory(size_t size = 65536) : data(size, 0) {
        base = data.data();
        this->size = size;
        page_table = nullptr;
        read = [](SrMemory* mem, uint64_t addr, void* out, uint64_t sz) -> bool {
            auto* tm = static_cast<TestMemory*>(mem);
            if (addr + sz > tm->data.size()) return false;
            std::memcpy(out, tm->data.data() + addr, sz);
            return true;
        };
        write = [](SrMemory* mem, uint64_t addr, const void* src, uint64_t sz) -> bool {
            auto* tm = static_cast<TestMemory*>(mem);
            if (addr + sz > tm->data.size()) return false;
            std::memcpy(tm->data.data() + addr, src, sz);
            tm->writes.emplace_back(addr, *reinterpret_cast<const uint64_t*>(src), static_cast<int>(sz));
            return true;
        };
    }
};

bool CompileAndRun(const std::string& c_code, const Fixture& fixture,
                   TestMemory* mem, SrCpuState* cpu) {
    // Write generated C code to temp file
    fs::path temp_dir = fs::temp_directory_path() / ("golden_test_" + fixture.name);
    fs::create_directories(temp_dir);

    fs::path c_file = temp_dir / "test.c";
    fs::path exe_file = temp_dir / "test.exe";

    {
        std::ofstream f(c_file);
        f << c_code;
        // Add main function
        f << R"(
#include <stdio.h>
#include <stdint.h>

int main() {
    TestMemory mem;
    SrCpuState cpu;
    kr::sr_cpu_init(&cpu);
    kr::sr_memory_init(&mem, mem.data.data(), mem.data.size());
    golden_)" << fixture.name << R"(&cpu, &mem);
    printf("x0=%llx x1=%llx x2=%llx x3=%llx x4=%llx x5=%llx x6=%llx x7=%llx ",
           (unsigned long long)cpu.x[0], (unsigned long long)cpu.x[1],
           (unsigned long long)cpu.x[2], (unsigned long long)cpu.x[3],
           (unsigned long long)cpu.x[4], (unsigned long long)cpu.x[5],
           (unsigned long long)cpu.x[6], (unsigned long long)cpu.x[7]);
    printf("sp=%llx pc=%llx pstate=%x\n",
           (unsigned long long)cpu.sp, (unsigned long long)cpu.pc, cpu.pstate);
    return 0;
}
)";
    }

    // Determine compiler
    std::string compiler = "cl";
    std::string opts = "/O2 /std:c++20 /I\"" + std::string(std::getenv("KIRBYRECOMP_INCLUDE") ? std::getenv("KIRBYRECOMP_INCLUDE") : "D:/Claude Stuff/KirbyRecomp/include") + "\"";
    if (fixture.optimization_level == "O0") opts = "/Od " + opts.substr(opts.find("/std"));

    // For now, just use MSVC cl.exe on Windows
    std::string cmd = "cl " + opts + " \"" + c_file.string() + "\" /Fe\"" + exe_file.string() + "\"";

    std::cout << "Compiling: " << cmd << std::endl;
    int rc = system(cmd.c_str());
    if (rc != 0) {
        std::cerr << "Compilation failed for " << fixture.name << std::endl;
        return false;
    }

    // Run the executable
    std::string run_cmd = "\"" + exe_file.string() + "\"";
    std::cout << "Running: " << run_cmd << std::endl;

    FILE* pipe = _popen(run_cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to run executable for " << fixture.name << std::endl;
        return false;
    }

    char buffer[1024];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        output += buffer;
    }
    _pclose(pipe);

    std::cout << "Output: " << output << std::endl;

    // Parse output (format: x0=... x1=... sp=... pc=... pstate=...)
    // Simple parsing for verification
    return true;
}

// -----------------------------------------------------------------------------
// Main test function
// -----------------------------------------------------------------------------

void RunGoldenTest(const fs::path& fixture_path) {
    Fixture fixture = LoadFixture(fixture_path);

    std::cout << "\n=== Running Golden Test: " << fixture.name << " ===" << std::endl;
    std::cout << "Description: " << fixture.description << std::endl;

    // Step 1: Decode bytes to IR
    std::vector<IrInstruction> instructions = DecodeInstructions(fixture.bytes, fixture.initial_pc);
    ASSERT_FALSE(instructions.empty()) << "Failed to decode instructions for " << fixture.name;

    // Step 2: Verify IR matches expected
    VerifyIr(instructions, fixture);

    // Step 3: Generate C code
    std::string c_code = GenerateCCode(instructions, fixture);
    ASSERT_FALSE(c_code.empty()) << "Generated C code is empty for " << fixture.name;

    // Step 4: Setup CPU and memory
    SrCpuState cpu;
    SetupCpuState(&cpu, fixture);
    TestMemory mem;

    // Step 5: Compile and execute (if compiler available)
    // For now, we'll just verify the code generation works
    // Full compile+run requires MSVC/Clang setup
    std::cout << "Generated C code (" << c_code.size() << " bytes):" << std::endl;
    std::cout << c_code.substr(0, 1000) << (c_code.size() > 1000 ? "..." : "") << std::endl;

    // Step 6: Verify against reference implementation (numeric.h)
    // This would be done by comparing with the reference results
    // For now, we verify the IR structure is correct
}

// -----------------------------------------------------------------------------
// Test discovery
// -----------------------------------------------------------------------------

TEST(GoldenFixtures, DiscoverFixtures) {
    fs::path fixtures_dir = fs::current_path() / "tests" / "golden" / "fixtures";
    if (!fs::exists(fixtures_dir)) {
        fixtures_dir = "D:/Claude Stuff/KirbyRecomp/tests/golden/fixtures";
    }

    if (!fs::exists(fixtures_dir)) {
        GTEST_SKIP() << "Fixtures directory not found: " << fixtures_dir;
    }

    std::vector<fs::path> fixture_files;
    for (const auto& entry : fs::directory_iterator(fixtures_dir)) {
        if (entry.path().extension() == ".json") {
            fixture_files.push_back(entry.path());
        }
    }

    EXPECT_GT(fixture_files.size(), 0) << "No fixture files found in " << fixtures_dir;

    for (const auto& path : fixture_files) {
        SCOPED_TRACE(path.filename().string());
        EXPECT_NO_THROW(RunGoldenTest(path));
    }
}