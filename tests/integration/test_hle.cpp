// HLE Service Manager Tests

#include <gtest/gtest.h>
#include <kirbyrecomp/hle/service.h>
#include <kirbyrecomp/runtime.h>
#include <memory>
#include <thread>
#include <chrono>

using namespace kr::hle;
using namespace kr;

TEST(HleServiceManager, CreateAndGetServices) {
    ServiceManager& sm = ServiceManager::instance();

    // Create built-in services
    Result result = ServiceFactory::create_builtin_services(sm);
    EXPECT_EQ(result, Result::Success);

    // Test getting services
    IService* sm_service = sm.get_service("sm:");
    EXPECT_NE(sm_service, nullptr);
    EXPECT_STREQ(sm_service->get_name(), "sm:");

    IService* fs_service = sm.get_service("fs:");
    EXPECT_NE(fs_service, nullptr);
    EXPECT_STREQ(fs_service->get_name(), "fs:");

    IService* am_service = sm.get_service("am:");
    EXPECT_NE(am_service, nullptr);
    EXPECT_STREQ(am_service->get_name(), "am:");

    IService* hid_service = sm.get_service("hid:");
    EXPECT_NE(hid_service, nullptr);
    EXPECT_STREQ(hid_service->get_name(), "hid:");

    IService* time_service = sm.get_service("time:");
    EXPECT_NE(time_service, nullptr);
    EXPECT_STREQ(time_service->get_name(), "time:");

    IService* set_service = sm.get_service("set:");
    EXPECT_NE(set_service, nullptr);
    EXPECT_STREQ(set_service->get_name(), "set:");

    IService* audren_service = sm.get_service("audren:");
    EXPECT_NE(audren_service, nullptr);
    EXPECT_STREQ(audren_service->get_name(), "audren:");

    IService* audout_service = sm.get_service("audout:");
    EXPECT_NE(audout_service, nullptr);
    EXPECT_STREQ(audout_service->get_name(), "audout:");

    IService* hwopus_service = sm.get_service("hwopus:");
    EXPECT_NE(hwopus_service, nullptr);
    EXPECT_STREQ(hwopus_service->get_name(), "hwopus:");

    IService* nvn_service = sm.get_service("nvn:");
    EXPECT_NE(nvn_service, nullptr);
    EXPECT_STREQ(nvn_service->get_name(), "nvn:");

    // Test getting non-existent service
    IService* missing = sm.get_service("missing:");
    EXPECT_EQ(missing, nullptr);
}

TEST(HleServiceManager, SessionCreation) {
    ServiceManager& sm = ServiceManager::instance();

    // Create services
    ServiceFactory::create_builtin_services(sm);

    // Create session for sm:
    Session* session = sm.create_session("sm:");
    EXPECT_NE(session, nullptr);
    EXPECT_STREQ(session->service->get_name(), "sm:");

    // Create session for fs:
    Session* fs_session = sm.create_session("fs:");
    EXPECT_NE(fs_session, nullptr);
    EXPECT_STREQ(fs_session->service->get_name(), "fs:");

    // Test invalid service
    Session* invalid = sm.create_session("invalid:");
    EXPECT_EQ(invalid, nullptr);

    // Close sessions
    sm.close_session(session->session_id);
    sm.close_session(sm.create_session("fs:")->session_id);
}

TEST(HleServiceManager, ServiceDispatch) {
    ServiceManager& sm = ServiceManager::instance();
    ServiceFactory::create_builtin_services(sm);

    // Test am: GetAppletType
    IService* am = sm.get_service("am:");
    EXPECT_NE(am, nullptr);

    uint32_t applet_type = 0xFFFFFFFF;
    Result dispatch_result = am->dispatch(nullptr, 0, nullptr, 0, &applet_type, sizeof(applet_type));
    EXPECT_EQ(dispatch_result, Result::Success);
    EXPECT_EQ(applet_type, 0u); // Application type

    // GetPerformanceMode
    uint32_t perf_mode = 0xFFFFFFFF;
    dispatch_result = am->dispatch(nullptr, 1, nullptr, 0, &perf_mode, sizeof(perf_mode));
    EXPECT_EQ(dispatch_result, Result::Success);
    EXPECT_EQ(perf_mode, 0u);

    // Test time: GetCurrentTime
    IService* time_svc = sm.get_service("time:");
    EXPECT_NE(time_svc, nullptr);

    uint64_t time_result = 0;
    dispatch_result = time_svc->dispatch(nullptr, 0, nullptr, 0, &time_result, sizeof(time_result));
    EXPECT_EQ(dispatch_result, Result::Success);
    EXPECT_EQ(time_result, 0ull);
}

