#pragma once

#include "Common/Gfx_Window.h"
#include "Common/Gfx_Flags.h"
#include "Common/Gfx_Events.h"
#include "Common/Gfx_CmdBuffer.h"
#include "Common/Gfx_Texture.h"
#include "Common/Gfx_Window.h"

#include "Backend/Gfx_VulkanAllocator.h"
#include "Backend/Gfx_VulkanInstance.h"
#include "Backend/Gfx_VulkanDevice.h"
#include "Backend/Gfx_VulkanSwapchain.h"
#include "Backend/Gfx_VulkanSemaphore.h"

#include "Backend/Gfx_VulkanImGui.h"

#include <functional>

namespace SmolEngine
{
	class Gfx_Framebuffer;
	class Gfx_ShaderIncluder;
	class Gfx_RenderContext;

	struct GfxContextCreateDesc
	{
		WindowCreateDesc* myWindowDesc = nullptr;
		FeaturesFlags myFeaturesFlags = FeaturesFlags::ImguiEnable | FeaturesFlags::RendererEnable;
		std::string myAssetPath = "";
	};

	class Gfx_App
	{
	public:
		Gfx_App();
		~Gfx_App();
								 
		void Create(GfxContextCreateDesc* desc);
		void ProcessEvents();
		void BeginFrame(float time);
		void SwapBuffers();
		void Shutdown();  
		void Resize(uint32_t* width, uint32_t* height);
		void SetEventCallback(std::function<void(Gfx_Event&)> callback);
		void SetFramebufferSize(uint32_t width, uint32_t height);
		float CalculateDeltaTime();

		static Gfx_VulkanSemaphore& GetSemaphore();
		static Gfx_VulkanSwapchain& GetSwapchain();
		static Gfx_VulkanInstance& GetInstance();
		static Gfx_VulkanDevice& GetDevice();
		static Gfx_App* GetSingleton();
		static Gfx_CmdBuffer* GetCommandBuffer();

		Ref<Gfx_Framebuffer> GetFramebuffer();
		Gfx_Window* GetWindow() const;
		glm::vec2 GetWindowSize() const;
		float GetGltfTime() const;
		float GetDeltaTime() const;
		const std::string& GetAssetsPath() const;
		bool IsWindowMinimized() const;
		bool IsOpen() const;

									      
	private:
		void CreateAPIContext();
		void OnEvent(Gfx_Event& event);

	private:
		static Gfx_App* s_Instance;

		std::function<void(Gfx_Event&)> m_EventCallback;
		Ref<Gfx_ShaderIncluder> m_ShaderIncluder;
		Ref<Gfx_Framebuffer> m_Framebuffer;
		Ref<Gfx_VulkanImGui> m_ImGuiContext;
		Ref<Gfx_RenderContext> m_RenderContext;
		Gfx_VulkanAllocator* m_Allocator;
		Ref<Gfx_Window> m_Window;

		GfxContextCreateDesc m_Desc;
		Gfx_CmdBuffer m_CmdBuffer;
		Gfx_VulkanSwapchain m_Swapchain;
		Gfx_VulkanSemaphore m_Semaphore;
		Gfx_VulkanInstance m_Instance;
		Gfx_VulkanDevice m_Device;
		Gfx_EventSender m_EventHandler;

#ifdef AFTERMATH
		GpuCrashTracker m_CrachTracker{};
#endif
		std::string m_Root;
		float m_LastFrameTime = 1.0f;
		float m_DeltaTime = 0.0f;
		bool m_bWindowMinimized = false;
		bool m_bOpen = true;
	};

#define VK_DESTROY_HANDLE(handle, dtor) if (handle) { dtor(handle, nullptr); handle = VK_NULL_HANDLE; }
#define VK_DESTROY_DEVICE_HANDLE(handle, dtor) if (handle) { dtor(Gfx_App::GetDevice().GetLogicalDevice(), handle, nullptr); handle = VK_NULL_HANDLE; }
}
