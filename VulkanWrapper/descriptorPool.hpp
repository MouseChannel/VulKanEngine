#pragma once

#include "../base.h"
#include "device.hpp"

#include "description.h"

namespace VK::Wrapper {
class DescriptorPool {
private:
  VkDescriptorPool m_Pool{VK_NULL_HANDLE};
  Device::Ptr m_Device{nullptr};

public:
  using Ptr = std::shared_ptr<DescriptorPool>;
  static Ptr Create(const Device::Ptr &device) {
    return std::make_shared<DescriptorPool>(device);
  }
  DescriptorPool(const Device::Ptr &device);
  ~DescriptorPool();

  void Build(std::vector<UniformParameter::Ptr> &params, const int &frameCount);
  [[nodiscard]] auto GetPool() const { return m_Pool; }
};
DescriptorPool::DescriptorPool(const Device::Ptr &device) { m_Device = device; }
DescriptorPool::~DescriptorPool() {
  if (m_Pool != VK_NULL_HANDLE) {
    vkDestroyDescriptorPool(m_Device->GetDevice(), m_Pool, nullptr);
  }
}

void DescriptorPool::Build(std::vector<UniformParameter::Ptr> &params,
                           const int &frameCount) {
  // 统计下有多少个uniformBuffer
  int uniformBufferCount = 0;

  int textureCount = 0;
  for (const auto &param : params) {
    if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
      uniformBufferCount++;
    }
    if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
      textureCount++;
    // TODO
  }
  // 描述每一种uniform都有多少个
  // 即一个DescriptorSet有几个buffer，几个Texture
  std::vector<VkDescriptorPoolSize> poolSizes{};
  VkDescriptorPoolSize uniformBufferSize{};
  uniformBufferSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uniformBufferSize.descriptorCount = uniformBufferCount * frameCount;
  poolSizes.push_back(uniformBufferSize);

  VkDescriptorPoolSize textureSize{};
  textureSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  textureSize.descriptorCount =
      textureCount * frameCount; // 这边的size是指，有多少个descriptor
  poolSizes.push_back(textureSize);

  // 创建pool
  VkDescriptorPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  createInfo.pPoolSizes = poolSizes.data();
  // 每一帧都有自己单独的descriptorSet,不然无法支持并行
  createInfo.maxSets = static_cast<uint32_t>(frameCount);

  if (vkCreateDescriptorPool(m_Device->GetDevice(), &createInfo, nullptr,
                             &m_Pool) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to create Descriptor pool");
  }
}

} // namespace VK::Wrapper