TEST(HleServiceManager, SessionLifecycle) {
    ServiceManager& sm = ServiceManager::instance();
    ServiceFactory::create_builtin_services(sm);

    Session* s1 = sm.create_session("sm:");
    Session* s2 = sm.create_session("fs:");
    Session* s3 = sm.create_session("am:");

    EXPECT_NE(s1, nullptr);
    EXPECT_NE(s2, nullptr);
    EXPECT_NE(s3, nullptr);
    EXPECT_NE(s1->session_id, s2->session_id);
    EXPECT_NE(s2->session_id, s3->session_id);

    // Close sessions
    sm.close_session(s1->session_id);
    sm.close_session(s2->session_id);
    sm.close_session(s3->session_id);
}

TEST(HleFileSystem, OpenFile) {
    ServiceManager& sm = ServiceManager::instance();
    ServiceFactory::create_builtin_services(sm);

    IService* fs = sm.get_service("fs:");
    EXPECT_NE(fs, nullptr);

    // Try to open a file from ExeFS
    const char* path = "main";
    uint64_t handle = 0;
    Result result = fs->dispatch(nullptr, 0, path, strlen(path) + 1, &handle, sizeof(handle));
    EXPECT_EQ(result, Result::Success);
    EXPECT_NE(handle, 0);

    // Try to get file size
    uint64_t size = 0;
    result = fs->dispatch(nullptr, 6, &handle, sizeof(handle), &size, sizeof(size));
    EXPECT_EQ(result, Result::Success);
    EXPECT_EQ(size, 5720825ull); // main.nso file size (5.46 MB)
}

TEST(HleFileSystem, OpenDirectory) {
    ServiceManager& sm = ServiceManager::instance();
    ServiceFactory::create_builtin_services(sm);

    IService* fs = sm.get_service("fs:");
    EXPECT_NE(fs, nullptr);

    // Try to open ExeFS directory
    const char* path = "";
    uint64_t handle = 0;
    Result result = fs->dispatch(nullptr, 5, path, strlen(path) + 1, &handle, sizeof(handle));
    EXPECT_EQ(result, Result::Success);
    EXPECT_NE(handle, 0);
}

// ============================================================================
// Threading SVC Tests
// ============================================================================

TEST(HleThreading, CreateThreadAndWait) {
    // Test CreateThread (SVC 0x0A) and WaitSynchronization (SVC 0x0B)
    SrCpuState cpu;
    sr_cpu_init(&cpu);

    // Set up CreateThread arguments
    // x0 = entry point (we'll use a dummy function)
    // x1 = arg
    // x2 = stack base
    // x3 = stack size
    // x4 = priority
    // x5 = core id
    cpu.x[0] = 0x1000; // dummy entry point
    cpu.x[1] = 0x1234; // arg
    cpu.x[2] = 0x100000; // stack base
    cpu.x[3] = 0x10000; // stack size (64KB)
    cpu.x[4] = 0; // priority
    cpu.x[5] = -1; // core id (any)

    // Call CreateThread via dispatch
    bool handled = sr_rt_dispatch_thread_svc(&cpu, 0x0A);
    EXPECT_TRUE(handled);

    // Check that we got a thread handle back
    uint64_t thread_handle = cpu.x[0];
    EXPECT_NE(thread_handle, 0ull);
    EXPECT_EQ(cpu.x[1], 0ull); // Success

    // Call WaitSynchronization on the thread
    cpu.x[0] = thread_handle;
    handled = sr_rt_dispatch_thread_svc(&cpu, 0x0B);
    EXPECT_TRUE(handled);
    EXPECT_EQ(cpu.x[0], 0ull); // Success
}

