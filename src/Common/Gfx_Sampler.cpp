#include "Gfx_Precompiled.h"
#include "Common/Gfx_Sampler.h"
#include "Backend/Gfx_VulkanHelpers.h"

namespace SmolEngine
{
	Gfx_Sampler::Gfx_Sampler() : 
		m_Sampler{nullptr} {}

	Gfx_Sampler::~Gfx_Sampler()
	{
		Free();
	}

	void Gfx_Sampler::Create(SamplerCreateDesc* desc)
	{
		VkSamplerCreateInfo samplerCI = {};

		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.magFilter = Gfx_VulkanHelpers::GetFilter(desc->myFilterMode);
		samplerCI.minFilter = samplerCI.magFilter;
		samplerCI.mipmapMode = Gfx_VulkanHelpers::GetMipmapMode(desc->myMipmapMode);
		samplerCI.addressModeU = Gfx_VulkanHelpers::GetAddressMode(desc->myAddressMode);
		samplerCI.addressModeV = samplerCI.addressModeU;
		samplerCI.addressModeW = samplerCI.addressModeU;
		samplerCI.compareOp = VK_COMPARE_OP_NEVER;
		samplerCI.mipLodBias = desc->myLoadBias;
		samplerCI.minLod = desc->myMinLoad;
		samplerCI.maxLod = desc->myMaxLoad;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.borderColor = Gfx_VulkanHelpers::GetBorderColor(desc->myBorderColor);
		samplerCI.unnormalizedCoordinates = VK_FALSE;
		samplerCI.compareEnable = desc->myCompareEnable;

		auto& device = Gfx_Context::GetDevice();
		if (device.GetDeviceFeatures()->samplerAnisotropy && desc->myAnisotropyEnable)
		{
			samplerCI.maxAnisotropy = device.GetDeviceProperties()->limits.maxSamplerAnisotropy;
			samplerCI.anisotropyEnable = VK_TRUE;
		}

		VK_CHECK_RESULT(vkCreateSampler(device.GetLogicalDevice(), &samplerCI, nullptr, &m_Sampler));
	}

	void Gfx_Sampler::Free()
	{
		VK_DESTROY_DEVICE_HANDLE(m_Sampler, vkDestroySampler);
	}

}