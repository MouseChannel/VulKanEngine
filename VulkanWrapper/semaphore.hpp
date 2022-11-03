#include "../base.h"
#include "device.hpp"

namespace VK::Wrapper {
class Semaphore {
private:
  VkSemaphore m_Semaphore{VK_NULL_HANDLE};
  Device::Ptr m_Device{nullptr};

public:
  using Ptr = std::shared_ptr<Semaphore>;
  static Ptr Create(const Device::Ptr &device) {
    return std::make_shared<Semaphore>(device);
  }
  Semaphore(const Device::Ptr &device);

  ~Semaphore(){
		if (m_Semaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(m_Device->GetDevice(), m_Semaphore, nullptr);
		}
	}

  [[nodiscard]] auto& GetSemaphore() const { return m_Semaphore; }
};

Semaphore::Semaphore(const Device::Ptr &device) {
  m_Device = device;

  VkSemaphoreCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  if (vkCreateSemaphore(m_Device->GetDevice(), &createInfo, nullptr,
                        &m_Semaphore) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to create Semaphore");
  }
}
} // namespace VK::Wrapper