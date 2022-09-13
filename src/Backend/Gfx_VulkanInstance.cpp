#include "Gfx_Precompiled.h"
#include "Backend/Gfx_VulkanInstance.h"

#include <assert.h>

namespace SmolEngine
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance_DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		// DLSS - vkCmdCuLaunchKernelNVX: required parameter pLaunchInfo->pParams specified as NULL.
		const int ignoredWarning = 2044605652;
		if(pCallbackData->messageIdNumber != ignoredWarning)
			GFX_LOG(std::string(pCallbackData->pMessage), Gfx_Log::Level::Error)

		return VK_FALSE;
	}

	VkResult VulkanInstance_CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanInstance_DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	Gfx_VulkanInstance::Gfx_VulkanInstance() : 
		m_Instance{nullptr},
		m_Messenger{nullptr}
	{

	}

	Gfx_VulkanInstance::~Gfx_VulkanInstance()
	{
		VulkanInstance_DestroyDebugUtilsMessengerEXT(m_Instance, m_Messenger, nullptr);
	}

	void Gfx_VulkanInstance::Init()
	{
		CreateAppInfo();
	}

	bool Gfx_VulkanInstance::CreateAppInfo()
	{
		VkApplicationInfo appInfo = {};
		{
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

			appInfo.pApplicationName = "SmolEngine";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "SmolEngine";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_3;
		}

		return CreateInstance(appInfo);
	}

	bool Gfx_VulkanInstance::CreateInstance(const VkApplicationInfo& appInfo)
	{
		std::vector<const char*> instanceLayers = {};
		m_Extensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,  "VK_KHR_win32_surface" };

#ifdef DIA_DEBUG
		instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
		m_Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		VkInstanceCreateInfo instanceInfo = {};
		{
			instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			instanceInfo.pApplicationInfo= &appInfo;
			instanceInfo.enabledExtensionCount = static_cast<uint32_t>(m_Extensions.size());
			instanceInfo.ppEnabledExtensionNames = m_Extensions.data();
			instanceInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
			instanceInfo.ppEnabledLayerNames = instanceLayers.data();
		}

		VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_Instance);

#ifdef DIA_DEBUG
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = VulkanInstance_DebugCallback;
		createInfo.pUserData = nullptr;

		if (VulkanInstance_CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_Messenger) != VK_SUCCESS) {
			RUNTIME_ERROR("failed to set up debug messenger!");
		}
#endif

		assert(result == VK_SUCCESS);

		return result == VK_SUCCESS;
	}

	const VkInstance Gfx_VulkanInstance::GetInstance() const
	{
		return m_Instance;
	}
}