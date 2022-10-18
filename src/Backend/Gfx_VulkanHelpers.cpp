#include "Gfx_Precompiled.h"
#include "Backend/Gfx_VulkanHelpers.h"

#include "Common/Gfx_Buffer.h"
#include "Common/Gfx_CmdBuffer.h"
#include "Common/Gfx_PixelStorage.h"
#include "Common/Gfx_Framebuffer.h"

#include <mutex>

namespace SmolEngine
{
	std::mutex* s_locVulkanHelpersMutex = new std::mutex();

    void* Gfx_VulkanHelpers::AlignedAlloc(size_t size, size_t alignment)
    {
		void* data = nullptr;
#if defined(_MSC_VER) || defined(__MINGW32__)
		data = _aligned_malloc(size, alignment);
#else
		int res = posix_memalign(&data, alignment, size);
		if (res != 0)
			data = nullptr;
#endif
		return data;
    }

    void Gfx_VulkanHelpers::AlignedFree(void* data)
    {
#if	defined(_MSC_VER) || defined(__MINGW32__)
		_aligned_free(data);
#else
		free(data);
#endif
    }

	uint64_t Gfx_VulkanHelpers::GetBufferDeviceAddress(VkBuffer buffer)
	{
		auto& device = Gfx_Context::GetDevice();

		VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
		bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferDeviceAI.buffer = buffer;

		return device.vkGetBufferDeviceAddressKHR(device.GetLogicalDevice(), &bufferDeviceAI);
	}

	uint32_t Gfx_VulkanHelpers::GetAlignedSize(uint32_t value, uint32_t alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}


	VkFormat Gfx_VulkanHelpers::GetFormat(Format format)
	{
		switch (format)
		{
		case Format::R8_UNORM:
			return VK_FORMAT_R8_UNORM;
		case Format::R8G8_UNORM:
			return VK_FORMAT_R8G8_UNORM;
		case Format::R8G8B8A8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case Format::B8G8R8A8_UNORM:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case Format::D16_UNORM:
			return VK_FORMAT_D16_UNORM;
		case Format::D16_UNORM_S8_UINT:
			return VK_FORMAT_D16_UNORM_S8_UINT;
		case Format::D32_SFLOAT_S8_UINT:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;
		case Format::D32_SFLOAT:
			return VK_FORMAT_D32_SFLOAT;
		case Format::R8_SNORM:
			return VK_FORMAT_R8_SNORM;
		case Format::R8G8_SNORM:
			return VK_FORMAT_R8G8_SNORM;
		case Format::R8G8B8A8_SNORM:
			return VK_FORMAT_R8G8B8A8_SNORM;
		case Format::R16_UNORM:
			return VK_FORMAT_R16_UNORM;
		case Format::R16G16_UNORM:
			return VK_FORMAT_R16G16_UNORM;
		case Format::R16G16B16A16_UNORM:
			return VK_FORMAT_R16G16B16A16_UNORM;
		case Format::R16_SNORM:
			return VK_FORMAT_R16_SNORM;
		case Format::R16G16_SNORM:
			return VK_FORMAT_R16G16_SNORM;
		case Format::R16G16B16A16_SNORM:
			return VK_FORMAT_R16G16B16A16_SNORM;
		case Format::R16_SFLOAT:
			return VK_FORMAT_R16_SFLOAT;
		case Format::R16G16_SFLOAT:
			return VK_FORMAT_R16G16_SFLOAT;
		case Format::R16G16B16A16_SFLOAT:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Format::R32_SFLOAT:
			return VK_FORMAT_R32_SFLOAT;
		case Format::R32G32_SFLOAT:
			return VK_FORMAT_R32G32_SFLOAT;
		case Format::R32G32B32A32_SFLOAT:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case Format::R16G16B16_SFLOAT:
			return VK_FORMAT_R16G16B16_SFLOAT;
		case Format::R32G32B32_SFLOAT:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case Format::R16_UINT:
			return VK_FORMAT_R16_UINT;
		case Format::R16G16_UINT:
			return VK_FORMAT_R16G16_UINT;
		case Format::R16G16B16A16_UINT:
			return VK_FORMAT_R16G16B16A16_UINT;
		case Format::R32_UINT:
			return VK_FORMAT_R32_UINT;
		case Format::R32G32_UINT:
			return VK_FORMAT_R32G32_UINT;
		case Format::R32G32B32A32_UINT:
			return VK_FORMAT_R32G32B32A32_UINT;
		case Format::R16_SINT:
			return VK_FORMAT_R16_SINT;
		case Format::R16G16_SINT:
			return VK_FORMAT_R16G16_SINT;
		case Format::R16G16B16A16_SINT:
			return VK_FORMAT_R16G16B16A16_SINT;
		case Format::R32_SINT:
			return VK_FORMAT_R32_SINT;
		case Format::R32G32_SINT:
			return VK_FORMAT_R32G32_SINT;
		case Format::R32G32B32A32_SINT:
			return VK_FORMAT_R32G32B32A32_SINT;
		default:
			return VK_FORMAT_R8_UNORM;
		}
	}

