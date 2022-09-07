#pragma once
#include "Backend/Gfx_VulkanCore.h"

namespace SmolEngine
{
	class Gfx_VulkanInstance
	{
	public:
		Gfx_VulkanInstance();
		~Gfx_VulkanInstance();

		void                     Init();
		const VkInstance         GetInstance() const;

	private:
		bool                     CreateAppInfo();
		bool                     CreateInstance(const VkApplicationInfo& info);

	private:
		VkInstance               m_Instance;
		VkDebugUtilsMessengerEXT m_Messenger;
		std::vector<const char*> m_Extensions;

		friend class VulkanRendererAPI;
		friend class DenoisePass;
	};
}