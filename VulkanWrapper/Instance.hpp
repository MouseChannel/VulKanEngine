//
// Created by mocheng on 2022/10/21.
//

#pragma once
 
#ifndef VK_INSTANCE_H
#define VK_INSTANCE_H

#endif // VK_INSTANCE_H
#include "../base.h"
#include "vulkan/vulkan_core.h"
#include <memory>
#include <stdexcept>

namespace VK::Wrapper {
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pMessageData, void *pUserData) {
  std::cout << "ValidationLayer: " << pMessageData->pMessage << std::endl;

  return VK_FALSE;
}
static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *debugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");

  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, debugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}
static void
DestroyDebugUtilsMessengerEXT(VkInstance instance,
                              VkDebugUtilsMessengerEXT debugMessenger,
                              const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func != nullptr) {
    return func(instance, debugMessenger, pAllocator);
  }
}

class Instance {
private:
  VkInstance m_Instance{nullptr};
  VkDebugUtilsMessengerEXT m_Debugger{VK_NULL_HANDLE};

public:
  using Ptr = std::shared_ptr<Instance>;
  static Ptr Create() { return std::make_shared<Instance>(); }
  Instance();

  ~Instance();
  [[nodiscard]] auto GetInstance() const { return m_Instance; }
  bool CheckValidationLayerSupport();
  std::vector<const char *> GetRequiredExtensions();
  void PrintAvailableExtensions();
  void SetupDebugger();
};

inline Instance::Instance() {
  if (!CheckValidationLayerSupport()) {
    throw std::runtime_error("Error: validation layer is not supported");
  }

  PrintAvailableExtensions();

  VkApplicationInfo app_info{};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "mochengVK";

  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No ENGINE";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info{};

  auto extensions = GetRequiredExtensions();
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledLayerCount =
      static_cast<uint32_t>(validationLayers.size());
  create_info.ppEnabledLayerNames = validationLayers.data();

  if (vkCreateInstance(&create_info, nullptr, &m_Instance) != VK_SUCCESS) {
    throw std::runtime_error("create Instance failed");
  }
  SetupDebugger();
}
Instance::~Instance() {
  DestroyDebugUtilsMessengerEXT(m_Instance, m_Debugger, nullptr);

 
  vkDestroyInstance(m_Instance, nullptr);
}
bool Instance::CheckValidationLayerSupport() {
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const auto &layerName : validationLayers) {
    bool layerFound = false;

    for (const auto &layerProp : availableLayers) {
      if (std::strcmp(layerName, layerProp.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}
void Instance::SetupDebugger() {
  // if (!mEnableValidationLayer) { return; }

  VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

  createInfo.pfnUserCallback = debugCallBack;
  createInfo.pUserData = nullptr;

  if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr,
                                   &m_Debugger) != VK_SUCCESS) {
    throw std::runtime_error("Error:failed to create debugger");
  }
  std::cout << "vaild here" << std::endl;
}

std::vector<const char *> Instance::GetRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;

  const char **glfwExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);

  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  return extensions;
}

void Instance::PrintAvailableExtensions() {
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         extensions.data());

  std::cout << "Available extensions:" << std::endl;

  for (const auto &extension : extensions) {
    std::cout << extension.extensionName << std::endl;
  }
}

} // namespace VK::Wrapper