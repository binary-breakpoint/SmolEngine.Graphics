#pragma once

#include "Backend/Gfx_VulkanAllocator.h"

#include <vector>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace SmolEngine
{
	class Gfx_CmdBuffer;
	class Gfx_VulkanDevice;
	class Gfx_VulkanInstance;

	class Gfx_VulkanSwapchain
	{
		friend class Gfx_VulkanCmdBuffer;
		friend class Gfx_Framebuffer;
		friend class Gfx_VulkanSemaphore;
	public:
		struct Buffer
		{
			VkImage Image = VK_NULL_HANDLE;;
			VkImageView View = VK_NULL_HANDLE;;
		};

		struct DepthStencil
		{
			VkImage Image = VK_NULL_HANDLE;
			VkImageView ImageView = VK_NULL_HANDLE;;
			VmaAllocation Alloc = VK_NULL_HANDLE;;
		};

		Gfx_VulkanSwapchain();

		bool Init(Gfx_VulkanInstance* instance, Gfx_VulkanDevice* device, GLFWwindow* window, bool clearOP);
		bool Prepare(uint32_t width, uint32_t height);
		void Create(uint32_t* width, uint32_t* height, bool vSync = false);
		void ClearColors(VkCommandBuffer cmdBuffer, const glm::vec4& color);
		void CleanUp();

		VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore);
		VkResult QueuePresent(VkQueue queue, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
		void OnResize(uint32_t* width, uint32_t* height, bool vsync, Gfx_CmdBuffer* cmd = nullptr);

		const VkFramebuffer GetCurrentFramebuffer() const;
		const VkImage GetCurrentImage() const;
		const VkSwapchainKHR& GetVkSwapchain() const;
		const VkRenderPass GetVkRenderPass() const;
		const VkSurfaceKHR GetVkSurface() const;
		const VkFormat& GetColorFormat() const;
		const VkFormat& GetDepthFormat() const;
		const Buffer& GetCurrentBuffer() const;
		uint32_t GetCurrentBufferIndex() const;
		uint32_t& GetCurrentBufferIndexRef();
		uint32_t GetHeight() const;
		uint32_t GetWidth() const;

	private:
		VkResult CreateFramebuffers(uint32_t width, uint32_t height);
		VkResult InitSurface(GLFWwindow* window);
		VkResult CreateRenderPass(bool clearOP);
		VkResult CreateDepthStencil();
		void GetPtrs();
		void FindColorSpaceFormat();
		void FindDepthStencilFormat();
		void FreeResources();

	private:
		DepthStencil* m_DepthStencil;
		VkRenderPass m_RenderPass;
		VkSwapchainKHR m_Swapchain;
		VkPipelineCache m_PipelineCash;
		Gfx_VulkanInstance* m_Instance;
		Gfx_VulkanDevice* m_Device;
		VkSurfaceKHR m_Surface;
		VkSurfaceFormatKHR* m_SurfaceFormat;
		VkPresentModeKHR* m_PresentMode;

		uint32_t m_PresentModeCount;
		uint32_t m_SurfaceFormatCount;
		uint32_t m_ImageCount;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_QueueNodeIndex;
		uint32_t m_CurrentBufferIndex;

		VkClearAttachment m_ClearAttachments[2];

		VkFormat m_ColorFormat;
		VkFormat m_DepthBufferFormat;
		VkColorSpaceKHR m_ColorSpace;

		std::vector<VkImage> m_Images;
		std::vector<VkFramebuffer> m_Framebuffers;
		std::vector<Buffer> m_Buffers;
	};
}