#pragma once
#include "../base.h"
#include "description.h"
#include "descriptorPool.hpp"
#include "descriptorSetLayout.hpp"
#include "device.hpp"
#include "vulkan/vulkan_core.h"

namespace VK::Wrapper {
class DescriptorSet {
private:
  std::vector<VkDescriptorSet> m_DescriptorSets{};
  Device::Ptr m_Device{nullptr};

public:
  using Ptr = std::shared_ptr<DescriptorSet>;
  static Ptr Create(const Device::Ptr &device,
                    const std::vector<UniformParameter::Ptr> params,
                    const DescriptorSetLayout::Ptr &layout,
                    const DescriptorPool::Ptr &pool, int frameCount) {
    return std::make_shared<DescriptorSet>(device, params, layout, pool,
                                           frameCount);
  }
  DescriptorSet(const Device::Ptr &device,
                const std::vector<UniformParameter::Ptr> params,
                const DescriptorSetLayout::Ptr &layout,
                const DescriptorPool::Ptr &pool, int frameCount);
  ~DescriptorSet();
  [[nodiscard]] auto GetDescriptorSet(int frameCount) const {
    return m_DescriptorSets[frameCount];
  }
};

DescriptorSet::DescriptorSet(const Device::Ptr &device,
                             const std::vector<UniformParameter::Ptr> params,
                             const DescriptorSetLayout::Ptr &layout,
                             const DescriptorPool::Ptr &pool, int frameCount) {
  m_Device = device;

  std::vector<VkDescriptorSetLayout> layouts(frameCount, layout->GetLayout());

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = pool->GetPool();
  allocInfo.descriptorSetCount = frameCount;
  allocInfo.pSetLayouts = layouts.data();

  m_DescriptorSets.resize(frameCount);
  if (vkAllocateDescriptorSets(m_Device->GetDevice(), &allocInfo,
                               m_DescriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to allocate descriptor sets");
  }
  std::cout << m_DescriptorSets.size() << std::endl;

  for (int i = 0; i < frameCount; ++i) {
    // descriptorSetWrite，将params的信息，写入buffer
    std::vector<VkWriteDescriptorSet> descriptorSetWrites{};
    for (const auto &param : params) {
      VkWriteDescriptorSet descriptorSetWrite{};
      descriptorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptorSetWrite.dstSet = m_DescriptorSets[i];
      descriptorSetWrite.dstArrayElement = 0;
      descriptorSetWrite.descriptorType = param->mDescriptorType;
      descriptorSetWrite.descriptorCount = param->mCount;
      descriptorSetWrite.dstBinding = param->mBinding;

      if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
        descriptorSetWrite.pBufferInfo = &param->m_Buffers[i]->GetBufferInfo();
      }

      if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        descriptorSetWrite.pImageInfo = &param->mTexture->GetImageInfo();
      }

      descriptorSetWrites.push_back(descriptorSetWrite);
    }

    vkUpdateDescriptorSets(m_Device->GetDevice(),
                           static_cast<uint32_t>(descriptorSetWrites.size()),
                           descriptorSetWrites.data(), 0, nullptr);
  }
}

DescriptorSet::~DescriptorSet() {}
} // namespace VK::Wrapper