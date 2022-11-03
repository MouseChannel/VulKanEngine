#pragma once
#include "../base.h"
#include "description.h"
#include "device.hpp"
#include "vulkan/vulkan_core.h"
#include <vector>

namespace VK::Wrapper {

class DescriptorSetLayout {
private:
  VkDescriptorSetLayout m_Layout{nullptr};
  Device::Ptr m_Device{nullptr};
  std::vector<UniformParameter::Ptr> m_Params{};

public:
  using Ptr = std::shared_ptr<DescriptorSetLayout>;
  static Ptr Create(const Device::Ptr &device) {
    return std::make_shared<DescriptorSetLayout>(device);
  }

  DescriptorSetLayout(const Device::Ptr &device);
  ~DescriptorSetLayout();
  void Build(std::vector<UniformParameter::Ptr> params);
  [[nodiscard]] auto  &GetLayout() const { return m_Layout; }
};

DescriptorSetLayout::DescriptorSetLayout(const Device::Ptr &device) {
  m_Device = device;
}

DescriptorSetLayout::~DescriptorSetLayout() {
  if (m_Layout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(m_Device->GetDevice(), m_Layout, nullptr);
  }
}
void DescriptorSetLayout::Build(std::vector<UniformParameter::Ptr> params) {
  m_Params = params;

  if (m_Layout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(m_Device->GetDevice(), m_Layout, nullptr);
  }
  std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};

  for (const auto &param : m_Params) {
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.descriptorType = param->mDescriptorType;
    layoutBinding.binding = param->mBinding;
    layoutBinding.stageFlags = param->mStage;
    layoutBinding.descriptorCount = param->mCount;

    layoutBindings.push_back(layoutBinding);
  }

  VkDescriptorSetLayoutCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  createInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
  createInfo.pBindings = layoutBindings.data();

  if (vkCreateDescriptorSetLayout(m_Device->GetDevice(), &createInfo, nullptr,
                                  &m_Layout) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to create descriptor set layout");
  }
}

} // namespace VK::Wrapper