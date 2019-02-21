/*
    application.cpp: A Simple Vulkan/SDL2 Application
    Copyright (C) 2019 Malte Kieﬂling

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "application.h"

#include "SDL_vulkan.h"
#include "util.h"
#include <iostream>
#include <optional>

// needed down below for validation layers
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{

    std::cerr << "vulkan validation: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

Application::Application(const ApplicationCreateInfo& appCreateInfo)
    : createInfo(appCreateInfo)
    , running(true)
    , window(nullptr)
{
    if (SDL_Init(createInfo.sdlInitFlags) != 0) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical SDL Error", SDL_GetError(), nullptr);
        exit(-1);
    }

    window = SDL_CreateWindow(
        createInfo.title.c_str(),
        createInfo.x,
        createInfo.y,
        createInfo.w,
        createInfo.h,
        SDL_WINDOW_VULKAN);
    if (!window) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical SDL Window Error", SDL_GetError(), nullptr);
        exit(-1);
    }

    initVulkan();
}

Application::~Application()
{
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}

void Application::run()
{
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            handleEvent(e);
        }
    }

    logicalDevice.waitIdle();
}

void Application::handleEvent(const SDL_Event& e)
{
    switch (e.type) {
    case SDL_WINDOWEVENT:
        switch (e.window.type) {
        case SDL_WINDOWEVENT_CLOSE:
            running = false;
            break;
        case SDL_WINDOWEVENT_RESIZED:
            // TODO: Handle resize?
            break;
        default:
            break;
        }
        break;
    case SDL_QUIT:
        running = false;
        break;
    default:
        break;
    }
}

void Application::initVulkan()
{
    try {
        initVulkanInstance();
        initVulkanSurface();
        initVulkanPhysicalDevice();
        initVulkanLogicalDevice();
        rebuildSwapchain();
    } catch (vk::SystemError err) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Ciritical Vulkan Error", err.what(), window);
        exit(-1);
    }
}

void Application::initVulkanInstance()
{
    // get sdl2 needed extensions
    uint32_t sdlExtensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);

    // if we want validation, we need the extension for the callback
    if (createInfo.enableValidation) {
        createInfo.instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    std::vector<const char*> extensions;
    extensions.resize(sdlExtensionCount);
    SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, extensions.data());
    // add requested extensions
    extensions.insert(extensions.end(), createInfo.instanceExtensions.begin(), createInfo.instanceExtensions.end());

    // add validation layers if compatible and requested
    // also do the fixed adding of VK_LAYER_LUNARG_standard_validation
    if (createInfo.enableValidation) {
        createInfo.instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
    }

    auto availableLayers = vk::enumerateInstanceLayerProperties();
    std::vector<const char*> layers;
    for (auto layerName : createInfo.instanceLayers) {
        for (const auto& availableLayer : availableLayers) {
            if (strcmp(layerName, availableLayer.layerName) == 0) {
                layers.push_back(layerName);
            }
        }
    }

    vk::ApplicationInfo appInfo(
        createInfo.title.c_str(),
        VK_MAKE_VERSION(0, 0, 1),
        "No Engine",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_1);

    vk::InstanceCreateInfo instanceInfo(
        vk::InstanceCreateFlags(),
        &appInfo,
        static_cast<uint32_t>(layers.size()),
        layers.data(),
        static_cast<uint32_t>(extensions.size()),
        extensions.data());

    instance = vk::createInstance(instanceInfo);
    dlinstance.init(instance);

    if (createInfo.enableValidation) {
        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo(
            vk::DebugUtilsMessengerCreateFlagsEXT(),
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
            debugCallback,
            nullptr);
        debugMessenger = instance.createDebugUtilsMessengerEXT(debugCreateInfo, nullptr, dlinstance);
    }
}

void Application::initVulkanSurface()
{
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL Vulkan Window Surface Error", SDL_GetError(), window);
        exit(-1);
    }
    windowSurface = surface;
}

void Application::initVulkanPhysicalDevice()
{
    // lets see what we have
    auto availableDevices = instance.enumeratePhysicalDevices();
    if (availableDevices.empty()) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Vulkan Error", "No GPU Available", window);
        exit(-1);
    }
    // pick the phyiscal device.
    // we must support all extensions,
    for (auto device : availableDevices) {
        // check for queues
        if (!QueueFamilyData(device, windowSurface)) {
            continue;
        }

        // check extensions
        auto supportedExtensions = device.enumerateDeviceExtensionProperties();
        auto missingExtensions = createInfo.deviceExtensions;
        for (const auto& extension : supportedExtensions) {
            // remove_if failed for some reason
            auto missingIter = std::find_if(missingExtensions.begin(), missingExtensions.end(), [&extension](const char* l) {
                return strcmp(l, extension.extensionName) == 0;
            });
            if (missingIter != missingExtensions.end()) {
                missingExtensions.erase(missingIter);
            }
            if (missingExtensions.empty()) {
                break;
            }
        }
        if (!missingExtensions.empty()) {
            continue;
        }

        // check up on the swap chain
        auto formats = device.getSurfaceFormatsKHR(windowSurface);
        auto modes = device.getSurfacePresentModesKHR(windowSurface);
        if (formats.empty() || modes.empty()) {
            continue;
        }

        physicalDevice = device;
    }

    if (!physicalDevice) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Vulkan Error", "No Suitable GPU Availabe", window);
        exit(-1);
    }

    if (createInfo.enableValidation) {
        std::cerr << "Physical Device: " << physicalDevice.getProperties().deviceName << std::endl;
    }
}

void Application::initVulkanLogicalDevice()
{
    QueueFamilyData familyData(physicalDevice, windowSurface);
    auto queueInfos = familyData.getCreateInfos();
    vk::PhysicalDeviceFeatures features;
    vk::DeviceCreateInfo deviceInfo(
        vk::DeviceCreateFlags(),
        static_cast<uint32_t>(queueInfos.size()),
        queueInfos.data(),
        static_cast<uint32_t>(createInfo.instanceLayers.size()),
        createInfo.instanceLayers.data(),
        static_cast<uint32_t>(createInfo.deviceExtensions.size()),
        createInfo.deviceExtensions.data(),
        &features);

    logicalDevice = physicalDevice.createDevice(deviceInfo);
    dldevice.init(instance, logicalDevice);
    logicalDevice.getQueue(familyData.graphicsFamily.value(), 0, &graphicsQueue, dldevice);
    logicalDevice.getQueue(familyData.presentFamily.value(), 0, &presentQueue, dldevice);
}

void Application::rebuildSwapchain()
{
    // cleanup old swapchain
    logicalDevice.waitIdle();

    // make new swapchain
    // find properties
    auto formats = physicalDevice.getSurfaceFormatsKHR(windowSurface);
    auto modes = physicalDevice.getSurfacePresentModesKHR(windowSurface);
    if (modes.empty() || formats.empty()) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Vulkan Error", "No mode or format for swapchain", window);
        exit(-1);
    }

    // calculate extend
    auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(windowSurface);
    auto extend = capabilities.currentExtent;
    // if this is true, we need to yell a bit
    if (extend.width == std::numeric_limits<uint32_t>::max()) {
        extend = {
            static_cast<uint32_t>(createInfo.w),
            static_cast<uint32_t>(createInfo.h)
        };

        extend.width = std::max(
            capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, extend.width));
        extend.height = std::max(
            capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, extend.height));
    }

    // determin mode. try to use the default one
    auto mode = modes[0];
    for (const auto& availableMode : modes) {
        if (availableMode == createInfo.defaultPresetMode) {
            mode = availableMode;
            break;
        }
    }

    // determin format.
    vk::SurfaceFormatKHR format = formats[0];
    if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined) {
        format = { vk::Format::eB8G8R8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
    }

    // need this for the queue indexes
    auto queueIndexes = QueueFamilyData(physicalDevice, windowSurface).getUnique();

    // swap chain image count
    auto imageCount = std::min(capabilities.maxImageCount, capabilities.minImageCount + 1);
    vk::SwapchainCreateInfoKHR swapCreateInfo(
        vk::SwapchainCreateFlagsKHR(),
        windowSurface,
        imageCount,
        format.format,
        format.colorSpace,
        extend,
        1,
        vk::ImageUsageFlagBits::eColorAttachment,
        queueIndexes.size() > 1 ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive,
        static_cast<uint32_t>(queueIndexes.size()),
        queueIndexes.data(),
        capabilities.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        mode,
        true);

    swapchain = logicalDevice.createSwapchainKHR(swapCreateInfo);
    swapchainImages = logicalDevice.getSwapchainImagesKHR(swapchain);
    swapchainViews.reserve(swapchainImages.size());
    for (auto image : swapchainImages) {
        vk::ComponentMapping mapping(
            vk::ComponentSwizzle::eR,
            vk::ComponentSwizzle::eG,
            vk::ComponentSwizzle::eB,
            vk::ComponentSwizzle::eA);
        vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
        vk::ImageViewCreateInfo viewCreateInfo(
            vk::ImageViewCreateFlags(),
            image,
            vk::ImageViewType::e2D,
            format.format,
            mapping,
            range);
        swapchainViews.push_back(logicalDevice.createImageView(viewCreateInfo));
    }
}
