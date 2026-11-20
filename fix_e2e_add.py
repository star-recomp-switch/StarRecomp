with open('vendor/switchrecomp/tests/integration/test_e2e_add.cpp', 'r') as f:
    content = f.read()

old_text = '''    // Compile with g++ from PATH (link the full runtime + platform for symbol
    // availability).
    std::string cmd = "g++ -std=c++17 -I \"" + include_dir + "\" -I \"" +
                      runtime_dir + "\" \"" + out_c + "\" \"" +
                      runtime_dir + "/runtime.cpp\" \"" +
                      runtime_dir + "/memory_manager.cpp\" \"" +
                      runtime_dir + "/svc_table.cpp\" \"" +
                      runtime_dir + "/svc_thread.cpp\" \"" +
                      runtime_dir + "/hle_service.cpp\" \"" +
                      platform_dir + "/memory.cpp\" \"" +
                      platform_dir + "/thread.cpp\" -o \"" + out_exe + "\" 2>&1";
    int rc = std::system(cmd.c_str());'''

new_text = '''    // Compile with g++ from PATH (link the full runtime + platform for symbol
    // availability).
    // Set temp directory for g++ (mingw64 needs forward slashes in paths)
    _putenv_s("TMP", "C:/tmp/gcc_temp");
    _putenv_s("TEMP", "C:/tmp/gcc_temp");
    _putenv_s("TMPDIR", "C:/tmp/gcc_temp");
    std::string cmd = "C:/msys64/mingw64/bin/g++.exe -std=c++17 -Wl,--subsystem,console -I \"" + include_dir + "\" -I \"" +
                      runtime_dir + "\" \"" + out_c + "\" \"" +
                      runtime_dir + "/runtime.cpp\" \"" +
                      runtime_dir + "/memory_manager.cpp\" \"" +
                      runtime_dir + "/svc_table.cpp\" \"" +
                      runtime_dir + "/svc_thread.cpp\" \"" +
                      runtime_dir + "/hle_service.cpp\" \"" +
                      platform_dir + "/memory.cpp\" \"" +
                      platform_dir + "/thread.cpp\" -o \"" + out_exe + "\" 2>&1";
    int rc = std::system(cmd.c_str());'''

with open('vendor/switchrecomp/tests/integration/test_e2e_add.cpp', 'r') as f:
    content = f.read()

if old_text in content:
    content = content.replace(old_text, new_text)
    with open('vendor/switchrecomp/tests/integration/test_e2e_add.cpp', 'w') as f:
        f.write(content)
    print("Fixed test_e2e_add.cpp")
else:
    print("Old text not found!")
    # Find the pattern
    idx = content.find('std::string cmd = "g++ -std=c++17 -I')
    if idx >= 0:
        print("Found at index:", idx)
        print(content[idx:idx+500])

print("Done")