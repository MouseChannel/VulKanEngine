#pragma once

#include "../base.h"
#include "shader.hpp"

#include "renderPass.hpp"

namespace VK::Wrapper {
class Pipeline {
private:
  VkPipeline m_Pipeline{VK_NULL_HANDLE};
  VkPipelineLayout m_Layout{VK_NULL_HANDLE};
  Device::Ptr m_Device{nullptr};
  std::vector<Shader::Ptr> m_Shaders{};
  RenderPass::Ptr m_RenderPass{};

  std::vector<VkViewport> m_Viewports{};
  std::vector<VkRect2D> m_Scissors{};

public:
  using Ptr = std::shared_ptr<Pipeline>;

  static Ptr Create(const Device::Ptr &device,
                    const RenderPass::Ptr &renderPass) {
    return std::make_shared<Pipeline>(device, renderPass);
  }

  Pipeline(const Device::Ptr &device, const RenderPass::Ptr &renderPass);

  ~Pipeline();
  [[nodiscard]] auto GetPipeline() { return m_Pipeline; }
  [[nodiscard]] auto GetLayout() { return m_Layout; }
  void Make_VertexInput_Info(
      std::vector<VkVertexInputBindingDescription> &vertexbindindDes,
      std::vector<VkVertexInputAttributeDescription> &attributeDes);

  void Make_AssemblyInput_Info();

  void Make_ViewPort_Info();

  void Make_Raster_Info();

  void Make_MultiSample_Info();

  void Make_BlendAttachment_Info();

  void Make_BlendState_Info();

  void Make_DepthStecil_Info();

  void Make_LayoutCreate_Info(VkDescriptorSetLayout  & layout);

  void SetShaderGroup(const std::vector<Shader::Ptr> &shaderGroup) {
    m_Shaders = shaderGroup;
  }

  void SetViewports(const std::vector<VkViewport> &viewports) {
    m_Viewports = viewports;
  }

  void SetScissors(const std::vector<VkRect2D> &scissors) {
    m_Scissors = scissors;
  }

  void PushBlendAttachment(
      const VkPipelineColorBlendAttachmentState &blendAttachment) {
    m_BlendAttachment.push_back(blendAttachment);
  }

