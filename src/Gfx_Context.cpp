#include "Gfx_Precompiled.h"
#include "Gfx_Context.h"
#include "Common/Gfx_Input.h"
#include "Common/Gfx_Framebuffer.h"
#include "Common/Gfx_Sampler.h"

#include "Tools/Gfx_ShaderIncluder.h"

#include <GLFW/glfw3.h>

namespace SmolEngine
{
	Gfx_Context* Gfx_Context::s_Instance = nullptr;

	Gfx_Context::Gfx_Context()
	{
		s_Instance = this;
	}

	Gfx_Context::~Gfx_Context()
	{
		s_Instance = nullptr;

		Shutdown();
	}

	void Gfx_Context::Create(GfxContextCreateDesc* desc)
	{
		assert(desc != nullptr);
		assert(desc->myWindowDesc != nullptr);

		m_Desc = *desc;
		m_Root = desc->myAssetPath;
		m_EventHandler.OnEventFn = std::bind(&Gfx_Context::OnEvent, this, std::placeholders::_1);

		WindowCreateDesc* winDesc = desc->myWindowDesc;

		m_Window = std::make_shared<Gfx_Window>();
		desc->myWindowDesc->myEventHandler = &m_EventHandler;
		m_Window->Create(winDesc);

		CreateAPIContext();

		m_ShaderIncluder = std::make_shared<Gfx_ShaderIncluder>();
		m_ShaderIncluder->AddIncludeDir(m_Root + "Shaders/");
		m_ShaderIncluder->AddIncludeDir(m_Root + "../tests/shaders");

		m_World = std::make_shared<Gfx_World>();

		// Creates default framebuffer
		{
			FramebufferCreateDesc fbDesc = {};
			fbDesc.mySampler = Gfx_World::GetSampler();
			fbDesc.myWidth = winDesc->myWidth;
			fbDesc.myHeight = winDesc->myHeight;
			fbDesc.myIsTargetsSwapchain = winDesc->myTargetsSwapchain;
			fbDesc.myIsResizable = true;
			fbDesc.myIsAutoSync = false;
			fbDesc.myIsUsedByImGui = true;

			FramebufferAttachment colorAttachment = FramebufferAttachment(Format::B8G8R8A8_UNORM, glm::vec4(0), "COLOR_0");
			FramebufferAttachment depthAttachment = FramebufferAttachment(Format::D32_SFLOAT_S8_UINT, glm::vec4(1), "Depth_0");

			fbDesc.myAttachments = { colorAttachment, depthAttachment};

			m_Framebuffer = std::make_shared<Gfx_Framebuffer>();
			m_Framebuffer->Create(&fbDesc);
		}
	}