TEST(HleThreading, StartThread) {
    // Test StartThread (SVC 0x22)
    SrCpuState cpu;
    sr_cpu_init(&cpu);

    // Create a thread first
    cpu.x[0] = 0x1000;
    cpu.x[1] = 0x1234;
    cpu.x[2] = 0x100000;
    cpu.x[3] = 0x10000;
    cpu.x[4] = 0;
    cpu.x[5] = -1;

    bool handled = sr_rt_dispatch_thread_svc(&cpu, 0x0A);
    EXPECT_TRUE(handled);
    uint64_t thread_handle = cpu.x[0];

    // Call StartThread
    cpu.x[0] = thread_handle;
    handled = sr_rt_dispatch_thread_svc(&cpu, 0x22);
    EXPECT_TRUE(handled);
    EXPECT_EQ(cpu.x[0], 0ull); // Success
}

TEST(HleThreading, SleepThread) {
    // Test SleepThread (SVC 0x24)
    SrCpuState cpu;
    sr_cpu_init(&cpu);

    // Sleep for a short duration
    cpu.x[0] = 1000000; // 1ms in nanoseconds

    bool handled = sr_rt_dispatch_thread_svc(&cpu, 0x24);
    EXPECT_TRUE(handled);
    EXPECT_EQ(cpu.x[0], 0ull); // Success
}

TEST(HleThreading, GetResult) {
    // Test GetResult (SVC 0x0C)
    SrCpuState cpu;
    sr_cpu_init(&cpu);

    bool handled = sr_rt_dispatch_thread_svc(&cpu, 0x0C);
    EXPECT_TRUE(handled);
    EXPECT_EQ(cpu.x[0], 0ull); // Returns 0 in simplified impl
}

TEST(HleThreading, EventSynchronization) {
    // Test CreateEvent, SignalEvent, WaitEvent
    SrCpuState cpu;
    sr_cpu_init(&cpu);

    // Create event
    cpu.x[1] = 0; // initial state = non-signaled
    cpu.x[2] = 1; // auto-reset
    bool handled = sr_rt_dispatch_thread_svc(&cpu, 0x42);
    EXPECT_TRUE(handled);
    EXPECT_EQ(cpu.x[0], 0ull);
    uint64_t event_id = cpu.x[1];
    EXPECT_NE(event_id, 0ull);

    // Signal event
    cpu.x[0] = event_id;
    handled = sr_rt_dispatch_thread_svc(&cpu, 0x40);
    EXPECT_TRUE(handled);
    EXPECT_EQ(cpu.x[0], 0ull);

    // Wait for event
    cpu.x[0] = event_id;
    cpu.x[1] = 1000000000; // 1 second timeout
    handled = sr_rt_dispatch_thread_svc(&cpu, 0x41);
    EXPECT_TRUE(handled);
    EXPECT_EQ(cpu.x[0], 0ull);
}

TEST(HleThreading, CloseHandle) {
    // Test CloseHandle
    SrCpuState cpu;
    sr_cpu_init(&cpu);

    // Create a thread
    cpu.x[0] = 0x1000;
    cpu.x[1] = 0x1234;
    cpu.x[2] = 0x100000;
    cpu.x[3] = 0x10000;
    cpu.x[4] = 0;
    cpu.x[5] = -1;

    bool handled = sr_rt_dispatch_thread_svc(&cpu, 0x0A);
    EXPECT_TRUE(handled);
    uint64_t thread_handle = cpu.x[0];

    // Close the handle
    cpu.x[0] = thread_handle;
    handled = sr_rt_dispatch_thread_svc(&cpu, 0x43);
    EXPECT_TRUE(handled);
    EXPECT_EQ(cpu.x[0], 0ull); // Success

    // Try to close again - should fail
    cpu.x[0] = thread_handle;
    handled = sr_rt_dispatch_thread_svc(&cpu, 0x43);
    EXPECT_TRUE(handled);
    EXPECT_NE(cpu.x[0], 0ull); // Not found
}