#include "Gfx_Precompiled.h"
#include "Common/Gfx_PixelStorage.h"
#include "Backend/Gfx_VulkanHelpers.h"

namespace SmolEngine
{
	Gfx_PixelStorage::Gfx_PixelStorage() :
		m_Alloc{nullptr},
		m_Image{nullptr},
		m_ImageView{nullptr} 
	{

	}

	Gfx_PixelStorage::~Gfx_PixelStorage()
	{
		Free();
	}

	void Gfx_PixelStorage::Create(const PixelStorageCreateDesc* desc)
	{
		m_Desc = *desc;

		m_Desc.myMipLevels == 0 ? static_cast<uint32_t>(floor(log2(std::max(m_Desc.myWidth, m_Desc.myHeight)))) + 1 : m_Desc.myMipLevels;

		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = Gfx_VulkanHelpers::GetFormat(m_Desc.myFormat);
		imageCI.mipLevels = m_Desc.myMipLevels;
		imageCI.extent.depth = 1;
		imageCI.arrayLayers = m_Desc.myArrayLayers;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.extent = { m_Desc.myWidth, m_Desc.myHeight, 1 };
		imageCI.usage = m_Desc.myUsageFlags;
		imageCI.flags = m_Desc.myCreateFlags;

		m_Alloc = Gfx_VulkanAllocator::AllocImage(imageCI, VMA_MEMORY_USAGE_GPU_ONLY, m_Image);

		VkImageViewCreateInfo imageViewCI = {};
		imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		switch (imageCI.flags)
		{
		case VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT:
		{
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			break;
		}
		case VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT:
		{
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			break;
		}
		default:
		{
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			break;
		}
		}

		imageViewCI.format = imageCI.format;
		imageViewCI.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		imageViewCI.subresourceRange.aspectMask = m_Desc.myAspectMask;
		imageViewCI.subresourceRange.baseMipLevel = 0;
		imageViewCI.subresourceRange.baseArrayLayer = 0;
		imageViewCI.subresourceRange.layerCount = imageCI.arrayLayers;
		imageViewCI.subresourceRange.levelCount = m_Desc.myMipLevels;
		imageViewCI.image = m_Image;

		VK_CHECK_RESULT(vkCreateImageView(Gfx_Context::GetDevice().GetLogicalDevice(), &imageViewCI, nullptr, &m_ImageView));
	}

	void Gfx_PixelStorage::SetImageLayout(VkImageLayout layout)
	{
		m_Desc.myLayout = layout;
	}

	void Gfx_PixelStorage::Free()
	{
		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();

		if (m_ImageView != nullptr && m_Image != nullptr)
		{
			vkDestroyImageView(device, m_ImageView, nullptr);
			Gfx_VulkanAllocator::FreeImage(m_Image, m_Alloc);

			m_Alloc = nullptr;
			m_Image = nullptr;
			m_ImageView = nullptr;
		}
	}

	bool Gfx_PixelStorage::IsGood() const
	{
		return m_Alloc != nullptr;
	}

}