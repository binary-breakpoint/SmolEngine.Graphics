#include "Gfx_Precompiled.h"
#include "Common/Gfx_Framebuffer.h"
#include "Common/Gfx_Sampler.h"
#include "Common/Gfx_CmdBuffer.h"
#include "Gfx_RenderContext.h"

#include "Backend/Gfx_VulkanHelpers.h"

#include <imgui/backends/imgui_impl_vulkan.h>

namespace SmolEngine
{
	FramebufferAttachment::FramebufferAttachment(Format format, const glm::vec4& color, const std::string& name)
		:
		myName{name},
		myColor{color},
		myFormat{format} {}

	Gfx_Framebuffer::Gfx_Framebuffer()
		:
		m_DepthAttachment{nullptr},
		m_RenderPass{nullptr} {}

	Gfx_Framebuffer::~Gfx_Framebuffer()
	{
		Free();
	}

	void Gfx_Framebuffer::Create(FramebufferCreateDesc* info)
	{
		GFX_ASSERT(m_Desc.myAttachments.size() > 0)
		GFX_ASSERT(m_Desc.mySize.x > 0 && m_Desc.mySize.y > 0)

		if (info->mySampler == nullptr)
			info->mySampler = Gfx_RenderContext::GetDefaultSampler();

		m_Desc = *info;

		const uint32_t arraySize = static_cast<uint32_t>(m_Desc.myAttachments.size());

		m_Attachments.resize(arraySize);
		std::vector<VkImageView> imageViews(arraySize);

		Gfx_CmdBuffer cmdBuffer{};
		CmdBufferCreateDesc cmdDesc{};
		cmdBuffer.Create(&cmdDesc);

		cmdBuffer.CmdBeginRecord();

		for (uint32_t i = 0; i < arraySize; ++i)
		{
			FramebufferAttachment& attachmentDesc = m_Desc.myAttachments[i];
			Gfx_Framebuffer::Attachment& attachment = m_Attachments[i];

			const bool isDepthAttachement = Gfx_VulkanHelpers::IsDepthFormat(attachmentDesc.myFormat);

			VkImageUsageFlags usageFlags = isDepthAttachement ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT :
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

			if (isDepthAttachement)
			{
				switch (attachmentDesc.myDepthMode)
				{
				case DepthMode::DEPTH_STENCIL:
					aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
					break;
				case DepthMode::DEPTH_ONLY:
					aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
					break;
				case DepthMode::STENCIL_ONLY:
					aspectFlags = VK_IMAGE_ASPECT_STENCIL_BIT;
					break;
				}
			}

			PixelStorageCreateDesc pixelStorageDesc{};

			pixelStorageDesc.myUsageFlags = usageFlags;
			pixelStorageDesc.myAspectMask = aspectFlags;
			pixelStorageDesc.myArrayLayers = 1;
			pixelStorageDesc.mySize = info->mySize;
			pixelStorageDesc.myFormat = attachmentDesc.myFormat;
			pixelStorageDesc.myMipLevels = attachmentDesc.myMips;

			attachment.myPixelStorage = Gfx_RenderContext::CreatePixelStorage(pixelStorageDesc);

			imageViews[i] = attachment.myPixelStorage->GetImageView();

			if (!attachmentDesc.myName.empty())
				m_AttachmentsMap[attachmentDesc.myName] = i;

			attachment.myClearValue.color = { { attachmentDesc.myColor.r, attachmentDesc.myColor.g,
				attachmentDesc.myColor.b, attachmentDesc.myColor.a} };
			attachment.myClearValue.depthStencil = { attachmentDesc.myColor.r, 0 };
			attachment.myClearAttachment.aspectMask = aspectFlags;
			attachment.myClearAttachment.clearValue = attachment.myClearValue;
			attachment.myClearAttachment.colorAttachment = i;

			m_ClearValues.emplace_back(attachment.myClearValue);

			VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			if (isDepthAttachement)
			{
				finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			}
			else if (info->myIsTargetsSwapchain && !isDepthAttachement)
			{
				finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			}

			Gfx_VulkanHelpers::InsertImageMemoryBarrier(cmdBuffer.GetBuffer(), attachment.myPixelStorage.get(), 0, 0, finalLayout,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, { aspectFlags, 0, 1, 0, 1 });

			VkDescriptorImageInfo& desc = attachment.myImageInfo;
			desc.imageLayout = attachment.myPixelStorage->GetImageLayout();
			desc.imageView = attachment.myPixelStorage->GetImageView();
			desc.sampler = info->mySampler->GetSampler();

			if (isDepthAttachement)
			{
				m_DepthAttachment = &attachment;
			}
			else if(info->myIsUsedByImGui && !info->myIsTargetsSwapchain)
			{
				attachment.myImGuiID = ImGui_ImplVulkan_AddTexture(desc.sampler, desc.imageView, desc.imageLayout);
			}
		}

		cmdBuffer.CmdEndRecord();
		cmdBuffer.~Gfx_CmdBuffer();

		if (m_RenderPass == nullptr)
			Gfx_VulkanHelpers::CreateVkRenderPass(&m_Desc, m_RenderPass);

		VkFramebufferCreateInfo frameufferCreateInfo = {};
		frameufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameufferCreateInfo.pNext = NULL;
		frameufferCreateInfo.renderPass = m_RenderPass;
		frameufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
		frameufferCreateInfo.pAttachments = imageViews.data();
		frameufferCreateInfo.width = info->mySize.x;
		frameufferCreateInfo.height = info->mySize.y;
		frameufferCreateInfo.layers = 1;

		VkDevice device = Gfx_App::GetDevice().GetLogicalDevice();

		if (info->myIsTargetsSwapchain)
		{
			uint32_t count = Gfx_App::GetSwapchain().m_ImageCount;
			m_FrameBuffers.resize(count);

			for (uint32_t i = 0; i < count; ++i)
			{
				imageViews[0] = Gfx_App::GetSwapchain().m_Buffers[i].View;
				VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameufferCreateInfo, nullptr, &m_FrameBuffers[i]));
			}

