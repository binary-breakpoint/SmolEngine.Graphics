#include "Gfx_Precompiled.h"

#include "Backend/Gfx_VulkanSwapchain.h"
#include "Backend/Gfx_VulkanInstance.h"
#include "Backend/Gfx_VulkanDevice.h"
#include <GLFW/glfw3.h>

#include <vulkan_memory_allocator/vk_mem_alloc.h>

static PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR = nullptr;
static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
static PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR = nullptr;
static PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR = nullptr;
static PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR = nullptr;
static PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR = nullptr;
static PFN_vkQueuePresentKHR fpQueuePresentKHR = nullptr;

namespace SmolEngine
{
	Gfx_VulkanSwapchain::Gfx_VulkanSwapchain()
		:
		m_DepthStencil{nullptr},
		m_RenderPass{nullptr},
		m_Swapchain{nullptr},
		m_PipelineCash{nullptr},
		m_Instance{nullptr},
		m_Device{nullptr},
		m_Surface{nullptr},
		m_SurfaceFormat{nullptr},
		m_PresentMode{nullptr},
		m_PresentModeCount{0},
		m_SurfaceFormatCount{0},
		m_ImageCount{0},
		m_Width{0},
		m_Height{0},
		m_QueueNodeIndex{0},
		m_CurrentBufferIndex{0},
		m_ColorFormat{ VK_FORMAT_B8G8R8A8_UNORM },
		m_DepthBufferFormat{ VK_FORMAT_B8G8R8A8_UNORM },
		m_ColorSpace{ VkColorSpaceKHR::VK_COLORSPACE_SRGB_NONLINEAR_KHR }
		
    {

    }

    bool Gfx_VulkanSwapchain::Init(Gfx_VulkanInstance* instance, Gfx_VulkanDevice* device, GLFWwindow* window, bool clearOP)
    {
        m_Instance = instance;
		m_Device = device;
		m_DepthStencil = new DepthStencil();

		GetPtrs();
		FindDepthStencilFormat();
		if (InitSurface(window) == VK_SUCCESS)
		{
			return CreateRenderPass(clearOP) == VK_SUCCESS;
		}

		return false;
    }

	bool Gfx_VulkanSwapchain::Prepare(uint32_t width, uint32_t height)
	{
		FreeResources();

		if (CreateDepthStencil() == VK_SUCCESS)
		{
			if (CreateFramebuffers(width, height) == VK_SUCCESS)
			{
				return true;
			}
		}

		return false;
	}

