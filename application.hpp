//
// Created by mocheng on 2022/10/21.
//
#pragma once
#include "VulkanWrapper/Instance.hpp"
#include "VulkanWrapper/commandBuffer.hpp"
#include "VulkanWrapper/descriptorSetLayout.hpp"
#include "VulkanWrapper/device.hpp"
#include "VulkanWrapper/pipeline.hpp"
#include "VulkanWrapper/renderPass.hpp"
#include "VulkanWrapper/sampler.hpp"
#include "VulkanWrapper/semaphore.hpp"
#include "VulkanWrapper/swapChain.hpp"
#include "VulkanWrapper/windowSurface.hpp"
#include "header/glfw3.h"
#include "vulkan/vulkan_core.h"

#include "VulkanWrapper/fence.hpp"
#include "model.hpp"
#include "texture/texture.hpp"
#include "uniformManager.hpp"
#include <vector>

namespace VK {

class Application {
private:
  void InitWindow();
  void InitVulkan();

  void MainLoop();

  void CleanUp();

private:
  unsigned int m_Width{800};
  unsigned int m_Height{600};
  Wrapper::UniformManager::Ptr m_UniformManager{nullptr};
  Wrapper::Window::Ptr m_Window{nullptr};
  Wrapper::WindowSurface::Ptr m_Surface{nullptr};

  Wrapper::Instance::Ptr m_Instance{nullptr};
  Wrapper::Device::Ptr m_Device{nullptr};
  Wrapper::SwapChain::Ptr m_SwapChain{nullptr};
  Wrapper::RenderPass::Ptr m_RenderPass{nullptr};
  Wrapper::Pipeline::Ptr m_Pipeline{nullptr};
  Wrapper::CommandPool::Ptr m_CommandPool{nullptr};
  Wrapper::DescriptorSetLayout::Ptr m_DescriptorSetLayout{nullptr};
  Model::Ptr m_Model{nullptr};
  std::vector<Wrapper::CommandBuffer::Ptr> m_CommandBuffers{};
  std::vector<Wrapper::Semaphore::Ptr> m_ImageAvailableSemaphores{};
  std::vector<Wrapper::Semaphore::Ptr> m_RenderFinishedSemaphores{};
  std::vector<Wrapper::Fence::Ptr> m_Fences{};
  VPMatrices m_VPMatrices;
  int m_CurrentFrame{0};

public:
  Application() = default;

  ~Application() = default;

  void Run();
  void CreatePipeline();
  void CreateRenderPass();
  void CreateCommandBuffer();
  void CreateSyncObjects();
  void ReCreateSwapChain();
  void CleanupSwapChain();