	VkSamplerAddressMode Gfx_VulkanHelpers::GetAddressMode(AddressMode mode)
	{
		switch (mode)
		{
		case AddressMode::CLAMP_TO_BORDER:                   return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case AddressMode::CLAMP_TO_EDGE:                     return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case AddressMode::MIRRORED_REPEAT:                   return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case AddressMode::MIRROR_CLAMP_TO_EDGE:              return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		case AddressMode::REPEAT:                            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}

		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	VkSamplerMipmapMode Gfx_VulkanHelpers::GetMipmapMode(FilterMode mode)
	{
		switch (mode)
		{
		case FilterMode::NEAREST:                       return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case FilterMode::LINEAR:                        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}

		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	VkFilter Gfx_VulkanHelpers::GetFilter(FilterMode fiter)
	{
		switch (fiter)
		{
		case FilterMode::NEAREST:                       return VK_FILTER_NEAREST;
		case FilterMode::LINEAR:                        return VK_FILTER_LINEAR;
		}

		return VK_FILTER_NEAREST;
	}

	VkBorderColor Gfx_VulkanHelpers::GetBorderColor(BorderColor color)
	{
		switch (color)
		{
		case BorderColor::FLOAT_OPAQUE_BLACK:           return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		case BorderColor::FLOAT_OPAQUE_WHITE:           return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		case BorderColor::FLOAT_TRANSPARENT_BLACK:      return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		case BorderColor::INT_OPAQUE_BLACK:             return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		case BorderColor::INT_OPAQUE_WHITE:             return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
		case BorderColor::INT_TRANSPARENT_BLACK:        return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
		}

		return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	}

	VkAttachmentStoreOp Gfx_VulkanHelpers::GetStoreOp(StoreOp op)
	{
		switch (op)
		{
		case StoreOp::STORE_OP_STORE:
			return VK_ATTACHMENT_STORE_OP_STORE;
		case StoreOp::STORE_OP_DONT_CARE:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		case StoreOp::STORE_OP_NONE_KHR:
			return VK_ATTACHMENT_STORE_OP_NONE_KHR;
		default:
			return VK_ATTACHMENT_STORE_OP_STORE;
		}
	}

	VkAttachmentLoadOp Gfx_VulkanHelpers::GetLoadOp(LoadOp op)
	{
		switch (op)
		{
		case LoadOp::LOAD_OP_LOAD:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		case LoadOp::LOAD_OP_CLEAR:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case LoadOp::LOAD_OP_DONT_CARE:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		default:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		}
	}

	VkShaderStageFlagBits Gfx_VulkanHelpers::GetShaderStage(ShaderStage stage)
	{
		VkShaderStageFlags flags = 0;

		if ((stage & ShaderStage::Vertex) == ShaderStage::Vertex)
		{
			flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		}

		if ((stage & ShaderStage::Fragment) == ShaderStage::Fragment)
		{
			flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		if ((stage & ShaderStage::Compute) == ShaderStage::Compute)
		{
			flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
		}

		if ((stage & ShaderStage::Geometry) == ShaderStage::Geometry)
		{
			flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
		}

		if ((stage & ShaderStage::RayGen) == ShaderStage::RayGen)
		{
			flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		}

		// Any hit group
		{
			if ((stage & ShaderStage::RayAnyHit_0) == ShaderStage::RayAnyHit_0)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			}

			if ((stage & ShaderStage::RayAnyHit_1) == ShaderStage::RayAnyHit_1)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			}

			if ((stage & ShaderStage::RayAnyHit_2) == ShaderStage::RayAnyHit_2)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			}

			if ((stage & ShaderStage::RayAnyHit_3) == ShaderStage::RayAnyHit_3)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			}
		}

