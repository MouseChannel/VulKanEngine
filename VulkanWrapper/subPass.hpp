#pragma once
#include "../base.h"
#include "vulkan/vulkan_core.h"
#include <vector>

namespace VK::Wrapper {

	class SubPass {
	private:
		VkSubpassDescription m_SubPassDescription{};
		//AttachmentReference代表的是在RenderPassAttachments数组里面的下标
		//该pass的输出
		std::vector<VkAttachmentReference> m_ColorAttachmentReferences{};
		//上pass的输出，即该pass的输入
		std::vector<VkAttachmentReference> m_InputAttachmentReferences{};
		VkAttachmentReference m_DepthStencilAttachmentReference{};

	public:
		SubPass();
		~SubPass();
		void AddColorAttachmentReference(const VkAttachmentReference& ref);

		void AddInputAttachmentReference(const VkAttachmentReference& ref);

		void SetDepthStencilAttachmentReference(const VkAttachmentReference& ref);

		void BuildSubPassDescription();

		[[nodiscard]] auto GetSubPassDescription() const {
			return m_SubPassDescription;
		}
	};
	SubPass::SubPass() {

	}

	SubPass::~SubPass() {

	}

	void SubPass::AddColorAttachmentReference(const VkAttachmentReference& ref) {
		m_ColorAttachmentReferences.push_back(ref);
	}

	void SubPass::AddInputAttachmentReference(const VkAttachmentReference& ref) {
		m_InputAttachmentReferences.push_back(ref);
	}

	void SubPass::SetDepthStencilAttachmentReference(const VkAttachmentReference& ref) {
		m_DepthStencilAttachmentReference = ref;
	}

	void SubPass::BuildSubPassDescription() {
		if (m_ColorAttachmentReferences.empty()) {
			throw std::runtime_error("Error: color attachment group is empty!");
		}
		m_SubPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		m_SubPassDescription.colorAttachmentCount = static_cast<uint32_t>(m_ColorAttachmentReferences.size());
		m_SubPassDescription.pColorAttachments = m_ColorAttachmentReferences.data();

		m_SubPassDescription.inputAttachmentCount = static_cast<uint32_t>(m_InputAttachmentReferences.size());
		m_SubPassDescription.pInputAttachments = m_InputAttachmentReferences.data();


		m_SubPassDescription.pDepthStencilAttachment = m_DepthStencilAttachmentReference.layout == VK_IMAGE_LAYOUT_UNDEFINED ? nullptr : &m_DepthStencilAttachmentReference;
	}



}