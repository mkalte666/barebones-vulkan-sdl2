/*
    utll.cpp: Various (Vulkan) utilities
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

#include "util.h"

QueueFamilyData::QueueFamilyData(const vk::PhysicalDevice& device, vk::SurfaceKHR windowSurface)
{
    auto properties = device.getQueueFamilyProperties();
    int i = 0;
    for (const auto& family : properties) {
        if (family.queueCount > 0 && family.queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsFamily = i;
        }

        bool presentSupport = device.getSurfaceSupportKHR(i, windowSurface);
        if (family.queueCount > 0 && presentSupport) {
            presentFamily = i;
        }

        // are we complete
        if (*this) {
            break;
        }

        i++;
    }
}

std::vector<uint32_t> QueueFamilyData::get() const
{
    std::vector<uint32_t> result;
    if (!(*this)) {
        return result;
    }

    result.push_back(graphicsFamily.value());
    result.push_back(presentFamily.value());

    return result;
}

std::vector<uint32_t> QueueFamilyData::getUnique() const
{
    std::vector<uint32_t> result;
    if (!(*this)) {
        return result;
    }
    for (auto family : get()) {
        if (std::find(result.begin(), result.end(), family) == result.end()) {
            result.push_back(family);
        }
    }
    return result;
}

std::vector<vk::DeviceQueueCreateInfo> QueueFamilyData::getCreateInfos() const
{
    std::vector<vk::DeviceQueueCreateInfo> result;
    float priority = 1.0f;
    for (auto index : getUnique()) {
        vk::DeviceQueueCreateInfo info(
            vk::DeviceQueueCreateFlags(),
            index,
            1,
            &priority);
        result.push_back(info);
    }

    return result;
}

QueueFamilyData::operator bool() const
{
    return graphicsFamily.has_value()
        && presentFamily.has_value();
}