	void Gfx_Context::SwapBuffers()
	{
		// ImGUI pass
		if ((m_Desc.myFeaturesFlags
			& FeaturesFlags::ImguiEnable) == FeaturesFlags::ImguiEnable) [[unlikely]]
		{
			m_ImGuiContext->EndFrame();
			m_ImGuiContext->Draw(&m_Swapchain);
		}

		const auto& present_ref = m_Semaphore.GetPresentCompleteSemaphore();
		const auto& render_ref = m_Semaphore.GetRenderCompleteSemaphore();

		VK_CHECK_RESULT(vkWaitForFences(m_Device.GetLogicalDevice(), 1,
			&m_Semaphore.GetVkFences()[m_Swapchain.GetCurrentBufferIndex()], VK_TRUE, UINT64_MAX));
		VK_CHECK_RESULT(vkResetFences(m_Device.GetLogicalDevice(), 1, 
			&m_Semaphore.GetVkFences()[m_Swapchain.GetCurrentBufferIndex()]));

		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo = {};
		{
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.pWaitDstStageMask = &waitStageMask;
			submitInfo.pWaitSemaphores = &present_ref;
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &render_ref;
			submitInfo.signalSemaphoreCount = 1;  
			submitInfo.pCommandBuffers = &m_CmdBuffer.m_Buffer; 
			submitInfo.commandBufferCount = 1;
		}

		m_CmdBuffer.CmdEndRecord();
		m_CmdBuffer.m_State = Gfx_CmdBuffer::State::Wait;

		VK_CHECK_RESULT(vkQueueSubmit(m_Device.GetQueue(Gfx_VulkanDevice::QueueFamilyFlags::Graphics),
		 1, &submitInfo, m_Semaphore.GetVkFences()[m_Swapchain.GetCurrentBufferIndex()]));

		{
			VkResult present = m_Swapchain.QueuePresent(m_Device.GetQueue(Gfx_VulkanDevice::QueueFamilyFlags::Graphics), render_ref);
			if (!((present == VK_SUCCESS) || (present == VK_SUBOPTIMAL_KHR))) {

				if (present == VK_ERROR_OUT_OF_DATE_KHR)
				{
					uint32_t w = m_Swapchain.GetWidth();
					uint32_t h = m_Swapchain.GetHeight();

					m_Swapchain.OnResize(&w, &h, &m_CmdBuffer);
					return;
				}
				else
				{
					VK_CHECK_RESULT(present);
				}
			}

			VkResult result = (vkWaitForFences(m_Device.GetLogicalDevice(),
				1, &m_Semaphore.GetVkFences()[m_Swapchain.GetCurrentBufferIndex()], VK_TRUE, UINT64_MAX));
#ifdef  SMOLENGINE_DEBUG
			if (VK_ERROR_DEVICE_LOST == result)
			{
				/* Device lost notification is asynchronous to the NVIDIA display
				   driver's GPU crash handling. Give the Nsight Aftermath GPU crash dump
				   thread some time to do its work before terminating the process. */
				std::this_thread::sleep_for(std::chrono::microseconds(3000));
			}
#endif
		}

		m_CmdBuffer.Free();
	}

	void Gfx_Context::ProcessEvents()
	{
		m_Window->ProcessEvents();
	}

	void Gfx_Context::BeginFrame(float time)
	{
		m_DeltaTime = time;

		if ((m_Desc.myFeaturesFlags
			& FeaturesFlags::ImguiEnable) == FeaturesFlags::ImguiEnable) [[unlikely]] { m_ImGuiContext->NewFrame(); }

		VK_CHECK_RESULT(m_Swapchain.AcquireNextImage(m_Semaphore.GetPresentCompleteSemaphore()));

		CmdBufferCreateDesc cmdDesc{};
		m_CmdBuffer.Create(&cmdDesc);
		m_CmdBuffer.CmdBeginRecord();
	}

	void Gfx_Context::Shutdown()
	{
		if ((m_Desc.myFeaturesFlags
			& FeaturesFlags::ImguiEnable) == FeaturesFlags::ImguiEnable) [[unlikely]] { m_ImGuiContext->ShutDown(); }

		m_Window->ShutDown();
    }

	void Gfx_Context::Resize(uint32_t* width, uint32_t* height)
	{
		const WindowCreateDesc& winDesc = m_Window->GetCreateDesc();

		m_Swapchain.OnResize(width, height, winDesc.myVSync, &m_CmdBuffer);

		if (winDesc.myAutoResize) [[likely]]
		{
			SetFramebufferSize(*width, *height);
		}
	}

	void Gfx_Context::SetEventCallback(std::function<void(Gfx_Event&)> callback)
	{
		m_EventCallback = callback;
	}

	void Gfx_Context::SetFramebufferSize(uint32_t width, uint32_t height)
	{
		m_Framebuffer->OnResize(width, height);
	}

	float Gfx_Context::CalculateDeltaTime()
	{
		float time = (float)glfwGetTime();
		float deltaTime = time - m_LastFrameTime;
		m_LastFrameTime = time;
		return deltaTime;
	}

	bool Gfx_Context::IsWindowMinimized() const
	{
		return m_bWindowMinimized;
	}

	bool Gfx_Context::IsOpen() const
	{
		return m_bOpen;
	}

