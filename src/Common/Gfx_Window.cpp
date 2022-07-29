#include "Gfx_Precompiled.h"
#include "Common/Gfx_Window.h"
#include "Common/Gfx_Events.h"

#include <GLFW/glfw3.h>

namespace Dia
{
	static Gfx_Window::Data* s_DataPtr = nullptr;
	static Gfx_EventSender*  s_EventHandler = nullptr;

	Gfx_Window::Gfx_Window()
	{

	}

	void Gfx_Window::Create(WindowCreateDesc* desc)
	{
		assert(desc != nullptr);

		s_DataPtr = &m_Data;
		s_EventHandler = desc->myEventHandler;

		m_CreateDesc = *desc;
		m_Data.myTitle = desc->myTitle;
		m_Data.myHeight = desc->myHeight;
		m_Data.myWidth = desc->myWidth;

		glfwInit();
		glfwSetErrorCallback([](int error, const char* description) { Gfx_Log::LogError("GLFW Error ({0}): {1}", error, description); });
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		m_Window = glfwCreateWindow((int)desc->myWidth, (int)desc->myHeight, desc->myTitle.c_str(), nullptr, nullptr);
		if (!m_Window)
		{
			RUNTIME_ERROR("Failed to create window!");
		}

		if (desc->myFullscreen)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(m_Window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
		}

		SetVSync(desc->myVSync);

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint)
			{
				CharInputEvent input;
				input.m_Codepoint = codepoint;
				s_EventHandler->SendEvent(input, Gfx_Event::Type::CHAR_INPUT, Gfx_Event::Category::EVENT_KEYBOARD);
			});

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				s_DataPtr->myHeight = height;
				s_DataPtr->myWidth = width;

				WindowResizeEvent resizeEvent(*s_DataPtr);
				s_EventHandler->SendEvent(resizeEvent, Gfx_Event::Type::WINDOW_RESIZE, Gfx_Event::Category::EVENT_APP);
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowCloseEvent closeEvent;
				s_EventHandler->SendEvent(closeEvent, Gfx_Event::Type::WINDOW_CLOSE, Gfx_Event::Category::EVENT_APP);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				KeyEvent keyEvent;

				switch (action)
				{
				case GLFW_PRESS:
				{
					s_EventHandler->SendEvent(keyEvent, Gfx_Event::Type::KEY_PRESS, Gfx_Event::Category::EVENT_KEYBOARD, action, key);
					break;
				}
				case GLFW_RELEASE:
				{
					s_EventHandler->SendEvent(keyEvent, Gfx_Event::Type::KEY_RELEASE, Gfx_Event::Category::EVENT_KEYBOARD, action, key);
					break;
				}
				case GLFW_REPEAT:
				{
					s_EventHandler->SendEvent(keyEvent, Gfx_Event::Type::KEY_REPEAT, Gfx_Event::Category::EVENT_KEYBOARD, action, key);
					break;
				}
				default:
					break;
				}
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				MouseButtonEvent mouseButton(button);

				switch (action)
				{
				case GLFW_PRESS:
				{
					s_EventHandler->SendEvent(mouseButton, Gfx_Event::Type::MOUSE_PRESS, Gfx_Event::Category::EVENT_MOUSE, action, button);
					break;
				}
				case GLFW_RELEASE:
				{
					s_EventHandler->SendEvent(mouseButton, Gfx_Event::Type::MOUSE_RELEASE, Gfx_Event::Category::EVENT_MOUSE, action, button);
					break;
				}
				}

			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				MouseScrollEvent scrollEvent(static_cast<float>(xOffset), static_cast<float>(yOffset));
				s_EventHandler->SendEvent(scrollEvent, Gfx_Event::Type::MOUSE_SCROLL, Gfx_Event::Category::EVENT_MOUSE);
			});


		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
			{
				MouseMoveEvent mouseEvent(xPos, yPos);
				s_EventHandler->SendEvent(mouseEvent, Gfx_Event::Type::MOUSE_MOVE, Gfx_Event::Category::EVENT_MOUSE);
			});

	}

	void Gfx_Window::ProcessEvents()
	{
		if (!glfwWindowShouldClose(m_Window))
		{
			glfwPollEvents();
		}
	}

	void Gfx_Window::SetWidth(uint32_t value)
	{
		glfwSetWindowSize(m_Window, static_cast<int>(value), static_cast<int>(m_Data.myHeight));
	}

	void Gfx_Window::SetHeight(uint32_t value)
	{
		glfwSetWindowSize(m_Window, static_cast<int>(m_Data.myWidth), static_cast<int>(value));
	}

	GLFWwindow* Gfx_Window::GetNativeWindow() const
	{
		return m_Window;
	}

	void Gfx_Window::SetTitle(const std::string& name)
	{
		glfwSetWindowTitle(m_Window, name.c_str());
	}

	Gfx_Window::Data* Gfx_Window::GetData()
	{
		return &m_Data;
	}

	const WindowCreateDesc& Gfx_Window::GetCreateDesc() const
	{
		return m_CreateDesc;
	}

	uint32_t Gfx_Window::GetWidth() const
	{
		return m_Data.myWidth;
	}

	uint32_t Gfx_Window::GetHeight() const
	{
		return m_Data.myHeight;
	}

	void Gfx_Window::ShutDown()
	{
		glfwDestroyWindow(m_Window);
	}

	void Gfx_Window::SetVSync(bool enabled)
	{
		Gfx_Log::LogInfo("VSync enabled: {0}", enabled);
		enabled ? glfwSwapInterval(1) : glfwSwapInterval(0);
	}
}