#pragma once
#include "Backend/Gfx_VulkanCore.h"
#include "Common/Gfx_Events.h"

struct GLFWwindow;

namespace Dia
{
	class Gfx_VulkanCmdBuffer;
	class Gfx_VulkanSwapchain;

	class Gfx_VulkanImGui
	{
	public:
		void Create();
		void ShutDown();
		void NewFrame();
		void EndFrame();
		void OnEvent(Gfx_Event& e);
		void Draw(Gfx_VulkanSwapchain* target);

	private:
		void OnSetup();

	private:
		VkPipelineCache m_PipelineCache;
		VkDescriptorPool m_DescriptorPool;
		VkDevice m_Device;
	};
}