		// Close hit group
		{
			if ((stage & ShaderStage::RayCloseHit_0) == ShaderStage::RayCloseHit_0)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			}

			if ((stage & ShaderStage::RayCloseHit_1) == ShaderStage::RayCloseHit_1)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			}

			if ((stage & ShaderStage::RayCloseHit_2) == ShaderStage::RayCloseHit_2)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			}

			if ((stage & ShaderStage::RayCloseHit_3) == ShaderStage::RayCloseHit_3)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			}
		}

		// Miss group
		{
			if ((stage & ShaderStage::RayMiss_0) == ShaderStage::RayMiss_0)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR;
			}

			if ((stage & ShaderStage::RayMiss_1) == ShaderStage::RayMiss_1)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR;
			}

			if ((stage & ShaderStage::RayMiss_2) == ShaderStage::RayMiss_2)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR;
			}

			if ((stage & ShaderStage::RayMiss_3) == ShaderStage::RayMiss_3)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR;
			}
		}

		// Callable group
		{
			if ((stage & ShaderStage::Callable_0) == ShaderStage::Callable_0)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR;
			}

			if ((stage & ShaderStage::Callable_1) == ShaderStage::Callable_1)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR;
			}

			if ((stage & ShaderStage::Callable_2) == ShaderStage::Callable_2)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR;
			}

			if ((stage & ShaderStage::Callable_3) == ShaderStage::Callable_3)
			{
				flags |= VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR;
			}
		}

		return (VkShaderStageFlagBits)flags;
	}

	VkPrimitiveTopology Gfx_VulkanHelpers::GetTopology(DrawMode mode)
	{
		switch (mode)
		{
		case DrawMode::Triangle: 
			return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case DrawMode::Line:		   
			return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case DrawMode::Fan:			   
			return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
		case DrawMode::Triangle_Strip: 
			return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		default:			           
			return VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}

	VkCullModeFlags Gfx_VulkanHelpers::GetCullMode(CullMode mode)
	{
		switch (mode)
		{
		case CullMode::Back:
			return VK_CULL_MODE_BACK_BIT;
		case CullMode::Front:
			return VK_CULL_MODE_FRONT_BIT;
		case CullMode::None:
			return VK_CULL_MODE_NONE;
		default:
			return VK_CULL_MODE_BACK_BIT;
		}
	}

	VkPolygonMode Gfx_VulkanHelpers::GetPolygonMode(PolygonMode mode)
	{
		switch (mode)
		{
		case PolygonMode::Fill:
			return VkPolygonMode::VK_POLYGON_MODE_FILL;
		case PolygonMode::Line:
			return VkPolygonMode::VK_POLYGON_MODE_LINE;
		case PolygonMode::Point:
			return VkPolygonMode::VK_POLYGON_MODE_POINT;
		default:
			return VkPolygonMode::VK_POLYGON_MODE_FILL;
		}
	}

	VkBlendFactor Gfx_VulkanHelpers::GetBlendFactor(BlendFactor factor)
	{
		switch (factor)
		{
		case BlendFactor::NONE:
			return VK_BLEND_FACTOR_ZERO;
		case BlendFactor::ONE:
			return VK_BLEND_FACTOR_ONE;
		case BlendFactor::ZERO:
			return VK_BLEND_FACTOR_ZERO;
		case BlendFactor::SRC_ALPHA:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case BlendFactor::SRC_COLOR:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case BlendFactor::ONE_MINUS_SRC_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BlendFactor::DST_COLOR:
			return VK_BLEND_FACTOR_DST_COLOR;
		case BlendFactor::ONE_MINUS_DST_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case BlendFactor::ONE_MINUS_SRC_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DST_ALPHA:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case BlendFactor::ONE_MINUS_DST_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case BlendFactor::CONSTANT_COLOR:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case BlendFactor::ONE_MINUS_CONSTANT_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case BlendFactor::CONSTANT_ALPHA:
			return VK_BLEND_FACTOR_CONSTANT_ALPHA;
		case BlendFactor::ONE_MINUS_CONSTANT_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		case BlendFactor::SRC_ALPHA_SATURATE:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case BlendFactor::SRC1_COLOR:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case BlendFactor::ONE_MINUS_SRC1_COLOR:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case BlendFactor::SRC1_ALPHA:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case BlendFactor::ONE_MINUS_SRC1_ALPHA:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		default:
			return VK_BLEND_FACTOR_ONE;
		}
	}

	VkBlendOp Gfx_VulkanHelpers::GetBlendOp(BlendOp op)
	{
		switch (op)
		{
		case BlendOp::ADD:
			return VK_BLEND_OP_ADD;
		case BlendOp::SUBTRACT:
			return VK_BLEND_OP_SUBTRACT;
		case BlendOp::REVERSE_SUBTRACT:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BlendOp::MIN:
			return VK_BLEND_OP_MIN;
		case BlendOp::MAX:
			return VK_BLEND_OP_MAX;
		default:
			return VK_BLEND_OP_ADD;
		}
	}

	bool Gfx_VulkanHelpers::IsDepthFormat(Format format)
	{
		switch (format)
		{
		case Format::D16_UNORM:
			return true;
		case Format::D16_UNORM_S8_UINT:
			return true;;
		case Format::D32_SFLOAT:
			return true;
		case Format::D32_SFLOAT_S8_UINT:
			return true;
		default:
			return false;
		}
	}

	void Gfx_VulkanHelpers::ClearImage(class Gfx_PixelStorage* storage, const glm::vec4& color, VkImageLayout layout, VkCommandBuffer cmdbuffer,
		VkImageSubresourceRange* pRange)
	{
		VkClearColorValue val{};

		val.float32[0] = color.r;
		val.float32[1] = color.g;
		val.float32[2] = color.b;
		val.float32[3] = color.a;

		val.int32[0] = static_cast<int32_t>(color.r);
		val.int32[1] = static_cast<int32_t>(color.g);
		val.int32[2] = static_cast<int32_t>(color.b);
		val.int32[3] = static_cast<int32_t>(color.a);

		val.uint32[0] = static_cast<uint32_t>(color.r);
		val.uint32[1] = static_cast<uint32_t>(color.g);
		val.uint32[2] = static_cast<uint32_t>(color.b);
		val.uint32[3] = static_cast<uint32_t>(color.a);

		vkCmdClearColorImage(cmdbuffer, storage->m_Image, layout, &val, pRange == nullptr ? 0 : 1, pRange);
	}

	void Gfx_VulkanHelpers::CopyDataToImage(Gfx_PixelStorage* storage, void* data, VkImageLayout layout)
	{
		BufferCreateDesc bufferDesc{};
		bufferDesc.myData = data;
		bufferDesc.mySize = storage->m_Desc.myWidth * storage->m_Desc.myHeight * Gfx_Helpers::GetFormatSize(storage->m_Desc.myFormat);
		bufferDesc.myFlags = BufferCreateDesc::CreateFlags::Staging;

		Gfx_Buffer stagingBuffer{};
		stagingBuffer.Create(bufferDesc);

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = storage->m_Desc.myMipLevels;
		subresourceRange.layerCount = storage->m_Desc.myArrayLayers;

		Gfx_CmdBuffer cmdBuffer{};
		CmdBufferCreateDesc cmdDesc{};
		cmdBuffer.Create(&cmdDesc);

		cmdBuffer.CmdBeginRecord();

		InsertImageMemoryBarrier(
			cmdBuffer.GetBuffer(),
			storage,
			0,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			subresourceRange);

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = storage->m_Desc.myWidth;
		bufferCopyRegion.imageExtent.height = storage->m_Desc.myHeight;
		bufferCopyRegion.imageExtent.depth = storage->m_Desc.myArrayLayers;

		vkCmdCopyBufferToImage(cmdBuffer.GetBuffer(), stagingBuffer.GetRawBuffer(), storage->m_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

		GenerateMipMaps(cmdBuffer.GetBuffer(), storage, subresourceRange);

		InsertImageMemoryBarrier(cmdBuffer.GetBuffer(), storage, 0, 0, layout,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, subresourceRange);

		cmdBuffer.CmdEndRecord();

		Gfx_VulkanHelpers::ExecuteCmdBuffer(&cmdBuffer);
	}


	void Gfx_VulkanHelpers::InitializeImageResource(class Gfx_PixelStorage* storage, VkImageLayout layout)
	{
		Gfx_CmdBuffer cmdBuffer{};
		CmdBufferCreateDesc cmdDesc{};
		cmdBuffer.Create(&cmdDesc);

		cmdBuffer.CmdBeginRecord();

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = storage->m_Desc.myMipLevels;
		subresourceRange.layerCount = storage->m_Desc.myArrayLayers;

		GenerateMipMaps(cmdBuffer.GetBuffer(), storage, subresourceRange);

		InsertImageMemoryBarrier(cmdBuffer.GetBuffer(), storage, 0, 0, layout,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, subresourceRange);

		cmdBuffer.CmdEndRecord();

		Gfx_VulkanHelpers::ExecuteCmdBuffer(&cmdBuffer);
	}

	bool Gfx_VulkanHelpers::IsFormatIsFilterable(VkFormat format, VkImageTiling tiling)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(Gfx_Context::GetDevice().GetPhysicalDevice(), format, &formatProps);

		if (tiling == VK_IMAGE_TILING_OPTIMAL)
			return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

		if (tiling == VK_IMAGE_TILING_LINEAR)
			return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

		return false;
	}

	void Gfx_VulkanHelpers::InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, class Gfx_PixelStorage* storage, VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange)
	{
		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.srcAccessMask = srcAccessMask;
		imageMemoryBarrier.dstAccessMask = dstAccessMask;
		imageMemoryBarrier.oldLayout = storage->GetImageLayout();
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = storage->GetImage();
		imageMemoryBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(
			cmdbuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);

		storage->SetImageLayout(newImageLayout);
	}


	void Gfx_VulkanHelpers::GenerateMipMaps(VkCommandBuffer cmd, Gfx_PixelStorage* storage, VkImageSubresourceRange& range)
	{
		const PixelStorageCreateDesc& desc = storage->GetDesc();

		for (int32_t i = 1; i < desc.myMipLevels; i++)
		{
			VkImageBlit imageBlit{};

			imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.srcSubresource.mipLevel = i - 1;
			imageBlit.srcOffsets[1].x = int32_t(desc.myWidth >> (i - 1));
			imageBlit.srcOffsets[1].y = int32_t(desc.myHeight >> (i - 1));
			imageBlit.srcOffsets[1].z = 1;

			imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.dstSubresource.layerCount = 1;
			imageBlit.dstSubresource.mipLevel = i;
			imageBlit.dstOffsets[1].x = int32_t(desc.myWidth >> i);
			imageBlit.dstOffsets[1].y = int32_t(desc.myHeight >> i);
			imageBlit.dstOffsets[1].z = 1;

			VkImageSubresourceRange mipSubRange = {};
			mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mipSubRange.baseMipLevel = i;
			mipSubRange.levelCount = 1;
			mipSubRange.layerCount = 1;

			InsertImageMemoryBarrier(
				cmd,
				storage,
				0,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				mipSubRange);

			vkCmdBlitImage(
				cmd,
				storage->GetImage(),
				storage->GetImageLayout(),
				storage->GetImage(),
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlit,
				VK_FILTER_LINEAR);

			InsertImageMemoryBarrier(
				cmd,
				storage,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_TRANSFER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				mipSubRange);
		}
	}


	void Gfx_VulkanHelpers::CreateVkRenderPass(FramebufferCreateDesc* fbDesc, VkRenderPass& outVkPass)
	{
		const uint32_t arraySize = static_cast<uint32_t>(fbDesc->myAttachments.size());
		std::vector<VkAttachmentReference> colorAttachmentReferences;
		std::vector<VkAttachmentReference> depthAttachmentReferences;
		std::vector<VkAttachmentDescription> attachmentDescriptions(arraySize);

		for (uint32_t i = 0; i < arraySize; ++i)
		{
			FramebufferAttachment& attachment = fbDesc->myAttachments[i];

			VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (fbDesc->myIsTargetsSwapchain)
				layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			const bool isDepth = IsDepthFormat(attachment.myFormat);

			attachmentDescriptions[i].format = GetFormat(attachment.myFormat);
			attachmentDescriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDescriptions[i].loadOp =  GetLoadOp(attachment.myLoadOp);
			attachmentDescriptions[i].storeOp = GetStoreOp(attachment.myStoreOp);
			attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescriptions[i].finalLayout = isDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : layout;

			VkAttachmentReference attachmentReference{};
			attachmentReference.attachment = i;
			attachmentReference.layout = isDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			if (isDepth)
				depthAttachmentReferences.emplace_back(attachmentReference);
			else
				colorAttachmentReferences.emplace_back(attachmentReference);
		}

		VkSubpassDescription subpass = {};
		{
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size());
			subpass.pColorAttachments = colorAttachmentReferences.data();
			subpass.pDepthStencilAttachment = depthAttachmentReferences.data();
		}

		std::vector<VkSubpassDependency> dependencies;
		if (fbDesc->myIsAutoSync)
		{
			// Use subpass dependencies for attachment layout transitions
			dependencies.resize(2);

			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}

		VkRenderPassCreateInfo renderPassCI = {};
		{
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
			renderPassCI.pAttachments = attachmentDescriptions.data();
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpass;
			renderPassCI.dependencyCount = static_cast<uint32_t>(dependencies.size());
			renderPassCI.pDependencies = dependencies.data();
		}

		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();
		VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassCI, nullptr, &outVkPass));
	}

	void Gfx_VulkanHelpers::SetImageLayout(const ImageLayoutTransitionDesc& desc)
	{
		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = desc.OldImageLayout;
		imageMemoryBarrier.newLayout = desc.NewImageLayout;
		imageMemoryBarrier.image = desc.Image;
		imageMemoryBarrier.subresourceRange = desc.SubresourceRange;

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (desc.OldImageLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			imageMemoryBarrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Target layouts (new)
		// Destination access mask controls the dependency for the new image layout
		switch (desc.NewImageLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (imageMemoryBarrier.srcAccessMask == 0)
			{
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Put barrier inside setup command buffer
		vkCmdPipelineBarrier(
			desc.CmdBuffer,
			desc.SrcStageMask,
			desc.DstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	void Gfx_VulkanHelpers::CopyPixelStorageToSwapchain(uint32_t width, uint32_t height, Gfx_CmdBuffer* _cmd, Gfx_PixelStorage* storage)
	{
		const auto& swapbuffer = Gfx_Context::GetSwapchain().GetCurrentBuffer();
		VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		VkImageLayout oldLayout = storage->GetImageLayout();
		VkCommandBuffer cmd = _cmd->GetBuffer();

		ImageLayoutTransitionDesc transitionDesc{};
		transitionDesc.CmdBuffer = cmd;
		transitionDesc.Image = swapbuffer.Image;
		transitionDesc.OldImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		transitionDesc.NewImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		transitionDesc.SubresourceRange = subresourceRange;

		SetImageLayout(transitionDesc);

		transitionDesc.Image = storage->GetImage();
		transitionDesc.OldImageLayout = oldLayout;
		transitionDesc.NewImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		SetImageLayout(transitionDesc);

		VkImageCopy copyRegion{};
		copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.srcOffset = { 0, 0, 0 };
		copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
		copyRegion.dstOffset = { 0, 0, 0 };
		copyRegion.extent = { width, height, 1 };

		vkCmdCopyImage(cmd, storage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			swapbuffer.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		transitionDesc.Image = swapbuffer.Image;
		transitionDesc.OldImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		transitionDesc.NewImageLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		SetImageLayout(transitionDesc);

		transitionDesc.Image = storage->GetImage();
		transitionDesc.OldImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;;
		transitionDesc.NewImageLayout = oldLayout;

		SetImageLayout(transitionDesc);
	}

	void Gfx_VulkanHelpers::ExecuteCmdBuffer(Gfx_CmdBuffer* cmdBuffer)
	{
		if (!cmdBuffer->IsGood()) // assert
		{

		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;

		VkCommandBuffer buffer = cmdBuffer->GetBuffer();
		submitInfo.pCommandBuffers = &buffer;

		VkFence fence = nullptr;
		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();

		{
			VkFenceCreateInfo fenceCI = {};
			fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCI.flags = 0;

			VK_CHECK_RESULT(vkCreateFence(device, &fenceCI, nullptr, &fence));

			{
				std::lock_guard<std::mutex> lock(*s_locVulkanHelpersMutex);
				VkQueue queue = Gfx_Context::GetDevice().GetQueue(Gfx_VulkanDevice::QueueFamilyFlags::Graphics);
				VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
			}

			constexpr uint64_t time_out = 100000000000;
			VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, time_out));
			vkDestroyFence(device, fence, nullptr);
		}

		cmdBuffer->m_State = Gfx_CmdBuffer::State::Wait;
	}

}