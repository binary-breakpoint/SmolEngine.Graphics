#include "Gfx_Precompiled.h"
#include "Common/Gfx_Framebuffer.h"
#include "Common/Gfx_Sampler.h"
#include "Common/Gfx_CmdBuffer.h"
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
		if (m_Desc.myAttachments.size() > 1 && m_Desc.myIsTargetsSwapchain || m_Desc.myAttachments.size() == 0)
		{
			// TODO: Assert
		}

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
			pixelStorageDesc.myHeight = info->myHeight;
			pixelStorageDesc.myWidth = info->myWidth;
			pixelStorageDesc.myFormat = attachmentDesc.myFormat;
			pixelStorageDesc.myMipLevels = attachmentDesc.myMips;

			attachment.myPixelStorage.Create(&pixelStorageDesc);

			VkDescriptorImageInfo& descriptorInfo = attachment.myImageInfo;
			descriptorInfo.imageLayout = attachment.myPixelStorage.GetImageLayout();
			descriptorInfo.imageView = attachment.myPixelStorage.GetImageView();
			descriptorInfo.sampler = info->mySampler->GetSampler();

			imageViews[i] = attachment.myPixelStorage.GetImageView();

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

			ImageLayoutTransitionDesc layoutTransitionDesc{};
			layoutTransitionDesc.CmdBuffer = cmdBuffer.GetBuffer();
			layoutTransitionDesc.Image = attachment.myPixelStorage.GetImage();
			layoutTransitionDesc.OldImageLayout = attachment.myPixelStorage.GetImageLayout();
			layoutTransitionDesc.NewImageLayout = finalLayout;
			layoutTransitionDesc.SubresourceRange = { aspectFlags, 0, 1, 0, 1 };

			Gfx_VulkanHelpers::SetImageLayout(layoutTransitionDesc);

			if (isDepthAttachement)
			{
				m_DepthAttachment = &attachment;
			}
			else if(info->myIsUsedByImGui && !info->myIsTargetsSwapchain)
			{
				attachment.myImGuiID = ImGui_ImplVulkan_AddTexture(descriptorInfo.sampler, descriptorInfo.imageView,
					descriptorInfo.imageLayout);
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
		frameufferCreateInfo.width = info->myWidth;
		frameufferCreateInfo.height = info->myHeight;
		frameufferCreateInfo.layers = 1;

		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();

		if (info->myIsTargetsSwapchain)
		{
			uint32_t count = Gfx_Context::GetSwapchain().m_ImageCount;
			m_FrameBuffers.resize(count);

			for (uint32_t i = 0; i < count; ++i)
			{
				imageViews[0] = Gfx_Context::GetSwapchain().m_Buffers[i].View;
				VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameufferCreateInfo, nullptr, &m_FrameBuffers[i]));
			}

			return;
		}

		m_FrameBuffers.resize(1);
		VK_CHECK_RESULT(vkCreateFramebuffer(device, &frameufferCreateInfo, nullptr, &m_FrameBuffers[0]));
	}

	void Gfx_Framebuffer::OnResize(uint32_t width, uint32_t height)
	{
		if (m_Desc.myIsResizable)
		{
			Free();

			m_Desc.myWidth = width;
			m_Desc.myHeight = height;

			Create(&m_Desc);
		}
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

	void Gfx_Framebuffer::CmdClear(Gfx_CmdBuffer* cmd, const glm::vec4& color, uint32_t index /*= 0*/)
	{
		if (m_Attachments.size() < index)
		{
			auto& attachment = m_Attachments[index];

			attachment.myClearAttachment.clearValue.color = { { color.r,color.g, color.b, color.a } };

			VkClearRect clearRect = {};
			clearRect.layerCount = 1;
			clearRect.baseArrayLayer = 0;
			clearRect.rect.offset = { 0, 0 };
			clearRect.rect.extent = { (uint32_t)m_Desc.myWidth, (uint32_t)m_Desc.myHeight };

			vkCmdClearAttachments(cmd->GetBuffer(), 1, &attachment.myClearAttachment, 1, &clearRect);
		}
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
			return m_FrameBuffers[Gfx_Context::GetSwapchain().GetCurrentBufferIndex()];
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