	Gfx_Window* Gfx_Context::GetWindow() const
	{
		return m_Window.get();
	}

	glm::vec2 Gfx_Context::GetWindowSize() const
	{
		return glm::vec2(m_Window->GetWidth(), m_Window->GetHeight());
	}

	float Gfx_Context::GetGltfTime() const
	{
		return (float)glfwGetTime();
	}

	float Gfx_Context::GetDeltaTime() const
	{
		return m_DeltaTime;
	}

	const std::string& Gfx_Context::GetAssetsPath() const
	{
		return m_Root;
	}

	void Gfx_Context::CreateAPIContext()
	{
		GFX_ASSERT(glfwVulkanSupported() == GLFW_TRUE)

		m_Instance.Init();

#ifdef AFTERMATH
		// Enable Nsight Aftermath GPU crash dump creation.
		// This needs to be done before the Vulkan device is created.
		m_CrachTracker.Initialize();
#endif
		m_Device.Init(&m_Instance);

		m_Allocator = new Gfx_VulkanAllocator();
		m_Allocator->Init(&m_Device, &m_Instance);

		const WindowCreateDesc& winDesc = m_Window->GetCreateDesc();

		bool initialized = m_Swapchain.Init(&m_Instance, &m_Device, GetWindow()->GetNativeWindow(), !winDesc.myTargetsSwapchain);

		GFX_ASSERT_MSG(initialized, "Couldn't create swapchain!")

		uint32_t* width = &m_Window->GetData()->myWidth;
		uint32_t* height = &m_Window->GetData()->myHeight;

		m_Swapchain.Create(width, height, winDesc.myVSync);
		m_Semaphore.Create(&m_Device);
		m_Swapchain.Prepare(*width, *height);

		// Initialize ImGUI
		if ((m_Desc.myFeaturesFlags &
			FeaturesFlags::ImguiEnable) == FeaturesFlags::ImguiEnable) [[unlikely]]
		{
			m_ImGuiContext = std::make_shared<Gfx_VulkanImGui>();
			m_ImGuiContext->Create();
		}
	}

	void Gfx_Context::OnEvent(Gfx_Event& e)
	{
		if (e.IsType(Gfx_Event::Type::WINDOW_CLOSE)) { m_bOpen = false; }
		if (e.IsType(Gfx_Event::Type::WINDOW_RESIZE))
		{
			WindowResizeEvent* resize = e.Cast<WindowResizeEvent>();
			uint32_t width = resize->GetWidth();
			uint32_t height = resize->GetHeight();

			if (width == 0 || height == 0)
				m_bWindowMinimized = true;
			else
			{
				m_bWindowMinimized = false;
				Resize(&width, &height);
			}
		}

		if(m_EventCallback != nullptr)
			m_EventCallback(std::forward<Gfx_Event&>(e));
	}

	Gfx_Context* Gfx_Context::GetSingleton()
	{
		return s_Instance;
	}

	Ref<Gfx_Framebuffer> Gfx_Context::GetFramebuffer()
	{
		return m_Framebuffer;
	}

	Ref<Gfx_World> Gfx_Context::GetWorld()
	{
		return Gfx_Context::GetSingleton()->m_World;
	}

	Gfx_VulkanSemaphore& Gfx_Context::GetSemaphore()
	{
		return Gfx_Context::GetSingleton()->m_Semaphore;
	}

	Gfx_CmdBuffer* Gfx_Context::GetCommandBuffer()
	{
		return &Gfx_Context::GetSingleton()->m_CmdBuffer;
	}

	Gfx_VulkanSwapchain& Gfx_Context::GetSwapchain()
	{
		return Gfx_Context::GetSingleton()->m_Swapchain;
	}

	Gfx_VulkanInstance& Gfx_Context::GetInstance()
	{
		return Gfx_Context::GetSingleton()->m_Instance;
	}

	Gfx_VulkanDevice& Gfx_Context::GetDevice()
	{
		return Gfx_Context::GetSingleton()->m_Device;
	}

}