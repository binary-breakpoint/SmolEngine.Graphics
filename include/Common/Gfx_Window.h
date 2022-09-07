#pragma once
#include "Common/Gfx_Memory.h"

#include <string>

struct GLFWwindow;

namespace SmolEngine
{
	class Gfx_EventSender;

	struct WindowCreateDesc
	{
		Gfx_EventSender* myEventHandler = nullptr;
		uint32_t myWidth = 720;
		uint32_t myHeight = 480;
		std::string myTitle = "SomeApplication";
		bool myVSync = false;
		bool myFullscreen = false;
		bool myTargetsSwapchain = true;
		bool myAutoResize = true;
	};

	class Gfx_Window
	{
	public:
		struct Data
		{
			uint32_t myWidth;
			uint32_t myHeight;
			std::string myTitle;
		};

		Gfx_Window();

		void Create(WindowCreateDesc* desc);
		void ProcessEvents();
		void ShutDown();

		Data* GetData();
		const WindowCreateDesc& GetCreateDesc() const;
		GLFWwindow* GetNativeWindow() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

		void SetWidth(uint32_t value);
		void SetHeight(uint32_t value);
		void SetVSync(bool enabled);
		void SetTitle(const std::string& name);

	private:
		WindowCreateDesc m_CreateDesc;
		GLFWwindow* m_Window;
		Data m_Data;
	};

}