  void Build();

public:
  VkPipelineVertexInputStateCreateInfo m_VertexInputInfo{};
  VkPipelineInputAssemblyStateCreateInfo m_InputAssembly{};
  VkPipelineViewportStateCreateInfo m_Viewport{};
  VkPipelineRasterizationStateCreateInfo m_Rasterizer{};
  VkPipelineMultisampleStateCreateInfo m_Multisampling{};
  std::vector<VkPipelineColorBlendAttachmentState> m_BlendAttachment{};
  VkPipelineColorBlendStateCreateInfo m_BlendState{};
  VkPipelineDepthStencilStateCreateInfo m_DepthStencilState{};
  VkPipelineLayoutCreateInfo m_LayoutState{};
};

void Pipeline::Make_VertexInput_Info(
    std::vector<VkVertexInputBindingDescription> &vertexbindindDes,
    std::vector<VkVertexInputAttributeDescription> &attributeDes) {
  m_VertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  m_VertexInputInfo.vertexBindingDescriptionCount = vertexbindindDes.size();
  m_VertexInputInfo.pVertexBindingDescriptions = vertexbindindDes.data();
  m_VertexInputInfo.vertexAttributeDescriptionCount = attributeDes.size();
  m_VertexInputInfo.pVertexAttributeDescriptions = attributeDes.data();
}

void Pipeline::Make_AssemblyInput_Info() {
  m_InputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

  m_InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  m_InputAssembly.primitiveRestartEnable = VK_FALSE;
}

void Pipeline::Make_ViewPort_Info() {
  m_Viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

  m_Viewport.viewportCount = static_cast<uint32_t>(m_Viewports.size());
  m_Viewport.pViewports = m_Viewports.data();
  m_Viewport.scissorCount = static_cast<uint32_t>(m_Scissors.size());
  m_Viewport.pScissors = m_Scissors.data();
}

void Pipeline::Make_Raster_Info() {
  m_Rasterizer.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  m_Rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // ����ģʽ��Ҫ����gpu����
  m_Rasterizer.lineWidth = 1.0f; // ����1��Ҫ����gpu����
  m_Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  m_Rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  m_Rasterizer.depthBiasEnable = VK_FALSE;
  m_Rasterizer.depthBiasConstantFactor = 0.0f;
  m_Rasterizer.depthBiasClamp = 0.0f;
  m_Rasterizer.depthBiasSlopeFactor = 0.0f;
}

void Pipeline::Make_MultiSample_Info() {
  m_Multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  m_Multisampling.sampleShadingEnable = VK_FALSE;
  m_Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  m_Multisampling.minSampleShading = 1.0f;
  m_Multisampling.pSampleMask = nullptr;
  m_Multisampling.alphaToCoverageEnable = VK_FALSE;
  m_Multisampling.alphaToOneEnable = VK_FALSE;
}

void Pipeline::Make_BlendAttachment_Info() {
  VkPipelineColorBlendAttachmentState blendAttachment{};

  blendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  blendAttachment.blendEnable = VK_FALSE;
  blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

  blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

  PushBlendAttachment(blendAttachment);
}

void Pipeline::Make_BlendState_Info() {
  m_BlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  m_BlendState.logicOpEnable = VK_FALSE;
  m_BlendState.logicOp = VK_LOGIC_OP_COPY;

  // ���blendAttachment��factor��operation
  m_BlendState.blendConstants[0] = 0.0f;
  m_BlendState.blendConstants[1] = 0.0f;
  m_BlendState.blendConstants[2] = 0.0f;
  m_BlendState.blendConstants[3] = 0.0f;
  m_BlendState.attachmentCount =
      static_cast<uint32_t>(m_BlendAttachment.size());
  m_BlendState.pAttachments = m_BlendAttachment.data();
}

void Pipeline::Make_LayoutCreate_Info(VkDescriptorSetLayout  &   layout) {
  m_LayoutState.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  m_LayoutState.setLayoutCount = 1;
  m_LayoutState.pSetLayouts =  &layout;
  m_LayoutState.pushConstantRangeCount = 0;
  m_LayoutState.pPushConstantRanges = nullptr;
}

Pipeline::Pipeline(const Device::Ptr &device,
                   const RenderPass::Ptr &renderPass) {
  m_Device = device;
  m_RenderPass = renderPass;
}

Pipeline::~Pipeline() {
  if (m_Layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(m_Device->GetDevice(), m_Layout, nullptr);
  }
  if (m_Pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(m_Device->GetDevice(), m_Pipeline, nullptr);
  }
}

void Pipeline::Build() {
  std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos{};
  for (const auto &shader : m_Shaders) {

    shaderCreateInfos.push_back(shader->Make_Createinfo_in_pipeline());
  }
     std::cout << "HERE" << std::endl;
  // 设置视口裁剪
  m_Viewport.viewportCount = static_cast<uint32_t>(m_Viewports.size());
  m_Viewport.pViewports = m_Viewports.data();
  m_Viewport.scissorCount = static_cast<uint32_t>(m_Scissors.size());
  m_Viewport.pScissors = m_Scissors.data();

  if (m_Layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(m_Device->GetDevice(), m_Layout, nullptr);
  }

  if (vkCreatePipelineLayout(m_Device->GetDevice(), &m_LayoutState, nullptr,
                             &m_Layout) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to create pipeline layout");
  }

  VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

  pipelineCreateInfo.stageCount =
      static_cast<uint32_t>(shaderCreateInfos.size());
  pipelineCreateInfo.pStages = shaderCreateInfos.data();

  pipelineCreateInfo.pVertexInputState = &m_VertexInputInfo;
  pipelineCreateInfo.pInputAssemblyState = &m_InputAssembly;
  pipelineCreateInfo.pViewportState = &m_Viewport;
  pipelineCreateInfo.pRasterizationState = &m_Rasterizer;
  pipelineCreateInfo.pMultisampleState = &m_Multisampling;
  pipelineCreateInfo.pDepthStencilState = nullptr; // TODO: add depth and
  // stencil
  pipelineCreateInfo.pColorBlendState = &m_BlendState;

    pipelineCreateInfo.layout = m_Layout;
  pipelineCreateInfo.renderPass = m_RenderPass->GetRenderPass();
  pipelineCreateInfo.subpass = 0;

  // �Դ��ڵ�pipelineΪ�������д���������죬������Ҫָ��flagsΪVK_PIPELINE_CREATE_DERIVATIVE_BIT
  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = -1;

  if (m_Pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(m_Device->GetDevice(), m_Pipeline, nullptr);
  }


  // auto ss = vkCreateGraphicsPipelines(m_Device->GetDevice(), VK_NULL_HANDLE,
  // 1, &pipelineCreateInfo, nullptr, &m_Pipeline);
  if (vkCreateGraphicsPipelines(m_Device->GetDevice(), VK_NULL_HANDLE, 1,
                                &pipelineCreateInfo, nullptr,
                                &m_Pipeline) != VK_SUCCESS) {
    throw std::runtime_error("Error:failed to create pipeline");
  }

  std::cout << "HERE" << std::endl;
}

void Pipeline::Make_DepthStecil_Info() {
  m_DepthStencilState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
}

} // namespace VK::Wrapper