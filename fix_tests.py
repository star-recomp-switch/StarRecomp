import re
import os

test_dir = 'vendor/switchrecomp/tests/integration'
test_files = [
    'test_e2e_add.cpp',
    'test_e2e_call.cpp',
    'test_e2e_fcmp.cpp',
    'test_e2e_float.cpp',
    'test_e2e_fp64.cpp',
    'test_e2e_fp_misc.cpp',
    'test_e2e_fp_round.cpp',
    'test_e2e_image.cpp',
    'test_e2e_intcmp.cpp',
    'test_e2e_simd.cpp',
    'test_e2e_threads.cpp',
    'test_numeric_codegen.cpp',
]

def fix_test_file(filename):
    filepath = os.path.join('vendor/switchrecomp/tests/integration', filename)
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    original_content = content

    # Fix 1: Replace g++ --version check with explicit mingw64 g++
    content = re.sub(
        r'std::string cmd = std::string\("g\+\+ --version > "\) \+ null_dev \+ " 2>&1";',
        r'std::string cmd = "C:/msys64/mingw64/bin/g++.exe --version > " + null_dev + " 2>&1";',
        content
    )

    # Fix 2: Replace simple g++ compile command (test_e2e_add.cpp style)
    content = re.sub(
        r'std::string cmd = "g\+\+ -std=c\+\+17 -I \\\"" \+ include_dir \+ "\\"" -I \\\"" \+',
        r'_putenv_s("TMP", "C:/tmp/gcc_temp");\n    _putenv_s("TEMP", "C:/tmp/gcc_temp");\n    _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n    std::string cmd = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console -I \"" +',
        content
    )

    # Fix the end of the simple g++ command
    content = re.sub(
        r'runtime_dir \+ "\\"" \+ out_c \+ "\\"" \+ runtime_dir \+ "/runtime\.cpp" \+ runtime_dir \+ "/memory_manager\.cpp" \+ runtime_dir \+ "/svc_table\.cpp" \+ runtime_dir \+ "/svc_thread\.cpp" \+ runtime_dir \+ "/hle_service\.cpp" \+ platform_dir \+ "/memory\.cpp" \+ platform_dir \+ "/thread\.cpp\" -o \"" \+ out_exe \+ "\\"" 2>&1";',
        r'runtime_dir + "\" \"" + out_c + "\" \"" + runtime_dir + "/runtime.cpp\" \"" + runtime_dir + "/memory_manager.cpp\" \"" + runtime_dir + "/svc_table.cpp\" \"" + runtime_dir + "/svc_thread.cpp\" \"" + runtime_dir + "/hle_service.cpp\" \"" + platform_dir + "/memory.cpp\" \"" + platform_dir + "/thread.cpp\" -o \"" + out_exe + "\" 2>&1\";",
        content
    )

    # Fix compile lambda pattern - first add _putenv_s calls
    content = re.sub(
        r'auto compile = \[&\]\(const std::string& extra\) \{',
        r'auto compile = [&](const std::string& extra) {\n        // Set temp directory for g++ (mingw64 needs forward slashes in paths)\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");',
        content
    )

    # Fix g++ command in compile lambda
    content = re.sub(
        r'std::string c = "g\+\+ -std=c\+\+17 " \+ extra \+ " -I \\\"" \+ include_dir \+ "\\"" -I \\\"" \+',
        r'std::string c =\n            "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \"" +',
        content
    )

    # Fix the runtime_dir concatenation in compile lambda
    content = re.sub(
        r'runtime_dir \+ "\\"" \+ sources \+ " -o \\\"" \+ out_exe \+ "\\"" 2>&1";',
        r'runtime_dir + "\" " + sources + " -o \"" + out_exe + "\" 2>&1";',
        content
    )

    # Fix: Add _putenv_s calls at the start of compile lambda (first occurrence)
    content = re.sub(
        r'(auto compile = \[&\]\(const std::string& extra\) \{\n        // Set temp directory for g\+\+ \(mingw64 needs forward slashes in paths\)\n        _putenv_s\("TMP", "C:/tmp/gcc_temp"\);\n        _putenv_s\("TEMP", "C:/tmp/gcc_temp"\);\n        _putenv_s\("TMPDIR", "C:/tmp/gcc_temp"\);\n        std::string c =)\n            "C:/msys64/mingw64/bin/g\+\+\.exe -std=c\+\+17 -Wl,--subsystem,console " \+ extra \+ " -I \\\"" \+ include_dir \+ "\\"" -I "\\\"" \+',
        r'\1\n            runtime_dir + "\\"" -I "\\\"" +',
        content
    )

    # Fix 2nd occurrence of compile lambda (second occurrence in file)
    content = re.sub(
        r'(auto compile = \[&\]\(const std::string& extra\) \{\n        // Set temp directory for g\+\+ \(mingw64 needs forward slashes in paths\)\n        _putenv_s\("TMP", "C:/tmp/gcc_temp"\);\n        _putenv_s\("TEMP", "C:/tmp/gcc_temp"\);\n        _putenv_s\("TMPDIR", "C:/tmp/gcc_temp"\);\n        std::string c =)\n            "C:/msys64/mingw64/bin/g\+\+\.exe -std=c\+\+17 -Wl,--subsystem,console " \+ extra \+ " -I \\\"" \+ include_dir \+ "\\"" -I "\\\"" \+',
        r'\1\n            runtime_dir + "\\"" -I "\\\"" +',
        content
    )

    # Fix the g++ --version check
    content = re.sub(
        r'std::string cmd = std::string\("g\+\+ --version > "\) \+ null_dev \+ " 2>&1";',
        r'std::string cmd = "C:/msys64/mingw64/bin/g++.exe --version > " + null_dev + " 2>&1";',
        content
    )

    # Fix 2nd occurrence of compile lambda g++ command (second occurrence in file)
    content = re.sub(
        r'(auto compile = \[&\]\(const std::string& extra\) \{\n        // Set temp directory for g\+\+ \(mingw64 needs forward slashes in paths\)\n        _putenv_s\("TMP", "C:/tmp/gcc_temp"\);\n        _putenv_s\("TEMP", "C:/tmp/gcc_temp"\);\n        _putenv_s\("TMPDIR", "C:/tmp/gcc_temp"\);\n        std::string c =)\n            "C:/msys64/mingw64/bin/g\+\+\.exe -std=c\+\+17 -Wl,--subsystem,console " \+ extra \+ " -I \\\"" \+ include_dir \+ "\\"" -I "\\\"" \+',
        r'\1\n            runtime_dir + "\\"" -I "\\\"" +',
        content
    )

    # Fix simple g++ command pattern (for tests like test_e2e_fcmp.cpp)
    content = re.sub(
        r'std::string cmd = "g\+\+ -std=c\+\+17 -I \\\"" \+ include_dir \+ "\\"" -I \\\"" \+',
        r'_putenv_s("TMP", "C:/tmp/gcc_temp");\n    _putenv_s("TEMP", "C:/tmp/gcc_temp");\n    _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n    std::string cmd = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console -I \"" +',
        content
    )

    # Fix the runtime_dir concatenation in simple command
    content = re.sub(
        r'runtime_dir \+ "\\"" \+ out_c \+ "\\"" \+ runtime_dir \+ "/runtime\.cpp" \+ runtime_dir \+ "/memory_manager\.cpp" \+ runtime_dir \+ "/svc_table\.cpp" \+ runtime_dir \+ "/svc_thread\.cpp" \+ runtime_dir \+ "/hle_service\.cpp" \+ platform_dir \+ "/memory\.cpp" \+ platform_dir \+ "/thread\.cpp" \+ "-o \"" \+ out_exe \+ "\\"" 2>&1";',
        r'runtime_dir + "\" \"" + out_c + "\" \"" + runtime_dir + "/runtime.cpp\"" + runtime_dir + "/memory_manager.cpp\"" + runtime_dir + "/svc_table.cpp\"" + runtime_dir + "/svc_thread.cpp\"" + runtime_dir + "/hle_service.cpp\"" + platform_dir + "/memory.cpp\"" + platform_dir + "/thread.cpp\" -o \"" + out_exe + "\" 2>&1\";",
        content
    )

    # Fix compile lambda in test_e2e_fcmp.cpp (special case)
    content = re.sub(
        r'(auto compile = \[&\]\(const std::string& extra\) \{\n        std::string c =)\n            "g\+\+ -std=c\+\+17 " \+ extra \+ " -I \\\"" \+ include_dir \+ "\\"" -I \\\"" \+',
        r'\1\n            runtime_dir + "\\"" -I "\\\"" +',
        content
    )

    # Fix runtime_dir concatenation in fcmp
    content = re.sub(
        r'runtime_dir \+ "\\"" \+ out_c \+ "\\"" \+ runtime_dir \+ "/runtime\.cpp\\"" \+ runtime_dir \+ "/memory_manager\.cpp" \+ runtime_dir \+ "/svc_table\.cpp" \+ runtime_dir \+ "/svc_thread\.cpp" \+ runtime_dir \+ "/hle_service\.cpp" \+ platform_dir \+ "/memory\.cpp" \+ platform_dir \+ "/thread\.cpp" \+ "-o \"" \+ out_exe \+ "\\"" 2>&1";',
        r'runtime_dir + "\" \"" + out_c + "\" \"" + runtime_dir + "/runtime.cpp\"" + runtime_dir + "/memory_manager.cpp\"" + runtime_dir + "/svc_table.cpp\"" + runtime_dir + "/svc_thread.cpp\"" + runtime_dir + "/hle_service.cpp\"" + platform_dir + "/memory.cpp\"" + platform_dir + "/thread.cpp\" -o \"" + out_exe + "\" 2>&1\";",
        content
    )

    # Fix g++ command in test_e2e_fcmp.cpp
    content = re.sub(
        r'std::string cmd = "g\+\+ -std=c\+\+17 -I \\\"" \+ include_dir \+ "\\"" -I \\\"" \+',
        r'_putenv_s("TMP", "C:/tmp/gcc_temp");\n    _putenv_s("TEMP", "C:/tmp/gcc_temp");\n    _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n    std::string cmd = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console -I \"" +',
        content
    )

    # Fix runtime_dir concatenation in fcmp
    content = re.sub(
        r'runtime_dir \+ "\\"" \+ out_c \+ "\\"" \+ runtime_dir \+ "/runtime\.cpp\\"" \+ runtime_dir \+ "/memory_manager\.cpp" \+ runtime_dir \+ "/svc_table\.cpp" \+ runtime_dir \+ "/svc_thread\.cpp" \+ runtime_dir \+ "/hle_service\.cpp" \+ platform_dir \+ "/memory\.cpp" \+ platform_dir \+ "/thread\.cpp" \+ "-o \"" \+ out_exe \+ "\\"" 2>&1";',
        r'runtime_dir + "\" \"" + out_c + "\" \"" + runtime_dir + "/runtime.cpp\"" + runtime_dir + "/memory_manager.cpp\"" + runtime_dir + "/svc_table.cpp\"" + runtime_dir + "/svc_thread.cpp\"" + runtime_dir + "/hle_service.cpp\"" + platform_dir + "/memory.cpp\"" + platform_dir + "/thread.cpp\" -o \"" + out_exe + "\" 2>&1\";",
        content
    )

    # Fix the second occurrence of compile lambda in test_e2e_fcmp.cpp (if any)
    content = re.sub(
        r'(auto compile = \[&\]\(const std::string& extra\) \{\n        std::string c =)\n            "g\+\+ -std=c\+\+17 " \+ extra \+ " -I \\\"" \+ include_dir \+ "\\"" -I \\\"" \+',
        r'\1\n            runtime_dir + "\\"" -I "\\\"" +',
        content
    )

    # Fix runtime_dir concatenation in fcmp (second occurrence)
    content = re.sub(
        r'runtime_dir \+ "\\"" \+ out_c \+ "\\"" \+ runtime_dir \+ "/runtime\.cpp\\"" \+ runtime_dir \+ "/memory_manager\.cpp" \+ runtime_dir \+ "/svc_table\.cpp" \+ runtime_dir \+ "/svc_thread\.cpp" \+ runtime_dir \+ "/hle_service\.cpp" \+ platform_dir \+ "/memory\.cpp" \+ platform_dir \+ "/thread\.cpp" \+ "-o \"" \+ out_exe \+ "\\"" 2>&1";',
        r'runtime_dir + "\" \"" + out_c + "\" \"" + runtime_dir + "/runtime.cpp\"" + runtime_dir + "/memory_manager.cpp\"" + runtime_dir + "/svc_table.cpp\"" + runtime_dir + "/svc_thread.cpp\"" + runtime_dir + "/hle_service.cpp\"" + platform_dir + "/memory.cpp\"" + platform_dir + "/thread.cpp\" -o \"" + out_exe + "\" 2>&1\";",
        content
    )

    # Fix g++ command in test_e2e_fcmp.cpp
    content = re.sub(
        r'std::string cmd = "g\+\+ -std=c\+\+17 -I \\\"" \+ include_dir \+ "\\"" -I \\\"" \+',
        r'_putenv_s("TMP", "C:/tmp/gcc_temp");\n    _putenv_s("TEMP", "C:/tmp/gcc_temp");\n    _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n    std::string cmd = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console -I \"" +',
        content
    )

    # Fix runtime_dir concatenation in fcmp
    content = re.sub(
        r'runtime_dir \+ "\\"" \+ out_c \+ "\\"" \+ runtime_dir \+ "/runtime\.cpp\\"" \+ runtime_dir \+ "/memory_manager\.cpp" \+ runtime_dir \+ "/svc_table\.cpp" \+ runtime_dir \+ "/svc_thread\.cpp" \+ runtime_dir \+ "/hle_service\.cpp" \+ platform_dir \+ "/memory\.cpp" \+ platform_dir \+ "/thread\.cpp" \+ "-o \"" \+ out_exe \+ "\\"" 2>&1";',
        r'runtime_dir + "\" \"" + out_c + "\" \"" + runtime_dir + "/runtime.cpp\"" + runtime_dir + "/memory_manager.cpp\"" + runtime_dir + "/svc_table.cpp\"" + runtime_dir + "/svc_thread.cpp\"" + runtime_dir + "/hle_service.cpp\"" + platform_dir + "/memory.cpp\"" + platform_dir + "/thread.cpp\" -o \"" + out_exe + "\" 2>&1\";",
        content
    )

    if content != original_content:
        with open(filepath, 'w') as f:
            f.write(content)
        return True
    return False

def main():
    test_dir = 'vendor/switchrecomp/tests/integration'
    test_files = [
        'test_e2e_add.cpp',
        'test_e2e_call.cpp',
        'test_e2e_fcmp.cpp',
        'test_e2e_float.cpp',
        'test_e2e_fp64.cpp',
        'test_e2e_fp_misc.cpp',
        'test_e2e_fp_round.cpp',
        'test_e2e_image.cpp',
        'test_e2e_intcmp.cpp',
        'test_e2e_simd.cpp',
        'test_e2e_threads.cpp',
        'test_numeric_codegen.cpp',
    ]

    fixed_count = 0
    for f in test_files:
        if fix_test_file(f):
            fixed_count += 1

    print(f"Fixed {fixed_count} out of {len(test_files)} test files")

if __name__ == '__main__':
    main()