			return;
		}

		m_FrameBuffers.resize(1);
		VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameufferCreateInfo, nullptr, &m_FrameBuffers[0]));
	}

	void Gfx_Framebuffer::OnResize(const glm::ivec2& size)
	{
		m_Desc.mySize = size;

		Free();
		Create(&m_Desc);
	}

	void Gfx_Framebuffer::Free()
	{
		VK_DESTROY_DEVICE_HANDLE(m_RenderPass, vkDestroyRenderPass);

		for (auto& fb : m_FrameBuffers)
			VK_DESTROY_DEVICE_HANDLE(fb, vkDestroyFramebuffer);

		m_Attachments.clear();
		m_FrameBuffers.clear();
		m_AttachmentsMap.clear();
	}

	Ref<Gfx_PixelStorage> Gfx_Framebuffer::GetPixelStorage(const std::string& name)
	{
		Attachment* attachment = GetAttachment(name);
		if (attachment)
			return attachment->myPixelStorage;

		return nullptr;
	}

	Ref<Gfx_PixelStorage> Gfx_Framebuffer::GetPixelStorage(uint32_t index)
	{
		Attachment* attachment = GetAttachment(index);
		if (attachment)
			return attachment->myPixelStorage;

		return nullptr;
	}

	const glm::ivec2& Gfx_Framebuffer::GetSize() const
	{
		return m_Desc.mySize;
	}

	Gfx_Framebuffer::Attachment* Gfx_Framebuffer::GetAttachment(const std::string& name)
	{
		const auto& it = m_AttachmentsMap.find(name);
		if (it != m_AttachmentsMap.end())
			return &m_Attachments[it->second];

		return nullptr;
	}

	Gfx_Framebuffer::Attachment* Gfx_Framebuffer::GetAttachment(uint32_t index)
	{
		if (index < m_Attachments.size())
		{
			return &m_Attachments[index];
		}

		return nullptr;
	}

	Gfx_Framebuffer::Attachment* Gfx_Framebuffer::GetDepthAttachment()
	{
		return m_DepthAttachment;
	}

	VkFramebuffer Gfx_Framebuffer::GetRawBuffer() const
	{
		if (m_Desc.myIsTargetsSwapchain)
		{
			return m_FrameBuffers[Gfx_App::GetSwapchain().GetCurrentBufferIndex()];
		}

		return m_FrameBuffers[0];
	}

	void* Gfx_Framebuffer::GetImGuiTextureID(uint32_t index)
	{
		if (index < m_Attachments.size())
		{
			return m_Attachments[index].myImGuiID;
		}

		return nullptr;
	}

}