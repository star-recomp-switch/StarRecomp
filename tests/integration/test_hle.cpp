// HLE Service Manager Tests

#include <gtest/gtest.h>
#include <kirbyrecomp/hle/service.h>
#include <memory>

using namespace kr::hle;

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