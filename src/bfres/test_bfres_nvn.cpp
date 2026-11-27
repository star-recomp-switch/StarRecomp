// Test BFRES texture loading with NVN resource manager integration

#include "kirbyrecomp/bfres/bfres.h"
#include "kirbyrecomp/bfres/cmp_decompress.h"
#include "kirbyrecomp/nvn/resource_manager.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <vulkan/vulkan.h>

// Minimal Vulkan setup for testing
static VkInstance create_instance() {
    VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.pApplicationName = "BFRES NVN Test";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "KirbyRecomp";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    // Enable extensions needed by VMA
    const char* instance_extensions[] = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME
    };

    VkInstanceCreateInfo create_info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = 2;
    create_info.ppEnabledExtensionNames = instance_extensions;

    VkInstance instance;
    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return instance;
}

static VkPhysicalDevice pick_physical_device(VkInstance instance) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0) return VK_NULL_HANDLE;

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());
    return devices[0];
}

static VkDevice create_device(VkPhysicalDevice physical_device, uint32_t& queue_family_out, VkQueue& queue_out) {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    uint32_t graphics_queue_family = UINT32_MAX;
    for (uint32_t i = 0; i < queue_family_count; ++i) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_queue_family = i;
            break;
        }
    }
    if (graphics_queue_family == UINT32_MAX) return VK_NULL_HANDLE;

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queue_info.queueFamilyIndex = graphics_queue_family;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_priority;

    // Enable device extensions needed by VMA
    const char* device_extensions[] = {
        VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
        VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
        VK_KHR_BIND_MEMORY_2_EXTENSION_NAME
    };

    VkDeviceCreateInfo create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    create_info.queueCreateInfoCount = 1;
    create_info.pQueueCreateInfos = &queue_info;
    create_info.enabledExtensionCount = 3;
    create_info.ppEnabledExtensionNames = device_extensions;

    VkDevice device;
    VkResult result = vkCreateDevice(physical_device, &create_info, nullptr, &device);
    if (result != VK_SUCCESS) return VK_NULL_HANDLE;

    VkQueue queue;
    vkGetDeviceQueue(device, graphics_queue_family, 0, &queue);

    queue_family_out = graphics_queue_family;
    queue_out = queue;
    return device;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.cmp or file.bfres>" << std::endl;
        return 1;
    }

    std::string input_path = argv[1];
    std::vector<uint8_t> bfres_data;

    // Check if it's a .cmp file
    if (input_path.size() >= 4 && input_path.substr(input_path.size() - 4) == ".cmp") {
        std::cout << "Decompressing .cmp file..." << std::endl;
        std::string output_path = "/tmp/test_output.bfres";
        if (!kirbyrecomp::bfres::decompress_cmp_file(input_path, output_path)) {
            std::cerr << "Failed to decompress .cmp file" << std::endl;
            return 1;
        }

        std::ifstream f(output_path, std::ios::binary | std::ios::ate);
        if (!f) {
            std::cerr << "Failed to open decompressed file" << std::endl;
            return 1;
        }
        size_t size = f.tellg();
        f.seekg(0, std::ios::beg);
        bfres_data.resize(size);
        f.read(reinterpret_cast<char*>(bfres_data.data()), size);
        f.close();
    } else {
        std::ifstream f(input_path, std::ios::binary | std::ios::ate);
        if (!f) {
            std::cerr << "Failed to open file" << std::endl;
            return 1;
        }
        size_t size = f.tellg();
        f.seekg(0, std::ios::beg);
        bfres_data.resize(size);
        f.read(reinterpret_cast<char*>(bfres_data.data()), size);
        f.close();
    }

    std::cout << "File size: " << bfres_data.size() << " bytes" << std::endl;

    // Parse BFRES
    kirbyrecomp::bfres::BfresFile bfres(bfres_data);
    if (!bfres.parse()) {
        std::cerr << "Failed to parse BFRES: " << bfres.get_error() << std::endl;
        return 1;
    }

    std::cout << "BFRES parsed successfully!" << std::endl;
    std::cout << "BNTX Textures found: " << bfres.get_bntx_textures().size() << std::endl;

    // Initialize Vulkan
    std::cout << "\nInitializing Vulkan..." << std::endl;
    VkInstance instance = create_instance();
    if (instance == VK_NULL_HANDLE) {
        std::cerr << "Failed to create Vulkan instance" << std::endl;
        return 1;
    }

    VkPhysicalDevice physical_device = pick_physical_device(instance);
    if (physical_device == VK_NULL_HANDLE) {
        std::cerr << "No physical device found" << std::endl;
        vkDestroyInstance(instance, nullptr);
        return 1;
    }

    uint32_t queue_family;
    VkQueue queue;
    VkDevice device = create_device(physical_device, queue_family, queue);
    if (device == VK_NULL_HANDLE) {
        std::cerr << "Failed to create Vulkan device" << std::endl;
        vkDestroyInstance(instance, nullptr);
        return 1;
    }

    std::cout << "Vulkan initialized successfully!" << std::endl;

    // Create NVN resource manager
    kirbyrecomp::nvn::ResourceManager resource_manager(device, physical_device, instance);
    if (!resource_manager.initialize()) {
        std::cerr << "Failed to initialize resource manager: " << resource_manager.get_last_error() << std::endl;
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
        return 1;
    }

    std::cout << "Resource manager initialized!" << std::endl;

    // Test loading first few BNTX textures
    size_t max_textures = std::min<size_t>(bfres.get_bntx_textures().size(), 3);
    for (size_t i = 0; i < max_textures; ++i) {
        const auto& tex = bfres.get_bntx_textures()[i];
        std::cout << "\nLoading BNTX Texture " << i << ": " << tex.name << std::endl;
        std::cout << "  Size: " << tex.width << "x" << tex.height << ", Format: 0x" << std::hex << static_cast<uint32_t>(tex.format) << std::dec << std::endl;

        auto tex_data = bfres.get_bntx_texture_data(i);
        if (!tex_data) {
            std::cerr << "  Failed to extract texture data" << std::endl;
            continue;
        }

        std::cout << "  Extracted data size: " << tex_data->size() << std::endl;

        // Create BfresTextureLoadInfo
        kirbyrecomp::nvn::ResourceManager::BfresTextureLoadInfo load_info;
        load_info.name = tex.name;
        load_info.width = tex.width;
        load_info.height = tex.height;
        load_info.depth = tex.depth;
        load_info.mip_levels = tex.mip_levels;
        load_info.array_layers = tex.array_layers;
        load_info.format = static_cast<uint32_t>(tex.format);
        load_info.data = tex_data->data();
        load_info.data_size = static_cast<uint32_t>(tex_data->size());
        load_info.mip_offsets = tex.mip_offsets;
        load_info.mip_sizes = tex.mip_sizes;

        // Load texture via NVN resource manager (with ASTC->BC7 transcoding)
        uint64_t handle = resource_manager.create_texture_from_bfres(load_info);
        if (handle == 0) {
            std::cerr << "  Failed to create texture: " << resource_manager.get_last_error() << std::endl;
        } else {
            std::cout << "  Texture created successfully! Handle: " << handle << std::endl;
        }
    }

    // Cleanup
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);

    std::cout << "\nTest completed!" << std::endl;
    return 0;
}