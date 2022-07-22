#pragma once
#include "Backend/Gfx_VulkanAllocator.h"
#include "Common/Gfx_Flags.h"
#include "Common/Gfx_Asset.h"

namespace cereal
{
	class access;
}

namespace Dia
{
	struct PixelStorageCreateDesc
	{
		uint32_t myWidth = 0;
		uint32_t myHeight = 0;
		uint32_t myMipLevels = 1;
		uint32_t myArrayLayers = 1;
		Format myFormat = Format::R8G8B8A8_UNORM;
		VkImageAspectFlags myAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VkImageUsageFlags myUsageFlags;
		VkImageCreateFlags myCreateFlags;
		VkImageLayout myLayout;
	private:
		friend class cereal::access;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(myWidth, myHeight, myMipLevels, myArrayLayers, myFormat);
		}
	};

	class Gfx_PixelStorage final: public Gfx_Asset
	{
		friend class Gfx_VulkanHelpers;
	public:
		Gfx_PixelStorage();
		~Gfx_PixelStorage();

		void Free() override;
		void Create(const PixelStorageCreateDesc* desc);
		void SetImageLayout(VkImageLayout layout);
		bool IsGood() const override;

		const PixelStorageCreateDesc& GetDesc() const { return m_Desc; }
		VkImage GetImage() const { return m_Image; }
		VkImageView GetImageView() const { return m_ImageView; }
		VmaAllocation GetVmaAlloc() { return m_Alloc; }
		VkImageLayout GetImageLayout() { return m_Desc.myLayout; }

	private:
		VkImage m_Image;
		VmaAllocation m_Alloc;
		VkImageView m_ImageView;
		PixelStorageCreateDesc m_Desc;
	};
}