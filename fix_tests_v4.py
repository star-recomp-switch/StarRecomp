import os
import re

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
    content = content.replace(
        'std::string cmd = std::string("g++ --version > ") + null_dev + " 2>&1";',
        'std::string cmd = "C:/msys64/mingw64/bin/g++.exe --version > " + null_dev + " 2>&1";'
    )

    # Fix 2: Replace simple g++ compile command (test_e2e_add.cpp style)
    content = content.replace(
        'std::string cmd = "g++ -std=c++17 -I \\"" + include_dir + "\\"" -I \\"" +',
        '_putenv_s("TMP", "C:/tmp/gcc_temp");\n    _putenv_s("TEMP", "C:/tmp/gcc_temp");\n    _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n    std::string cmd = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console -I \\"" +'
    )

    # Fix the end of the simple g++ command
    content = content.replace(
        'runtime_dir + "\\"" + out_c + "\\"" + runtime_dir + "/runtime.cpp\\"" + runtime_dir + "/memory_manager.cpp\\"" + runtime_dir + "/svc_table.cpp\\"" + runtime_dir + "/svc_thread.cpp\\"" + runtime_dir + "/hle_service.cpp\\"" + platform_dir + "/memory.cpp\\"" + platform_dir + "/thread.cpp\\"" -o \\"" + out_exe + "\\"" 2>&1";',
        'runtime_dir + "\\" \"" + out_c + "\\" \"" + runtime_dir + "/runtime.cpp\\"" + runtime_dir + "/memory_manager.cpp\\"" + runtime_dir + "/svc_table.cpp\\"" + runtime_dir + "/svc_thread.cpp\\"" + runtime_dir + "/hle_service.cpp\\"" + platform_dir + "/memory.cpp\\"" + platform_dir + "/thread.cpp\\" -o \\"" + out_exe + "\\"" 2>&1\";'
    )

    # Fix 3: Replace compile lambda pattern - first add _putenv_s calls
    content = content.replace(
        'auto compile = [&](const std::string& extra) {',
        'auto compile = [&](const std::string& extra) {\n        // Set temp directory for g++ (mingw64 needs forward slashes in paths)\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");'
    )

    # Fix 3b: Replace g++ command in compile lambda
    content = content.replace(
        'std::string c = "g++ -std=c++17 " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" +'
    )

    # Fix 4: Fix the runtime_dir concatenation in compile lambda
    content = content.replace(
        'runtime_dir + "\\"" + sources + " -o \\\"" + out_exe + "\\"" 2>&1";',
        'runtime_dir + "\\" " + sources + \' -o "\\" + out_exe + \'" 2>&1";'
    )

    # Fix: Add _putenv_s calls at the start of compile lambda (first occurrence)
    content = content.replace(
        'auto compile = [&](const std::string& extra) {\n        // Set temp directory for g++ (mingw64 needs forward slashes in paths)\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'auto compile = [&](const std::string& extra) {\n        // Set temp directory for g++ (mingw64 needs forward slashes in paths)\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + \'\\" -I \\"" + runtime_dir + \'" " +'
    )

    # Fix 2nd occurrence of compile lambda
    content = content.replace(
        'auto compile = [&](const std::string& extra) {\n        // Set temp directory for g++ (mingw64 needs forward slashes in paths)\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'auto compile = [&](const std::string& extra) {\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + \'\\" -I \\"" + runtime_dir + \'" " +'
    )

    # Fix simple g++ command pattern
    content = content.replace(
        'std::string cmd = "g++ -std=c++17 -I \\" + include_dir + "\\"" -I \\"" +',
        '_putenv_s("TMP", "C:/tmp/gcc_temp");\n    _putenv_s("TEMP", "C:/tmp/gcc_temp");\n    _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n    std::string cmd = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console -I \\"" +'
    )

    # Fix the end of the simple g++ command
    content = content.replace(
        'runtime_dir + "\\"" + out_c + "\\"" + runtime_dir + "/runtime.cpp\\"" + runtime_dir + "/memory_manager.cpp\\"" + runtime_dir + "/svc_table.cpp\\"" + runtime_dir + "/svc_thread.cpp\\"" + runtime_dir + "/hle_service.cpp\\"" + platform_dir + "/memory.cpp\\"" + platform_dir + "/thread.cpp\\" -o \\" + out_exe + "\\" 2>&1";',
        'runtime_dir + \'" \' + out_c + \'" \'" + runtime_dir + "/runtime.cpp\\"" + runtime_dir + "/memory_manager.cpp\\" + runtime_dir + "/svc_table.cpp\\" + runtime_dir + "/svc_thread.cpp\\" + runtime_dir + "/hle_service.cpp\\" + platform_dir + "/memory.cpp\\" + platform_dir + "/thread.cpp\\" -o \\" + out_exe + "\\" 2>&1";'
    )

    # Fix 2nd occurrence of compile lambda g++ command
    content = content.replace(
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + \'\\" -I \\"" + runtime_dir + \'" " +'
    )

    # Fix runtime_dir concatenation in second compile lambda
    content = content.replace(
        'runtime_dir + "\\"" + sources + " -o \\\"" + out_exe + "\\"" 2>&1";',
        'runtime_dir + \'" " + sources + \' -o "\\" + out_exe + \'" 2>&1";'
    )

    # Fix simple g++ command pattern
    content = content.replace(
        'std::string cmd = "g++ -std=c++17 -I \\" + include_dir + "\\"" -I \\"" +',
        '_putenv_s("TMP", "C:/tmp/gcc_temp");\n    _putenv_s("TEMP", "C:/tmp/gcc_temp");\n    _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n    std::string cmd = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console -I \\"" +'
    )

    # Fix end of simple g++ command
    content = content.replace(
        'runtime_dir + "\\"" + out_c + "\\"" + runtime_dir + "/runtime.cpp\\"" + runtime_dir + "/memory_manager.cpp\\"" + runtime_dir + "/svc_table.cpp\\"" + runtime_dir + "/svc_thread.cpp\\"" + runtime_dir + "/hle_service.cpp\\"" + platform_dir + "/memory.cpp\\"" + platform_dir + "/thread.cpp\\" -o \\" + out_exe + "\\"" 2>&1";',
        'runtime_dir + \'" \' + out_c + \'" \'" + runtime_dir + "/runtime.cpp\\"" + runtime_dir + "/memory_manager.cpp\\" + runtime_dir + "/svc_table.cpp\\" + runtime_dir + "/svc_thread.cpp\\" + runtime_dir + "/hle_service.cpp\\" + platform_dir + "/memory.cpp\\" + platform_dir + "/thread.cpp\\" -o \\" + out_exe + "\\" 2>&1";'
    )

    # Fix compile lambda first occurrence
    content = content.replace(
        'auto compile = [&](const std::string& extra) {\n        std::string c =',
        'auto compile = [&](const std::string& extra) {\n        // Set temp directory for g++ (mingw64 needs forward slashes in paths)\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c ='
    )

    content = content.replace(
        'std::string c = "g++ -std=c++17 " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" +'
    )

    content = content.replace(
        'runtime_dir + "\\"" + sources + " -o \\\"" + out_exe + "\\"" 2>&1";',
        'runtime_dir + "\\" " + sources + \' -o "\\" + out_exe + \'" 2>&1";'
    )

    # Fix second compile lambda
    content = content.replace(
        'auto compile = [&](const std::string& extra) {\n        // Set temp directory for g++ (mingw64 needs forward slashes in paths)\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'auto compile = [&](const std::string& extra) {\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + \'\\" -I \\"" + runtime_dir + \'" " +'
    )

    content = content.replace(
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + \'\\" -I \\"" + runtime_dir + \'" " +'
    )

    content = content.replace(
        'runtime_dir + "\\"" + sources + " -o \\\"" + out_exe + "\\"" 2>&1";',
        'runtime_dir + \'" " + sources + \' -o "\\" + out_exe + \'" 2>&1";'
    )

    # Fix simple g++ command
    content = content.replace(
        'std::string cmd = "g++ -std=c++17 -I \\" + include_dir + "\\"" -I \\"" +',
        '_putenv_s("TMP", "C:/tmp/gcc_temp");\n    _putenv_s("TEMP", "C:/tmp/gcc_temp");\n    _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n    std::string cmd = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console -I \\"" +'
    )

    # Fix end of simple g++ command
    content = content.replace(
        'runtime_dir + "\\"" + out_c + "\\"" + runtime_dir + "/runtime.cpp\\"" + runtime_dir + "/memory_manager.cpp\\"" + runtime_dir + "/svc_table.cpp\\"" + runtime_dir + "/svc_thread.cpp\\"" + runtime_dir + "/hle_service.cpp\\"" + platform_dir + "/memory.cpp\\"" + platform_dir + "/thread.cpp\\" -o \\" + out_exe + "\\" 2>&1";',
        'runtime_dir + \'" \' + out_c + \'" \'" + runtime_dir + "/runtime.cpp\\"" + runtime_dir + "/memory_manager.cpp\\" + runtime_dir + "/svc_table.cpp\\" + runtime_dir + "/svc_thread.cpp\\" + runtime_dir + "/hle_service.cpp\\" + platform_dir + "/memory.cpp\\" + platform_dir + "/thread.cpp\\" -o \\" + out_exe + "\\" 2>&1";'
    )

    # Fix compile lambda first occurrence
    content = content.replace(
        'auto compile = [&](const std::string& extra) {\n        std::string c =',
        'auto compile = [&](const std::string& extra) {\n        // Set temp directory for g++ (mingw64 needs forward slashes in paths)\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c ='
    )

    content = content.replace(
        'std::string c = "g++ -std=c++17 " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + \'\\" -I \\"" +'
    )

    content = content.replace(
        'runtime_dir + "\\"" + sources + " -o \\\"" + out_exe + "\\"" 2>&1";',
        'runtime_dir + "\\" " + sources + \' -o "\\" + out_exe + \'" 2>&1";'
    )

    # Fix second compile lambda
    content = content.replace(
        'auto compile = [&](const std::string& extra) {\n        // Set temp directory for g++ (mingw64 needs forward slashes in paths)\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'auto compile = [&](const std::string& extra) {\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + \'\\" -I \\"" + runtime_dir + \'\\" " +'
    )

    content = content.replace(
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + \'\\" -I \\"" + runtime_dir + \'\\" " +'
    )

    content = content.replace(
        'runtime_dir + "\\"" + sources + " -o \\\"" + out_exe + "\\"" 2>&1";',
        'runtime_dir + \'" " + sources + \' -o "\\" + out_exe + \'" 2>&1";'
    )

    # Fix simple g++ command
    content = content.replace(
        'std::string cmd = "g++ -std=c++17 -I \\" + include_dir + "\\" -I \\" +',
        '_putenv_s("TMP", "C:/tmp/gcc_temp");\n    _putenv_s("TEMP", "C:/tmp/gcc_temp");\n    _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n    std::string cmd = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console -I \\" +'
    )

    # Fix end of simple g++ command
    content = content.replace(
        'runtime_dir + "\\"" + out_c + "\\" + runtime_dir + "/runtime.cpp\\" + runtime_dir + "/memory_manager.cpp\\" + runtime_dir + "/svc_table.cpp\\" + runtime_dir + "/svc_thread.cpp\\" + runtime_dir + "/hle_service.cpp\\" + platform_dir + "/memory.cpp\\" + platform_dir + "/thread.cpp" -o "\\" + out_exe + "\\" 2>&1";',
        'runtime_dir + \'" \' + out_c + \'" \'" + runtime_dir + "/runtime.cpp\\" + runtime_dir + "/memory_manager.cpp\\" + runtime_dir + "/svc_table.cpp\\" + runtime_dir + "/svc_thread.cpp" + runtime_dir + "/hle_service.cpp\\" + platform_dir + "/memory.cpp" + platform_dir + "/thread.cpp" -o \\" + out_exe + "\\" 2>&1";'
    )

    # Fix compile lambda first occurrence
    content = content.replace(
        'auto compile = [&](const std::string& extra) {\n        std::string c =',
        'auto compile = [&](const std::string& extra) {\n        // Set temp directory for g++ (mingw64 needs forward slashes in paths)\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c ='
    )

    content = content.replace(
        'std::string c = "g++ -std=c++17 " + extra + " -I \\" + include_dir + "\\" -I \\"" +',
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\" + include_dir + \'\\" -I \\"" +'
    )

    content = content.replace(
        'runtime_dir + "\\"" + sources + " -o \\\"" + out_exe + "\\"" 2>&1";',
        'runtime_dir + "\\" " + sources + \' -o "\\" + out_exe + \'" 2>&1";'
    )

    # Fix second compile lambda
    content = content.replace(
        'auto compile = [&](const std::string& extra) {\n        // Set temp directory for g++ (mingw64 needs forward slashes in paths)\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'auto compile = [&](const std::string& extra) {\n        _putenv_s("TMP", "C:/tmp/gcc_temp");\n        _putenv_s("TEMP", "C:/tmp/gcc_temp");\n        _putenv_s("TMPDIR", "C:/tmp/gcc_temp");\n        std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + \'\\" -I \\"" + runtime_dir + \'" " +'
    )

    content = content.replace(
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + "\\"" -I \\"" +',
        'std::string c = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console " + extra + " -I \\"" + include_dir + \'\\" -I \\"" + runtime_dir + \'" " +'
    )

    content = content.replace(
        'runtime_dir + "\\"" + sources + " -o \\\"" + out_exe + "\\"" 2>&1";',
        'runtime_dir + \'" " + sources + \' -o "\\" + out_exe + \'" 2>&1";'
    )

    if content != original_content:
        with open(filepath, 'w', encoding='utf-8') as f:
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