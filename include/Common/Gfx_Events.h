#pragma once
#include "Common/Gfx_Window.h"

#include <functional>

namespace SmolEngine
{
	class Gfx_Event
	{
	public:
		enum class Type : uint32_t
		{
			EVENT_TYPE_NONE = 0,

			KEY_PRESS,
			KEY_RELEASE,
			KEY_REPEAT,

			MOUSE_MOVE,
			MOUSE_BUTTON,
			MOUSE_SCROLL,
			MOUSE_PRESS,
			MOUSE_RELEASE,

			WINDOW_CLOSE,
			WINDOW_RESIZE,
			WINDOW_UPDATE,

			CHAR_INPUT,
		};

		enum class Category : uint32_t
		{
			EVENT_NONE = 0,
			EVENT_KEYBOARD,
			EVENT_MOUSE,
			EVENT_APP
		};

		Gfx_Event();

		bool IsType(Type type);
		bool IsCategory(Category category);
		static bool IsEventReceived(Type type, Gfx_Event& event);

		template<typename T>
		T* Cast()
		{
			return (T*)(this);
		}

		uint32_t m_Action;
		uint32_t m_Key;
		Category m_EventCategory;
		Type m_EventType;
		bool m_Handled;
	};

	class Gfx_EventSender
	{
	public:
		void SendEvent(Gfx_Event& event, Gfx_Event::Type eventType, Gfx_Event::Category eventCategory, int action = -1, int key = -1);

	public:
		std::function<void(Gfx_Event& event_to_send)> OnEventFn;
	};

	/*
	   Events
	*/

	class WindowResizeEvent: public Gfx_Event
	{
	public:
		WindowResizeEvent(Gfx_Window::Data& data) :
			m_Data(data) {}

		inline const unsigned int GetWidth() { return m_Data.myWidth; }
		inline const unsigned int GetHeight() { return m_Data.myHeight; }

	private:
		Gfx_Window::Data& m_Data;
	};

	class WindowCloseEvent: public Gfx_Event
	{
	public:
		WindowCloseEvent() = default;
	};

	class CharInputEvent: public Gfx_Event
	{
	public:
		unsigned int m_Codepoint;
	};

	class KeyEvent: public Gfx_Event
	{
	public:
		KeyEvent() = default;
	};

	class MouseMoveEvent: public Gfx_Event
	{
	public:
		MouseMoveEvent(double& xPos, double& yPos)
			:m_xPos(xPos), m_yPos(yPos) {}

		double m_xPos, m_yPos;
	};

	class MouseScrollEvent: public Gfx_Event
	{
	public:
		MouseScrollEvent(float xOffset, float yOffset)
			:m_xOffset(xOffset), m_yOffset(yOffset) {}

		inline const float GetXoffset() { return m_xOffset; }
		inline const float GetYoffset() { return m_yOffset; }

	private:
		float m_xOffset, m_yOffset;
	};

	class MouseButtonEvent: public Gfx_Event
	{
	public:
		MouseButtonEvent(int mouseButton)
			:m_MouseButton(mouseButton) {}

		int m_MouseButton;
	};
}