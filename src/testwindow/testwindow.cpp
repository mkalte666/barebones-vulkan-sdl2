/*
    testwindow.cpp: A window to test the installation
    Copyright (C) 2019 Malte Kießling

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

#include "SDL.h"
#include <SDL_vulkan.h>
#include <glm/glm.hpp>
#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <vulkan/vulkan.hpp>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Validation Layer:", pCallbackData->pMessage, nullptr);
    return VK_FALSE;
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    auto window = SDL_CreateWindow("Testwindow", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_VULKAN);

    if (!window) {
        auto err = SDL_GetError();
        std::cerr << "Cannot create VULKAN window: " << SDL_GetError() << std::endl;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Window Error", err, nullptr);
        exit(-1);
    }

    auto supportedExtensions = vk::enumerateInstanceExtensionProperties();
    std::cerr << "Vulkan extensions supported: " << supportedExtensions.size() << std::endl;
    for (const auto& extension : supportedExtensions) {
        std::cerr << "Extension found: " << extension.extensionName << std::endl;
    }

    std::vector<const char*> extensions = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

    // get count of sdl extensions
    uint32_t extensionCount;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
    // actually query them
    uint32_t extensionOffset = static_cast<uint32_t>(extensions.size());
    extensions.resize(extensions.size() + extensionCount);
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data() + extensionOffset);

    for (auto& extension : extensions) {
        std::cerr << "Loading Extension: " << extension << std::endl;
    }

    const std::vector<const char*> validationLayersRequested = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    // check for validation layers
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    std::vector<const char*> validationLayers;
    for (auto layerName : validationLayersRequested) {
        for (const auto& availableLayer : availableLayers) {
            if (strcmp(layerName, availableLayer.layerName) == 0) {
                validationLayers.push_back(layerName);
            }
        }
    }
    for (auto& layer : validationLayers) {
        std::cerr << "Loading Layer: " << layer << std::endl;
    }

    // just checking glm
    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    try {

        // test for vulkan
        vk::ApplicationInfo appInfo(
            "Api Test",
            VK_MAKE_VERSION(0, 0, 1),
            "No Engine",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_1);
        vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlags(),
            &appInfo,
            static_cast<uint32_t>(validationLayers.size()),
            validationLayers.data(),
            static_cast<uint32_t>(extensions.size()),
            extensions.data());

        auto instance = vk::createInstance(createInfo);
        vk::DispatchLoaderDynamic dloader(instance);

        // create debug messengaer
        vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo(
            vk::DebugUtilsMessengerCreateFlagsEXT(),
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
            debugCallback);
        auto messenger = instance.createDebugUtilsMessengerEXT(messengerCreateInfo, nullptr, dloader);

        // window surface
        VkSurfaceKHR windowSurfaceC;
        SDL_Vulkan_CreateSurface(window, instance, &windowSurfaceC);
        vk::SurfaceKHR windowSurface(windowSurfaceC);

        // info about all physical devices
        auto devices = instance.enumeratePhysicalDevices();
        vk::PhysicalDevice physicalDevice;
        for (const auto& device : devices) {
            auto properties = device.getProperties();
            std::cerr << "Found device: " << properties.deviceName << std::endl;
            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                physicalDevice = device;
            }
        }
        std::cerr << "Device Chosen: " << physicalDevice.getProperties().deviceName << std::endl;

        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Vulkan Test", "This seems to work. Have fun!", window);

    } catch (vk::SystemError err) {
        std::cerr << "vk::SystemError: " << err.what() << std::endl;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Vulkan System Error", err.what(), nullptr);
    }

    SDL_Quit();

    return 0;
}
