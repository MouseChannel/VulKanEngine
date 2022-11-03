#pragma once
#include "../base.h"
#include "device.hpp"
namespace VK::Wrapper {
class CommandPool {
private:
  VkCommandPool mCommandPool{VK_NULL_HANDLE};
  Device::Ptr mDevice{nullptr};

public:
  using Ptr = std::shared_ptr<CommandPool>;
  static Ptr Create(const Device::Ptr &device,
                    VkCommandPoolCreateFlagBits flag =
                        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
    return std::make_shared<CommandPool>(device, flag);
  }

  CommandPool(const Device::Ptr &device,
              VkCommandPoolCreateFlagBits flag =
                  VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  ~CommandPool();
  [[nodiscard]] auto GetCommandPool() const { return mCommandPool; }
};

CommandPool::CommandPool(const Device::Ptr &device,
                         VkCommandPoolCreateFlagBits flag) {
  mDevice = device;

  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.queueFamilyIndex = device->GetGraphicQueueFamily().value();

  createInfo.flags = flag;

  if (vkCreateCommandPool(mDevice->GetDevice(), &createInfo, nullptr,
                          &mCommandPool) != VK_SUCCESS) {
    throw std::runtime_error("Error:  failed to create command pool");
  }
}
CommandPool::~CommandPool() {
  if (mCommandPool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(mDevice->GetDevice(), mCommandPool, nullptr);
  }
}

} // namespace VK::Wrapper