  void Render();
};

void Application::InitWindow() {
  m_Window = Wrapper::Window::Create(m_Width, m_Height);
}

void Application::MainLoop() {
  while (!m_Window->ShouldClose()) {
    m_Window->PollEvent();
    m_Model->update();
    m_UniformManager->Update(m_VPMatrices, m_Model->getUniform(),
                             m_CurrentFrame);

    Render();
  }
  vkDeviceWaitIdle(m_Device->GetDevice());
}
void Application::CleanUp() {
  m_Pipeline.reset();
  m_RenderPass.reset();
  m_SwapChain.reset();
  m_Device.reset();
  m_Surface.reset();
  m_Window.reset();
  m_Instance.reset();
  VkPhysicalDeviceMemoryProperties a;
  VkMemoryRequirements l;
}

void Application::InitVulkan() {
  m_Instance = Wrapper::Instance::Create();
  m_Surface = Wrapper::WindowSurface::Create(m_Instance, m_Window);
  m_Device = Wrapper::Device::Create(m_Instance, m_Surface);
  m_Model = Model::Create(m_Device);
  m_SwapChain = Wrapper::SwapChain::Create(m_Device, m_Window, m_Surface);
  m_RenderPass = Wrapper::RenderPass::Create(m_Device);
  CreateRenderPass();
  m_SwapChain->CreateFrameBuffers(m_RenderPass);

  m_Width = m_SwapChain->GetExtent().width;
  m_Height = m_SwapChain->GetExtent().height;

  m_CommandPool = Wrapper::CommandPool::Create(m_Device);

  // descriptor ============
  m_UniformManager = Wrapper::UniformManager::Create();
  m_UniformManager->Init(m_Device, m_CommandPool, m_SwapChain->GetImageCount());
  m_Pipeline = Wrapper::Pipeline::Create(m_Device, m_RenderPass);
  CreatePipeline();
  m_CommandBuffers.resize(m_SwapChain->GetImageCount());
  CreateCommandBuffer();
  CreateSyncObjects();
  ReCreateSwapChain();
}
// commandBuffer 创建时会直接录制所有信息，后面可以独立运行
void Application::CreateCommandBuffer() {

  for (int i = 0; i < m_SwapChain->GetImageCount(); ++i) {
    m_CommandBuffers[i] =
        Wrapper::CommandBuffer::Create(m_Device, m_CommandPool);
    m_CommandBuffers[i]->Begin();
    VkRenderPassBeginInfo renderBeginInfo{};
    renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderBeginInfo.renderPass = m_RenderPass->GetRenderPass();
    renderBeginInfo.framebuffer = m_SwapChain->GetFrameBuffer(i);
    renderBeginInfo.renderArea.offset = {0, 0};
    renderBeginInfo.renderArea.extent = m_SwapChain->GetExtent();
    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderBeginInfo.clearValueCount = 1;
    renderBeginInfo.pClearValues = &clearColor;

    m_CommandBuffers[i]->BeginRenderPass(renderBeginInfo);

    m_CommandBuffers[i]->BindGraphicPipeline(m_Pipeline->GetPipeline());
    m_CommandBuffers[i]->BindDescriptorSet(
        m_Pipeline->GetLayout(),
        m_UniformManager->GetDescriptorSet(m_CurrentFrame));

    m_CommandBuffers[i]->BindVertexBuffer(m_Model->getVertexBuffers());
    m_CommandBuffers[i]->BindIndexBuffer(
        m_Model->getIndexBuffer()->getBuffer());
    // m_CommandBuffers[i]->Draw(3);
    m_CommandBuffers[i]->DrawIndex(m_Model->getIndexCount());

    m_CommandBuffers[i]->EndRenderPass();

    m_CommandBuffers[i]->End();
  }
}
void Application::CreateSyncObjects() {
  for (int i = 0; i < m_SwapChain->GetImageCount(); ++i) {
    auto imageSemaphore = Wrapper::Semaphore::Create(m_Device);
    m_ImageAvailableSemaphores.push_back(imageSemaphore);

    auto renderSemaphore = Wrapper::Semaphore::Create(m_Device);
    m_RenderFinishedSemaphores.push_back(renderSemaphore);
    auto fence = Wrapper::Fence::Create(m_Device);
    m_Fences.push_back(fence);
  }
}

void Application::Render() {

 
  uint32_t imageIndex{0};

  // 显示完后点亮m_ImageAvailableSemaphores[m_CurrentFrame]，同时该图片供下一次渲染使用
  // 此处的imageIndex 为SwapChain的m_SwapChainImages索引
  // 此时的imageIndex 已经被显示完了，等待渲染
  auto result = vkAcquireNextImageKHR(
      m_Device->GetDevice(), m_SwapChain->GetSwapChain(), UINT64_MAX,
      m_ImageAvailableSemaphores[m_CurrentFrame]->GetSemaphore(),
      VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    ReCreateSwapChain();
    m_Window->m_WindowResized = false;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("Error: failed to acquire next image");
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  // 设置同步信息
  VkSemaphore waitSemaphores[] = {
      m_ImageAvailableSemaphores[m_CurrentFrame]->GetSemaphore()};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  // 提交哪些命令
  auto commandBuffer = m_CommandBuffers[imageIndex]->GetCommandBuffer();
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  VkSemaphore signalSemaphores[] = {
      m_RenderFinishedSemaphores[m_CurrentFrame]->GetSemaphore()};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;
  // 等待该槽位的上一个commandBuffer执行完毕

  m_Fences[m_CurrentFrame]->Block();
  // 阻塞住
  m_Fences[m_CurrentFrame]->ResetFence();
  if (vkQueueSubmit(m_Device->GetGraphicQueue(), 1, &submitInfo,
                    m_Fences[m_CurrentFrame]->GetFence()) != VK_SUCCESS) {
    throw std::runtime_error("Fail");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {m_SwapChain->GetSwapChain()};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(m_Device->GetPresentQueue(), &presentInfo);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      m_Window->m_WindowResized) {
    ReCreateSwapChain();
    m_Window->m_WindowResized = false;
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to present");
  }
  m_CurrentFrame = (m_CurrentFrame + 1) % m_SwapChain->GetImageCount();
}
void Application::Run() {

  InitWindow();
  InitVulkan();

  MainLoop();
  CleanUp();
}
void Application::CreatePipeline() {
  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)m_Width;
  viewport.height = (float)m_Height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = {m_Width, m_Height};
  m_Pipeline->SetViewports({viewport});
  m_Pipeline->SetScissors({scissor});
  std::vector<Wrapper::Shader::Ptr> shaderGroup{};

  auto shaderVertex =
      Wrapper::Shader::Create(m_Device, "D:\\cpp\\vk\\shaders/vs.spv",
                              VK_SHADER_STAGE_VERTEX_BIT, "main");
  shaderGroup.push_back(shaderVertex);

  auto shaderFragment =
      Wrapper::Shader::Create(m_Device, "D:\\cpp\\vk\\shaders/fs.spv",
                              VK_SHADER_STAGE_FRAGMENT_BIT, "main");
  shaderGroup.push_back(shaderFragment);

  auto vertexBindingDes = m_Model->getVertexInputBindingDescriptions();
  auto attributeDes = m_Model->getAttributeDescriptions();
  // 将shader加入到pipeline里面
  m_Pipeline->SetShaderGroup(shaderGroup);

  m_Pipeline->Make_ViewPort_Info();
  // 顶点设置
  m_Pipeline->Make_VertexInput_Info(vertexBindingDes, attributeDes);
  // 图元装配
  m_Pipeline->Make_AssemblyInput_Info();
  // 光栅化设置
  m_Pipeline->Make_Raster_Info();
  // 多重采样
  m_Pipeline->Make_MultiSample_Info();
  // TODO深度与模板

  // 颜色混合
  m_Pipeline->Make_BlendAttachment_Info();

  m_Pipeline->Make_BlendState_Info();

  // m_Pipeline->m_LayoutState.sType =
  //     VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  // m_Pipeline->m_LayoutState.setLayoutCount = 1;
  // m_Pipeline->m_LayoutState.pSetLayouts =
  //     &m_UniformManager->GetDescriptorLayout()->GetLayout();
  // m_Pipeline->m_LayoutState.pushConstantRangeCount = 0;
  // m_Pipeline->m_LayoutState.pPushConstantRanges = nullptr;
  auto pipelineLayout = m_UniformManager->GetDescriptorLayout()->GetLayout();
  m_Pipeline->Make_LayoutCreate_Info(pipelineLayout);
  m_Pipeline->Build();
}

void Application::CreateRenderPass() {

  VkAttachmentDescription attachmentDes{};
  attachmentDes.format = m_SwapChain->GetFormat();
  attachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
  attachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachmentDes.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  m_RenderPass->AddAttachment(attachmentDes);

  VkAttachmentReference attachmentRef{};
  attachmentRef.attachment = 0;
  attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // 做一个subpass
  Wrapper::SubPass subPass{};
  subPass.AddColorAttachmentReference(attachmentRef);
  subPass.BuildSubPassDescription();

  m_RenderPass->AddSubPass(subPass);
  // 创建依赖关系，这里需要按流程创建一个虚拟阶段
  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  // 上一个pass在哪一个stage的哪一个操作完成
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  // P一个pass在哪一个stage的哪一个操作阻塞
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                             VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  m_RenderPass->AddDependency(dependency);

  m_RenderPass->BuildRenderPass();
}

void Application::ReCreateSwapChain() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(m_Window->GetWindow(), &width, &height);
  while (width == 0 || height == 0) {
    glfwWaitEvents();
    glfwGetFramebufferSize(m_Window->GetWindow(), &width, &height);
  }

  vkDeviceWaitIdle(m_Device->GetDevice());

  CleanupSwapChain();

  m_SwapChain = Wrapper::SwapChain::Create(m_Device, m_Window, m_Surface);
  m_Width = m_SwapChain->GetExtent().width;
  m_Height = m_SwapChain->GetExtent().height;

  m_RenderPass = Wrapper::RenderPass::Create(m_Device);
  CreateRenderPass();

  m_SwapChain->CreateFrameBuffers(m_RenderPass);

  m_Pipeline = Wrapper::Pipeline::Create(m_Device, m_RenderPass);
  CreatePipeline();

  m_CommandBuffers.resize(m_SwapChain->GetImageCount());

  CreateCommandBuffer();

  CreateSyncObjects();
}

void Application::CleanupSwapChain() {
  m_SwapChain.reset();
  m_CommandBuffers.clear();
  m_Pipeline.reset();
  m_RenderPass.reset();
  m_ImageAvailableSemaphores.clear();
  m_RenderFinishedSemaphores.clear();
  m_Fences.clear();
}
} // namespace VK