	void Gfx_VulkanSwapchain::Create(uint32_t* width, uint32_t* height, bool vSync)
	{
		GFX_ASSERT_MSG((m_Device || m_Instance), "VulkanDevice or VulkanInstance is nullptr")
		
		VkSwapchainKHR oldSwapchain = m_Swapchain;

		// Get physical device surface properties and formats
		VkSurfaceCapabilitiesKHR surfCaps;
		VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device->GetPhysicalDevice(), m_Surface, &surfCaps));

		// Get available present modes
		uint32_t presentModeCount;
		VK_CHECK_RESULT(fpGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetPhysicalDevice(), m_Surface, &presentModeCount, NULL));
		assert(presentModeCount > 0);

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		VK_CHECK_RESULT(fpGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetPhysicalDevice(), m_Surface, &presentModeCount, presentModes.data()));

		VkExtent2D swapchainExtent = {};
		// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
		if (surfCaps.currentExtent.width == (uint32_t)-1)
		{
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchainExtent.width = *width;
			swapchainExtent.height = *height;
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = surfCaps.currentExtent;
			*width = surfCaps.currentExtent.width;
			*height = surfCaps.currentExtent.height;
		}

		m_Width = *width;
		m_Height = *height;

		// Select a present mode for the swapchain

		// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
		// This mode waits for the vertical blank ("v-sync")
		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		// If v-sync is not requested, try to find a mailbox mode
		// It's the lowest latency non-tearing present mode available
		if (!vSync)
		{
			for (size_t i = 0; i < presentModeCount; i++)
			{
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
				{
					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		}

		// Determine the number of images
		uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
		if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
		{
			desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
		}

		// Find the transformation of the surface
		VkSurfaceTransformFlagsKHR preTransform;
		if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			// We prefer a non-rotated transform
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			preTransform = surfCaps.currentTransform;
		}

		// Find a supported composite alpha format (not all devices support alpha opaque)
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Simply select the first composite alpha format available
		std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto& compositeAlphaFlag : compositeAlphaFlags) {
			if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) {
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}

		VkSwapchainCreateInfoKHR swapchainCI = {};
		{
			swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainCI.pNext = NULL;
			swapchainCI.surface = m_Surface;
			swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
			swapchainCI.imageFormat = m_ColorFormat;
			swapchainCI.imageColorSpace = m_ColorSpace;
			swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
			swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
			swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
			swapchainCI.imageArrayLayers = 1;
			swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCI.queueFamilyIndexCount = 0;
			swapchainCI.pQueueFamilyIndices = NULL;
			swapchainCI.presentMode = swapchainPresentMode;
			swapchainCI.oldSwapchain = oldSwapchain;

			// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
			swapchainCI.clipped = VK_TRUE;
			swapchainCI.compositeAlpha = compositeAlpha;

			// Enable transfer source on swap chain images if supported
			if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
				swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			}

			// Enable transfer destination on swap chain images if supported
			if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
				swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			}

			VK_CHECK_RESULT(fpCreateSwapchainKHR(m_Device->GetLogicalDevice(), &swapchainCI, nullptr, &m_Swapchain));
		}

		// If an existing swap chain is re-created, destroy the old swap chain
		// This also cleans up all the presentable images
		if (oldSwapchain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < m_ImageCount; i++)
			{
				vkDestroyImageView(m_Device->GetLogicalDevice(), m_Buffers[i].View, nullptr);
			}

			fpDestroySwapchainKHR(m_Device->GetLogicalDevice(), oldSwapchain, nullptr);
		}

		VK_CHECK_RESULT(fpGetSwapchainImagesKHR(m_Device->GetLogicalDevice(), m_Swapchain, &m_ImageCount, NULL));
		// Get the swap chain images
		m_Images.resize(m_ImageCount);
		VK_CHECK_RESULT(fpGetSwapchainImagesKHR(m_Device->GetLogicalDevice(), m_Swapchain, &m_ImageCount, m_Images.data()));

		// Get the swap chain buffers containing the image and imageview
		m_Buffers.resize(m_ImageCount);
		for (uint32_t i = 0; i < m_ImageCount; i++)
		{
			VkImageViewCreateInfo colorAttachmentView = {};
			{
				colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				colorAttachmentView.pNext = NULL;
				colorAttachmentView.format = m_ColorFormat;
				colorAttachmentView.components = {
					VK_COMPONENT_SWIZZLE_R,
					VK_COMPONENT_SWIZZLE_G,
					VK_COMPONENT_SWIZZLE_B,
					VK_COMPONENT_SWIZZLE_A
				};
				colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				colorAttachmentView.subresourceRange.baseMipLevel = 0;
				colorAttachmentView.subresourceRange.levelCount = 1;
				colorAttachmentView.subresourceRange.baseArrayLayer = 0;
				colorAttachmentView.subresourceRange.layerCount = 1;
				colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
				colorAttachmentView.flags = 0;

				m_Buffers[i].Image = m_Images[i];
				colorAttachmentView.image = m_Buffers[i].Image;
			}

			VK_CHECK_RESULT(vkCreateImageView(m_Device->GetLogicalDevice(), &colorAttachmentView, nullptr, &m_Buffers[i].View));
		}
	}

	void Gfx_VulkanSwapchain::OnResize(uint32_t* width, uint32_t* height, bool vsync, Gfx_CmdBuffer* cmd)
	{
		const auto& device = m_Device->GetLogicalDevice();

		vkDeviceWaitIdle(device);
		{
			Create(width, height, vsync);
			Prepare(*width, *height);

			if (cmd != nullptr)
				cmd->Reset();
		}
		vkDeviceWaitIdle(device);
	}

	void Gfx_VulkanSwapchain::CleanUp()
	{
		GFX_ASSERT_MSG((m_Device || m_Instance), "VulkanDevice or VulkanInstance is nullptr")

		if (m_Swapchain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < m_ImageCount; i++)
			{
				vkDestroyImageView(m_Device->GetLogicalDevice(), m_Buffers[i].View, nullptr);
			}
		}

		if (m_Surface != VK_NULL_HANDLE)
		{
			fpDestroySwapchainKHR(m_Device->GetLogicalDevice(), m_Swapchain, nullptr);
			vkDestroySurfaceKHR(m_Instance->GetInstance(), m_Surface, nullptr);
		}

		m_Surface = VK_NULL_HANDLE;
		m_Swapchain = VK_NULL_HANDLE;
	}

	void Gfx_VulkanSwapchain::ClearColors(VkCommandBuffer cmdBuffer, const glm::vec4& color)
	{
		VkClearRect clearRect = {};
		clearRect.layerCount = 1;
		clearRect.baseArrayLayer = 0;
		clearRect.rect.offset = { 0, 0 };
		clearRect.rect.extent = { m_Width,
			m_Height };

		vkCmdClearAttachments(cmdBuffer, 2, m_ClearAttachments, 1, &clearRect);
	}

	VkResult Gfx_VulkanSwapchain::AcquireNextImage(VkSemaphore presentCompleteSemaphore)
	{
		GFX_ASSERT_MSG((m_Device || m_Instance), "VulkanDevice or VulkanInstance is nullptr")

		// By setting timeout to UINT64_MAX we will always wait until the next image has been acquired or an actual error is thrown
		// With that we don't have to handle VK_NOT_READY
		return fpAcquireNextImageKHR(m_Device->GetLogicalDevice(), m_Swapchain, UINT64_MAX, presentCompleteSemaphore, (VkFence)nullptr, &m_CurrentBufferIndex);
	}

	VkResult Gfx_VulkanSwapchain::QueuePresent(VkQueue queue, VkSemaphore waitSemaphore)
	{
		GFX_ASSERT_MSG((m_Device || m_Instance), "VulkanDevice or VulkanInstance is nullptr")

		VkPresentInfoKHR presentInfo = {};
		{
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.pNext = NULL;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &m_Swapchain;
			presentInfo.pImageIndices = &m_CurrentBufferIndex;

			// Check if a wait semaphore has been specified to wait for before presenting the image
			if (waitSemaphore != VK_NULL_HANDLE)
			{
				presentInfo.pWaitSemaphores = &waitSemaphore;
				presentInfo.waitSemaphoreCount = 1;
			}

			return fpQueuePresentKHR(queue, &presentInfo);
		}
	}

    VkResult Gfx_VulkanSwapchain::InitSurface(GLFWwindow* window)
    {
		VkResult result = glfwCreateWindowSurface(m_Instance->GetInstance(), window, nullptr, &m_Surface);
		VK_CHECK_RESULT(result);

		VkBool32 supported;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_Device->GetPhysicalDevice(), 
			m_Device->GetQueueFamilyIndices().Graphics, m_Surface, &supported);
		assert(supported == 1);

		FindColorSpaceFormat();

		return result;
    }

	VkResult Gfx_VulkanSwapchain::CreateRenderPass(bool clearOP)
	{
		VkResult result = VK_ERROR_UNKNOWN;
		std::array<VkAttachmentDescription, 2> attachments = {};

		// color attachment
		{
			attachments[0].format = m_ColorFormat;
			attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[0].loadOp = clearOP ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		// depth attachment
		{
			attachments[1].format = m_DepthBufferFormat;
			attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		VkAttachmentReference colorRef = { 0 , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkAttachmentReference depthRef = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDes = {};
		{
			subpassDes.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDes.colorAttachmentCount = 1;
			subpassDes.pColorAttachments = &colorRef;
			subpassDes.pDepthStencilAttachment = &depthRef;
			subpassDes.inputAttachmentCount = 0;
			subpassDes.pInputAttachments = nullptr;
			subpassDes.preserveAttachmentCount = 0;
			subpassDes.pPreserveAttachments = nullptr;
			subpassDes.pResolveAttachments = nullptr;
		}

		VkSubpassDependency subpassDependency = {};
		{
			subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDependency.dstSubpass = 0;
			subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.srcAccessMask = 0;
			subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		VkRenderPassCreateInfo renderPassCI = {};
		{
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassCI.pAttachments = attachments.data();
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpassDes;
			renderPassCI.dependencyCount = 1;
			renderPassCI.pDependencies = &subpassDependency;
		}

		result = vkCreateRenderPass(m_Device->GetLogicalDevice(), &renderPassCI, nullptr, &m_RenderPass);
		VK_CHECK_RESULT(result);

		VkPipelineCacheCreateInfo pipelineCacheCI = {};
		{
			pipelineCacheCI.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		}

		result = vkCreatePipelineCache(m_Device->GetLogicalDevice(), &pipelineCacheCI, nullptr, &m_PipelineCash);
		VK_CHECK_RESULT(result);

		return result;
	}

	VkResult Gfx_VulkanSwapchain::CreateDepthStencil()
	{
		VkResult result = VK_ERROR_UNKNOWN;

		VkImageCreateInfo imageCI = {};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = m_DepthBufferFormat;
		imageCI.mipLevels = 1;
		imageCI.extent.depth = 1;
		imageCI.arrayLayers = 1;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCI.extent = { m_Width, m_Height, 1 };
		imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		m_DepthStencil->Alloc = Gfx_VulkanAllocator::AllocImage(imageCI, VMA_MEMORY_USAGE_GPU_ONLY, m_DepthStencil->Image);

		VkImageViewCreateInfo depthStencilViewCI = {};
		{
			depthStencilViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			depthStencilViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			depthStencilViewCI.format = m_DepthBufferFormat;
			depthStencilViewCI.flags = 0;
			depthStencilViewCI.subresourceRange = {};
			depthStencilViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			depthStencilViewCI.subresourceRange.baseMipLevel = 0;
			depthStencilViewCI.subresourceRange.levelCount = 1;
			depthStencilViewCI.subresourceRange.baseArrayLayer = 0;
			depthStencilViewCI.subresourceRange.layerCount = 1;
			depthStencilViewCI.image = m_DepthStencil->Image;
			depthStencilViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
			depthStencilViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
			depthStencilViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
			depthStencilViewCI.components.a = VK_COMPONENT_SWIZZLE_A;

			result = vkCreateImageView(m_Device->GetLogicalDevice(), &depthStencilViewCI, nullptr, &m_DepthStencil->ImageView);
			VK_CHECK_RESULT(result);
		}

		return result;
	}

	void Gfx_VulkanSwapchain::GetPtrs()
	{
		fpGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>
			(vkGetInstanceProcAddr(m_Instance->GetInstance(),
			"vkGetPhysicalDeviceSurfaceSupportKHR"));

		fpGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>
			(vkGetInstanceProcAddr(m_Instance->GetInstance(),
			"vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));

		fpGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>
			(vkGetInstanceProcAddr(m_Instance->GetInstance(),
			"vkGetPhysicalDeviceSurfaceFormatsKHR"));

		fpGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>
			(vkGetInstanceProcAddr(m_Instance->GetInstance(),
			"vkGetPhysicalDeviceSurfacePresentModesKHR"));

		fpCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vkGetDeviceProcAddr
		(m_Device->GetLogicalDevice(),
			"vkCreateSwapchainKHR"));

		fpDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vkGetDeviceProcAddr
		(m_Device->GetLogicalDevice(),
			"vkDestroySwapchainKHR"));

		fpGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vkGetDeviceProcAddr
		(m_Device->GetLogicalDevice(),
			"vkGetSwapchainImagesKHR"));

		fpAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr
		(m_Device->GetLogicalDevice(),
			"vkAcquireNextImageKHR"));

		fpQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(vkGetDeviceProcAddr
		(m_Device->GetLogicalDevice(),
			"vkQueuePresentKHR"));
	}

	VkResult Gfx_VulkanSwapchain::CreateFramebuffers(uint32_t width, uint32_t height)
	{
		VkResult result = VK_ERROR_UNKNOWN;
		VkImageView ivAttachment[2];
		// Depth attachment is the same for all framebuffers
		ivAttachment[1] = m_DepthStencil->ImageView;

		VkFramebufferCreateInfo framebufferCI = {};
		{
			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.pNext = NULL;
			framebufferCI.renderPass = m_RenderPass;
			framebufferCI.attachmentCount = 2;
			framebufferCI.pAttachments = ivAttachment;
			framebufferCI.width = width;
			framebufferCI.height = height;
			framebufferCI.layers = 1;
		}

		// Create framebuffer for every swapchain image
		m_Framebuffers.resize(m_ImageCount);
		for (uint32_t i = 0; i < m_Framebuffers.size(); ++i)
		{
			ivAttachment[0] = m_Buffers[i].View;
			result = (vkCreateFramebuffer(m_Device->GetLogicalDevice(), &framebufferCI, nullptr, &m_Framebuffers[i]));

			GFX_ASSERT_MSG(result == VK_SUCCESS, "VulkanFramebuffer::Create: Failed to create framebuffer object!")
		}

		m_ClearAttachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		m_ClearAttachments[0].clearValue.color = { { 0.1f, 0.1f, 0.1f, 1.0f} };
		m_ClearAttachments[0].colorAttachment = 0;

		m_ClearAttachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		m_ClearAttachments[1].clearValue.depthStencil = { 1.0f, 0 };

		return result;
	}

	void Gfx_VulkanSwapchain::FindColorSpaceFormat()
	{
		// Get list of supported surface formats
		uint32_t formatCount;
		VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalDevice(), m_Surface, &formatCount, NULL));
		assert(formatCount > 0);

		std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		VK_CHECK_RESULT(fpGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalDevice(), m_Surface, &formatCount, surfaceFormats.data()));

		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
		if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			m_ColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
			m_ColorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			// iterate over the list of available surface format and
			// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
			bool found_B8G8R8A8_UNORM = false;
			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					m_ColorFormat = surfaceFormat.format;
					m_ColorSpace = surfaceFormat.colorSpace;
					found_B8G8R8A8_UNORM = true;
					break;
				}
			}

			// in case VK_FORMAT_B8G8R8A8_UNORM is not available
			// select the first available color format
			if (!found_B8G8R8A8_UNORM)
			{
				m_ColorFormat = surfaceFormats[0].format;
				m_ColorSpace = surfaceFormats[0].colorSpace;
			}
		}
	}

	void Gfx_VulkanSwapchain::FindDepthStencilFormat()
	{
		VkResult result = VK_ERROR_UNKNOWN;
		std::vector<VkFormat> depthFormats =
		{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		bool formatFound = false;
		for (auto& format : depthFormats)
		{
			VkFormatProperties formatProbs;
			vkGetPhysicalDeviceFormatProperties(m_Device->GetPhysicalDevice(), format, &formatProbs);

			if (formatProbs.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				m_DepthBufferFormat = format;
				formatFound = true;
				break;
			}
		}

		GFX_ASSERT_MSG(formatFound, "Depth Stencil Format not found!")
	}

	void Gfx_VulkanSwapchain::FreeResources()
	{
		if (m_DepthStencil->Image != nullptr)
		{
			Gfx_VulkanAllocator::FreeImage(m_DepthStencil->Image, m_DepthStencil->Alloc);

			m_DepthStencil->Alloc = nullptr;
			m_DepthStencil->Image = nullptr;
		}

		if (m_DepthStencil->ImageView != nullptr)
		{
			vkDestroyImageView(m_Device->GetLogicalDevice(), m_DepthStencil->ImageView, nullptr);
			m_DepthStencil->ImageView = nullptr;
		}

		for (auto& framebuffer : m_Framebuffers)
		{
			if (framebuffer != VK_NULL_HANDLE)
			{
				vkDestroyFramebuffer(m_Device->GetLogicalDevice(), framebuffer, nullptr);
			}
		}

		m_Framebuffers.clear();
	}

	uint32_t Gfx_VulkanSwapchain::GetWidth() const
	{
		return m_Width;
	}

	const VkSurfaceKHR Gfx_VulkanSwapchain::GetVkSurface() const
	{
		return m_Surface;
	}

	const VkFormat& Gfx_VulkanSwapchain::GetColorFormat() const
	{
		return m_ColorFormat;
	}

	const VkFormat& Gfx_VulkanSwapchain::GetDepthFormat() const
	{
		return m_DepthBufferFormat;
	}

	const Gfx_VulkanSwapchain::Buffer& Gfx_VulkanSwapchain::GetCurrentBuffer() const
	{
		return m_Buffers[m_CurrentBufferIndex];
	}

	const VkSwapchainKHR& Gfx_VulkanSwapchain::GetVkSwapchain() const
	{
		return m_Swapchain;
	}

	const VkFramebuffer Gfx_VulkanSwapchain::GetCurrentFramebuffer() const
	{
		return m_Framebuffers[m_CurrentBufferIndex];
	}

	const  VkImage Gfx_VulkanSwapchain::GetCurrentImage() const
	{
		return m_Images[m_CurrentBufferIndex];
	}

	const VkRenderPass Gfx_VulkanSwapchain::GetVkRenderPass() const
	{
		return m_RenderPass;
	}

	uint32_t Gfx_VulkanSwapchain::GetCurrentBufferIndex() const
	{
		return m_CurrentBufferIndex;
	}

	uint32_t& Gfx_VulkanSwapchain::GetCurrentBufferIndexRef()
	{
		return m_CurrentBufferIndex;
	}

	uint32_t Gfx_VulkanSwapchain::GetHeight() const
	{
		return m_Height;
	}
}