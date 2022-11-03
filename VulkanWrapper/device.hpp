#pragma once
#include "../base.h"
#include "Instance.hpp"
#include "vulkan/vulkan_core.h"
#include "windowSurface.hpp"
#include <iostream>
#include <iterator>
#include <optional>
#include <set>
#include <stdint.h>
#include <vector>

namespace VK::Wrapper {
const std::vector<const char *> deviceRequiredExtensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

class Device {
private:
  Instance::Ptr m_Instance;
  VkDevice m_Device;
  WindowSurface::Ptr m_Surface{nullptr};
  VkPhysicalDevice m_PhysicalDevice{VK_NULL_HANDLE};
  // 渲染队列
  std::optional<uint32_t> m_GraphicQueueFamily;
  VkQueue m_GraphicQueue{VK_NULL_HANDLE};
  // 显示队列
  std::optional<uint32_t> m_PresentQueueFamily;
  VkQueue m_PresentQueue{VK_NULL_HANDLE};

public:
  using Ptr = std::shared_ptr<Device>;
  static Ptr Create(Instance::Ptr instance, WindowSurface::Ptr surface) {
    return std::make_shared<Device>(instance, surface);
  }

  Device(Instance::Ptr instance, WindowSurface::Ptr surface);
  ~Device();

  void PickPhysicalDevice();
  int RateDevice(VkPhysicalDevice device);
  bool IsDeviceSuitable(VkPhysicalDevice device);
  void InitQueueFamilies(VkPhysicalDevice device);
  void CreateLogicalDevice();
  VkSampleCountFlagBits getMaxUsableSampleCount();
  [[nodiscard]] auto GetDevice() const { return m_Device; }
  [[nodiscard]] auto GetPhysicalDevice() const { return m_PhysicalDevice; }

  [[nodiscard]] auto GetGraphicQueueFamily() const {
    return m_GraphicQueueFamily;
  }
  [[nodiscard]] auto GetPresentQueueFamily() const {
    return m_PresentQueueFamily;
  }
  [[nodiscard]] auto GetPresentQueue() { return m_PresentQueue; }
  [[nodiscard]] auto GetGraphicQueue() { return m_GraphicQueue; }

  bool IsQueueFamilyComplete();
};
Device::Device(Instance::Ptr instance, WindowSurface::Ptr surface) {
  m_Instance = instance;
  m_Surface = surface;
  PickPhysicalDevice();
  InitQueueFamilies(m_PhysicalDevice);
  CreateLogicalDevice();
}
Device::~Device() {
  vkDestroyDevice(m_Device, nullptr);

  m_Surface.reset();
  m_Instance.reset();
}

void Device::PickPhysicalDevice() {

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(m_Instance->GetInstance(), &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("Error:failed to enumeratePhysicalDevice");
  }
  std::cout << "has devices: " << deviceCount << std::endl;

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(m_Instance->GetInstance(), &deviceCount,
                             devices.data());
  for (auto i : devices) {
    m_PhysicalDevice = i;
    break;
  }
  IsDeviceSuitable(m_PhysicalDevice);

  // auto physicalSupport = IsDeviceSuitable(m_PhysicalDevice);
  // std::cout << "vaild : " << physicalSupport << std::endl;
  /*
          std::multimap<int, VkPhysicalDevice> candidates;

          for (const auto &device : devices) {
            int score = rateDevice(device);
            candidates.insert(std::make_pair(score, device));
          }

          if (candidates.rbegin()->first > 0 &&
                  isDeviceSuitable(candidates.rbegin()->second)) {
            mPhysicalDevice = candidates.rbegin()->second;
          }

          if (mPhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Error:failed to get physical device");
          }
          */
}
bool Device::IsDeviceSuitable(VkPhysicalDevice device) {

  VkPhysicalDeviceProperties deviceProp;
  vkGetPhysicalDeviceProperties(device, &deviceProp);

  VkPhysicalDeviceFeatures deviceFeatures;
  vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
  std::cout << "samp" << deviceFeatures.samplerAnisotropy << std::endl;
  //  VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
  return deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
         deviceFeatures.geometryShader;
}

inline void Device::CreateLogicalDevice() {

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

  std::set<uint32_t> queueFamilies = {m_GraphicQueueFamily.value(),
                                      m_PresentQueueFamily.value()};
  // queueFamilies.insert(m_GraphicQueueFamily.value());
  // queueFamilies.insert(m_PresentQueueFamily.value() );

  float queuePriority = 1.0;

  for (uint32_t queueFamily : queueFamilies) {

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
  deviceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceRequiredExtensions.size());
  deviceCreateInfo.ppEnabledExtensionNames = deviceRequiredExtensions.data();

  //   // layer��
  //   if (mInstance->getEnableValidationLayer()) {
  deviceCreateInfo.enabledLayerCount =
      static_cast<uint32_t>(validationLayers.size());
  deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
  //   } else {
  //     deviceCreateInfo.enabledLayerCount = 0;
  //   }
  deviceCreateInfo.enabledLayerCount = 0;

  if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) !=
      VK_SUCCESS) {
    throw std::runtime_error("Error:failed to create logical device");
  }

  vkGetDeviceQueue(m_Device, m_GraphicQueueFamily.value(), 0, &m_GraphicQueue);
  vkGetDeviceQueue(m_Device, m_PresentQueueFamily.value(), 0, &m_PresentQueue);
}

void Device::InitQueueFamilies(VkPhysicalDevice device) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueCount > 0 &&
        (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
      m_GraphicQueueFamily = i;
    }

    VkBool32 presentSupport = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface->GetSurface(),
                                         &presentSupport);

    if (presentSupport) {
      m_PresentQueueFamily = i;
    }

    if (IsQueueFamilyComplete()) {
      break;
    }

    ++i;
  }
}

bool Device::IsQueueFamilyComplete() {

  return m_GraphicQueueFamily.has_value() && m_PresentQueueFamily.has_value();
}
VkSampleCountFlagBits Device::getMaxUsableSampleCount() {
  VkPhysicalDeviceProperties props{};
  vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);

  VkSampleCountFlags counts =
      std::min(props.limits.framebufferColorSampleCounts,
               props.limits.framebufferDepthSampleCounts);

  if (counts & VK_SAMPLE_COUNT_64_BIT) {
    return VK_SAMPLE_COUNT_64_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_32_BIT) {
    return VK_SAMPLE_COUNT_32_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_16_BIT) {
    return VK_SAMPLE_COUNT_16_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_8_BIT) {
    return VK_SAMPLE_COUNT_8_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_4_BIT) {
    return VK_SAMPLE_COUNT_4_BIT;
  }
  if (counts & VK_SAMPLE_COUNT_2_BIT) {
    return VK_SAMPLE_COUNT_2_BIT;
  }

  return VK_SAMPLE_COUNT_1_BIT;
}
} // namespace VK::Wrapper