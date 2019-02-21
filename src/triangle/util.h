/*
    utll.h: Various (Vulkan) utilities
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

#ifndef _util_h
#define _util_h

#include <SDL.h>
#include <vulkan/vulkan.hpp>

#include <optional>
#include <vector>

// somewhat stolen from vulkan-tutorial.com
struct QueueFamilyData {
    QueueFamilyData(const vk::PhysicalDevice& device, vk::SurfaceKHR windowSurface);

    std::vector<uint32_t> get() const;

    std::vector<uint32_t> getUnique() const;

    std::vector<vk::DeviceQueueCreateInfo> getCreateInfos() const;

    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    operator bool() const;
};

#endif //_util_h