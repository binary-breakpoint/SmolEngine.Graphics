#pragma once
#include "Backend/Gfx_VulkanCore.h"

namespace SmolEngine
{
	class Gfx_VulkanInstance
	{
	public:
		Gfx_VulkanInstance();
		~Gfx_VulkanInstance();

		void Create();
		const VkInstance GetInstance() const;

	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_Messenger;
		std::vector<const char*> m_Extensions;
	};
}