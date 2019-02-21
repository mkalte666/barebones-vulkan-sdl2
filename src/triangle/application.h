/*
    application.h: A Simple Vulkan/SDL2 Application
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
#ifndef _application_h
#define _application_h

#include <SDL.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

struct ApplicationCreateInfo {
    std::string title = "";
    int x = SDL_WINDOWPOS_CENTERED;
    int y = SDL_WINDOWPOS_CENTERED;
    int w = 800;
    int h = 600;
    Uint32 sdlInitFlags = SDL_INIT_EVERYTHING;
    bool enableValidation = true;
    std::vector<const char*> instanceExtensions;
    std::vector<const char*> instanceLayers;
    std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    vk::PresentModeKHR defaultPresetMode = vk::PresentModeKHR::eFifo;
};

/**
 * \brief An Application using vulkan 
 * Init order is SDL->Vulkan 
 * Cleanup is the other way around
 * We use the c++ headers, so vulkan should clean itself up quite well
 */
class Application {
public:
    /**
     * Create a simple aplication with SDL event Handling
     * Vulkan queues and what not
     */
    Application(const ApplicationCreateInfo& appCreateInfo = ApplicationCreateInfo());
    /**
     * \brief Destructor
     */
    virtual ~Application();

    /**
     * Enter application loop
     */
    virtual void run();

    /**
     * Handle SDL events
     */
    virtual void handleEvent(const SDL_Event& e);

private:
    /// inits all of the vulkan we need
    virtual void initVulkan();
    /// init the vulkan instance
    virtual void initVulkanInstance();
    /// init the vulkan window surface
    virtual void initVulkanSurface();
    /// init (pick really) the vulkan physical device
    virtual void initVulkanPhysicalDevice();
    /// init the vulkan logical device
    virtual void initVulkanLogicalDevice();
    /// init/rebuild the swap chain 
    virtual void rebuildSwapchain();

private:
    ApplicationCreateInfo createInfo;
    bool running;
    SDL_Window* window;
    vk::Instance instance;
    vk::DispatchLoaderDynamic dlinstance;
    vk::DebugUtilsMessengerEXT debugMessenger;
    vk::SurfaceKHR windowSurface;
    vk::PhysicalDevice physicalDevice;
    vk::Device logicalDevice;
    vk::DispatchLoaderDynamic dldevice;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::SwapchainKHR swapchain;
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::ImageView> swapchainViews;
};

#endif // _application_h