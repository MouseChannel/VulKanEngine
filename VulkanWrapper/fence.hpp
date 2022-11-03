#include "../base.h"
#include "device.hpp"

namespace VK::Wrapper {
class Fence {
private:
  VkFence m_Fence{VK_NULL_HANDLE};
  Device::Ptr m_Device{nullptr};

public:
  using Ptr = std::shared_ptr<Fence>;
  static Ptr Create(const Device::Ptr &device, bool signaled = true) {
    return std::make_shared<Fence>(device, signaled);
  }
  Fence(const Device::Ptr &device, bool signaled = true);

  ~Fence();
  void ResetFence();

  void Block(uint64_t timeout = UINT64_MAX);

  [[nodiscard]] auto GetFence() const { return m_Fence; }
};
Fence::Fence(const Device::Ptr &device, bool signaled) {
  m_Device = device;

  VkFenceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

  if (vkCreateFence(m_Device->GetDevice(), &createInfo, nullptr, &m_Fence) !=
      VK_SUCCESS) {
    throw std::runtime_error("Error:failed to create fence");
  }
}
Fence::~Fence() {
  if (m_Fence != VK_NULL_HANDLE) {
    vkDestroyFence(m_Device->GetDevice(), m_Fence, nullptr);
  }
}
void Fence::ResetFence() { vkResetFences(m_Device->GetDevice(), 1, &m_Fence); }

void Fence::Block(uint64_t timeout) {
  vkWaitForFences(m_Device->GetDevice(), 1, &m_Fence, VK_TRUE, timeout);
}

} // namespace VK::Wrapper