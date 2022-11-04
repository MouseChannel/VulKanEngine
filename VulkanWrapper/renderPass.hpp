#pragma once
#include "../base.h"
#include "device.hpp"
#include "subPass.hpp"
#include "vulkan/vulkan_core.h"
#include <vector>

namespace VK::Wrapper {

class RenderPass {
private:
  VkRenderPass mRenderPass{VK_NULL_HANDLE};

  std::vector<SubPass> mSubPasses{};
  std::vector<VkSubpassDependency> mDependencies{};

  // description 表示某一个attachment的格式
  std::vector<VkAttachmentDescription> mAttachmentDescriptions{};
 
  Device::Ptr mDevice{nullptr};

public:
  using Ptr = std::shared_ptr<RenderPass>;
  static Ptr Create(const Device::Ptr &device) {
    return std::make_shared<RenderPass>(device);
  }

  RenderPass(const Device::Ptr &device);
  ~RenderPass();
  void AddSubPass(const SubPass &subpass) { mSubPasses.push_back(subpass); }

  void AddDependency(const VkSubpassDependency &dependency) {
    mDependencies.push_back(dependency);
  }

  void AddAttachment(const VkAttachmentDescription &attachmentDes) {
    mAttachmentDescriptions.push_back(attachmentDes);
  }

  void BuildRenderPass();

  [[nodiscard]] auto GetRenderPass() const { return mRenderPass; }
};

RenderPass::RenderPass(const Device::Ptr &device) { mDevice = device; }

RenderPass::~RenderPass() {
  if (mRenderPass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(mDevice->GetDevice(), mRenderPass, nullptr);
  }
}

void RenderPass::BuildRenderPass() {
  if (mSubPasses.empty() || mAttachmentDescriptions.empty() ||
      mDependencies.empty()) {
    throw std::runtime_error("Error: not enough elements to build renderPass");
  }

  // unwrap
  std::vector<VkSubpassDescription> subPasses{};
  for (int i = 0; i < mSubPasses.size(); ++i) {
    subPasses.push_back(mSubPasses[i].GetSubPassDescription());
  }

  VkRenderPassCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

  createInfo.attachmentCount =
      static_cast<uint32_t>(mAttachmentDescriptions.size());
  createInfo.pAttachments = mAttachmentDescriptions.data();

  createInfo.dependencyCount = static_cast<uint32_t>(mDependencies.size());
  createInfo.pDependencies = mDependencies.data();

  createInfo.subpassCount = static_cast<uint32_t>(subPasses.size());
  createInfo.pSubpasses = subPasses.data();

  if (vkCreateRenderPass(mDevice->GetDevice(), &createInfo, nullptr,
                         &mRenderPass) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to create renderPass");
  }
}
} // namespace VK::Wrapper