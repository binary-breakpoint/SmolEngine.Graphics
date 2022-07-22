#pragma once
#ifndef OPENGL_IMPL

#include "Backend/Gfx_VulkanCore.h"

#include <vector>

namespace Dia
{
	class Gfx_VulkanDevice;

	class Gfx_VulkanSemaphore // TODO: refacot
	{
	public:
		Gfx_VulkanSemaphore();

		void Create(const Gfx_VulkanDevice* device);
		static void CreateVkSemaphore(VkSemaphore& outSemapthore);

		// Getters
		const VkSemaphore GetPresentCompleteSemaphore() const;
		const VkSemaphore GetRenderCompleteSemaphore() const;
		const std::vector<VkFence>& GetVkFences() const;
		const VkSubmitInfo* GetSubmitInfo() const;

	private:
		VkSemaphore              m_PresentComplete;
		VkSemaphore              m_RenderComplete;
		VkSubmitInfo             m_SubmitInfo;
		std::vector<VkFence>     m_WaitFences;
	};
}
#endif