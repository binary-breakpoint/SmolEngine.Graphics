#include "Gfx_Precompiled.h"
#include "Common/Gfx_Texture.h"
#include "Backend/Gfx_VulkanHelpers.h"
#include "Gfx_RenderContext.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <imgui/backends/imgui_impl_vulkan.h>

namespace SmolEngine
{
	Gfx_Texture::Gfx_Texture()
		:
		m_ImguiHandle{nullptr} {}

	Gfx_Texture::~Gfx_Texture()
	{
		Free();
	}

	void Gfx_Texture::Create(TextureCreateDesc* info)
	{
		assert(info != nullptr);

		if (!info->myFilePath.empty())
		{
			int height, width, channels;

			stbi_set_flip_vertically_on_load(1);

			stbi_uc* data = nullptr;
			{
				data = stbi_load(info->myFilePath.c_str(), &width, &height, &channels, 4);
				GFX_ASSERT_MSG(data, "VulkanTexture:: Texture not found!")

				info->mySize.x = width;
				info->mySize.y = height;

				LoadEX(info, data);
			}

			stbi_image_free(data);
			return;
		}

		LoadEX(info, info->myUserData);
	}

	void Gfx_Texture::Free()
	{
		m_PixelStorage->Free();

		for (auto& [key, view] : m_ImageViewMap)
		{
			VK_DESTROY_DEVICE_HANDLE(view, vkDestroyImageView);
		}
	}

	const VkDescriptorImageInfo& Gfx_Texture::GetDescriptorImageInfo() const
	{
		return m_DescriptorImageInfo;
	}

	std::pair<uint32_t, uint32_t> Gfx_Texture::GetMipSize(uint32_t mip) const
	{
		uint32_t width = m_Desc.mySize.x;
		uint32_t height = m_Desc.mySize.y;
		while (mip != 0)
		{
			width /= 2;
			height /= 2;
			mip--;
		}

		return { width, height };
	}

	VkDescriptorImageInfo Gfx_Texture::GetMipImageView(uint32_t mip)
	{
		if (m_ImageViewMap.find(mip) == m_ImageViewMap.end())
		{
			VkImageViewCreateInfo imageViewCreateInfo = {};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = Gfx_VulkanHelpers::GetFormat(m_Desc.myFormat);
			imageViewCreateInfo.flags = 0;
			imageViewCreateInfo.subresourceRange = {};
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = mip;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;
			imageViewCreateInfo.image = m_PixelStorage->GetImage();

			VkImageView pView = nullptr;
			VK_CHECK_RESULT(vkCreateImageView(Gfx_App::GetDevice().GetLogicalDevice(), &imageViewCreateInfo, nullptr, &pView));
			m_ImageViewMap[mip] = pView;
		}

		VkDescriptorImageInfo imageinfo{};
		imageinfo.imageLayout = m_DescriptorImageInfo.imageLayout;
		imageinfo.sampler = m_DescriptorImageInfo.sampler;
		imageinfo.imageView = m_ImageViewMap[mip];
		return imageinfo;
	}

	Ref<Gfx_PixelStorage> Gfx_Texture::GetPixelStorage()
	{
		return m_PixelStorage;
	}

	TextureUsage Gfx_Texture::GetUsageFlags() const
	{
		return m_Desc.myUsage;
	}

	void* Gfx_Texture::GetImGuiTexture() const
	{
		return m_ImguiHandle;
	}

	bool Gfx_Texture::IsGood() const
	{
		return m_Desc.mySize.x > 0 && m_Desc.mySize.y > 0;
	}

	uint32_t Gfx_Texture::GetMips() const
	{
		return m_Desc.myMipLevels;
	}

	void Gfx_Texture::LoadEX(TextureCreateDesc* info, void* data)
	{
		GFX_ASSERT(info);
		GFX_ASSERT(info->mySize.x > 0 && info->mySize.y > 0)

		if (info->mySampler == nullptr)
			info->mySampler = Gfx_RenderContext::GetDefaultSampler();

		m_Desc = *info;

		PixelStorageCreateDesc pixelDesc{};

		pixelDesc.myLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		pixelDesc.myArrayLayers = info->myArrayLayers;
		pixelDesc.myFormat = info->myFormat;
		pixelDesc.mySize = info->mySize;
		pixelDesc.myMipLevels = info->myMipLevels;
		pixelDesc.myUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		if (info->myUsage == TextureUsage::ARRAY)
		{
			GFX_ASSERT(info->myArrayLayers > 1)
		}

		if (info->myUsage == TextureUsage::CUBEMAP)
		{
			GFX_ASSERT(info->myArrayLayers == 6)
			pixelDesc.myCreateFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		}

		if (info->myIsShaderWritable)
		{
			pixelDesc.myUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		}

		m_PixelStorage = Gfx_RenderContext::CreatePixelStorage(pixelDesc);

		VkImageLayout finalLayout = info->myIsShaderWritable ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (data != nullptr)
			Gfx_VulkanHelpers::CopyDataToImage(m_PixelStorage.get(), data, finalLayout);
		else
			Gfx_VulkanHelpers::InitializeImageResource(m_PixelStorage.get(), finalLayout);

		m_DescriptorImageInfo = {};
		m_DescriptorImageInfo.imageLayout = m_PixelStorage->GetImageLayout();
		m_DescriptorImageInfo.imageView = m_PixelStorage->GetImageView();
		m_DescriptorImageInfo.sampler = info->mySampler->GetSampler();

		if(info->myImGUIHandleEnable)
			m_ImguiHandle = ImGui_ImplVulkan_AddTexture(m_DescriptorImageInfo.sampler,
				m_DescriptorImageInfo.imageView, m_DescriptorImageInfo.imageLayout);
	}

}