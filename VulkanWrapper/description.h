#pragma once
#include "buffer.hpp"
#include "../texture/texture.hpp"

namespace VK::Wrapper {

	struct UniformParameter {
		using Ptr = std::shared_ptr<UniformParameter>;
		static Ptr create() { return std::make_shared<UniformParameter>(); }

		size_t mSize{ 0 };
		//shader 当中的bindingID
		uint32_t mBinding{ 0 };

		//
		uint32_t mCount{ 0 };
		//是buffer还是image
		VkDescriptorType mDescriptorType;
		//在vertexShader中接受还是在fragmentShader里面接受
		VkShaderStageFlagBits mStage;

		std::vector<Buffer::Ptr> m_Buffers{};
		Texture::Ptr mTexture{ nullptr };
	};

} // namespace VK::Wrapper
