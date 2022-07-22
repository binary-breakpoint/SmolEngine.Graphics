#pragma once
#include "Common/Gfx_Helpers.h"
#include "Backend/Gfx_VulkanCore.h"

namespace Dia
{
	class Gfx_CmdBuffer;
	struct FramebufferCreateDesc;

	struct ImageLayoutTransitionDesc
	{
		VkImage                 Image;
		VkCommandBuffer         CmdBuffer;
		VkImageLayout           OldImageLayout;
		VkImageLayout           NewImageLayout;
		VkImageSubresourceRange SubresourceRange;
		VkPipelineStageFlags    SrcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		VkPipelineStageFlags    DstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	};

	class Gfx_VulkanHelpers
	{
	public:
		static void* AlignedAlloc(size_t size, size_t alignment);
		static void AlignedFree(void* data);

		static uint64_t GetBufferDeviceAddress(VkBuffer buffer);
		static uint32_t GetAlignedSize(uint32_t value, uint32_t alignment);
		static VkFormat GetFormat(Format format);
		static VkSamplerAddressMode GetAddressMode(AddressMode mode);
		static VkSamplerMipmapMode GetMipmapMode(FilterMode mode);
		static VkFilter GetFilter(FilterMode fiter);
		static VkBorderColor GetBorderColor(BorderColor color);
		static VkAttachmentStoreOp GetStoreOp(StoreOp op);
		static VkAttachmentLoadOp GetLoadOp(LoadOp op);
		static VkShaderStageFlagBits GetShaderStage(ShaderStage stage);
		static VkPrimitiveTopology GetTopology(DrawMode mode);
		static VkCullModeFlags GetCullMode(CullMode mode);
		static VkPolygonMode GetPolygonMode(PolygonMode mode);
		static VkBlendFactor GetBlendFactor(BlendFactor factor);
		static VkBlendOp GetBlendOp(BlendOp op);

		static bool IsDepthFormat(Format format);
		static void ClearImage(class Gfx_PixelStorage* storage, const glm::vec4& color, VkImageLayout layout, 
			VkCommandBuffer cmdbuffer,  VkImageSubresourceRange* pRange = nullptr);

		static void CopyDataToImage(class Gfx_PixelStorage* storage, void* data);
		static void InitializeImageResource(class Gfx_PixelStorage* storage, VkImageLayout layout);
		static void CopyPixelStorageToSwapchain(uint32_t width, uint32_t height, Gfx_CmdBuffer* cmd, Gfx_PixelStorage* storage);
		static bool IsFormatIsFilterable(VkFormat format, VkImageTiling tiling);

		static void InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, 
			VkImageSubresourceRange subresourceRange);

		static void GenerateMipMaps(VkImage image, VkCommandBuffer cmd, int32_t width, int32_t height, int32_t mipLevel, 
			VkImageSubresourceRange& range);

		static void CreateVkRenderPass(FramebufferCreateDesc* fbDesc, VkRenderPass& outVkPass);

		static void SetImageLayout(const ImageLayoutTransitionDesc& desc);
		static void ExecuteCmdBuffer(Gfx_CmdBuffer* cmd);
	};
}