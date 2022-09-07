#pragma once
#include "Common/Gfx_Memory.h"
#include "Common/Gfx_Events.h"

namespace SmolEngine
{
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}

	typedef enum class MouseCode : uint16_t
	{
		// From glfw3.h
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Button6 = 6,
		Button7 = 7,

		ButtonLast = Button7,
		ButtonLeft = Button0,
		ButtonRight = Button1,
		ButtonMiddle = Button2
	} Mouse;

	inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
	{
		os << static_cast<int32_t>(mouseCode);
		return os;
	}

// From glfw3.h
#define F_KEY_SPACE           ::SmolEngine::Key::Space
#define F_KEY_APOSTROPHE      ::SmolEngine::Key::Apostrophe    /* ' */
#define F_KEY_COMMA           ::SmolEngine::Key::Comma         /* , */
#define F_KEY_MINUS           ::SmolEngine::Key::Minus         /* - */
#define F_KEY_PERIOD          ::SmolEngine::Key::Period        /* . */
#define F_KEY_SLASH           ::SmolEngine::Key::Slash         /* / */
#define F_KEY_0               ::SmolEngine::Key::D0
#define F_KEY_1               ::SmolEngine::Key::D1
#define F_KEY_2               ::SmolEngine::Key::D2
#define F_KEY_3               ::SmolEngine::Key::D3
#define F_KEY_4               ::SmolEngine::Key::D4
#define F_KEY_5               ::SmolEngine::Key::D5
#define F_KEY_6               ::SmolEngine::Key::D6
#define F_KEY_7               ::SmolEngine::Key::D7
#define F_KEY_8               ::SmolEngine::Key::D8
#define F_KEY_9               ::SmolEngine::Key::D9
#define F_KEY_SEMICOLON       ::SmolEngine::Key::Semicolon     /* ; */
#define F_KEY_EQUAL           ::SmolEngine::Key::Equal         /* = */
#define F_KEY_A               ::SmolEngine::Key::A
#define F_KEY_B               ::SmolEngine::Key::B
#define F_KEY_C               ::SmolEngine::Key::C
#define F_KEY_D               ::SmolEngine::Key::D
#define F_KEY_E               ::SmolEngine::Key::E
#define F_KEY_F               ::SmolEngine::Key::F
#define F_KEY_G               ::SmolEngine::Key::G
#define F_KEY_H               ::SmolEngine::Key::H
#define F_KEY_I               ::SmolEngine::Key::I
#define F_KEY_J               ::SmolEngine::Key::J
#define F_KEY_K               ::SmolEngine::Key::K
#define F_KEY_L               ::SmolEngine::Key::L
#define F_KEY_M               ::SmolEngine::Key::M
#define F_KEY_N               ::SmolEngine::Key::N
#define F_KEY_O               ::SmolEngine::Key::O
#define F_KEY_P               ::SmolEngine::Key::P
#define F_KEY_Q               ::SmolEngine::Key::Q
#define F_KEY_R               ::SmolEngine::Key::R
#define F_KEY_S               ::SmolEngine::Key::S
#define F_KEY_T               ::SmolEngine::Key::T
#define F_KEY_U               ::SmolEngine::Key::U
#define F_KEY_V               ::SmolEngine::Key::V
#define F_KEY_W               ::SmolEngine::Key::W
#define F_KEY_X               ::SmolEngine::Key::X
#define F_KEY_Y               ::SmolEngine::Key::Y
#define F_KEY_Z               ::SmolEngine::Key::Z
#define F_KEY_LEFT_BRACKET    ::SmolEngine::Key::LeftBracket   /* [ */
#define F_KEY_BACKSLASH       ::SmolEngine::Key::Backslash     /* \ */
#define F_KEY_RIGHT_BRACKET   ::SmolEngine::Key::RightBracket  /* ] */
#define F_KEY_GRAVE_ACCENT    ::SmolEngine::Key::GraveAccent   /* ` */
#define F_KEY_WORLD_1         ::SmolEngine::Key::World1        /* non-US #1 */
#define F_KEY_WORLD_2         ::SmolEngine::Key::World2        /* non-US #2 */

/* FunctFn keys */
#define F_KEY_ESCAPE          ::SmolEngine::Key::Escape
#define F_KEY_ENTER           ::SmolEngine::Key::Enter
#define F_KEY_TAB             ::SmolEngine::Key::Tab
#define F_KEY_BACKSPACE       ::SmolEngine::Key::Backspace
#define F_KEY_INSERT          ::SmolEngine::Key::Insert
#define F_KEY_DELETE          ::SmolEngine::Key::Delete
#define F_KEY_RIGHT           ::SmolEngine::Key::Right
#define F_KEY_LEFT            ::SmolEngine::Key::Left
#define F_KEY_DOWN            ::SmolEngine::Key::Down
#define F_KEY_UP              ::SmolEngine::Key::Up
#define F_KEY_PAGE_UP         ::SmolEngine::Key::PageUp
#define F_KEY_PAGE_DOWN       ::SmolEngine::Key::PageDown
#define F_KEY_HOME            ::SmolEngine::Key::Home
#define F_KEY_END             ::SmolEngine::Key::End
#define F_KEY_CAPS_LOCK       ::SmolEngine::Key::CapsLock
#define F_KEY_SCROLL_LOCK     ::SmolEngine::Key::ScrollLock
#define F_KEY_NUM_LOCK        ::SmolEngine::Key::NumLock
#define F_KEY_PRINT_SCREEN    ::SmolEngine::Key::PrintScreen
#define F_KEY_PAUSE           ::SmolEngine::Key::Pause
#define F_KEY_F1              ::SmolEngine::Key::F1
#define F_KEY_F2              ::SmolEngine::Key::F2
#define F_KEY_F3              ::SmolEngine::Key::F3
#define F_KEY_F4              ::SmolEngine::Key::F4
#define F_KEY_F5              ::SmolEngine::Key::F5
#define F_KEY_F6              ::SmolEngine::Key::F6
#define F_KEY_F7              ::SmolEngine::Key::F7
#define F_KEY_F8              ::SmolEngine::Key::F8
#define F_KEY_F9              ::SmolEngine::Key::F9
#define F_KEY_F10             ::SmolEngine::Key::F10
#define F_KEY_F11             ::SmolEngine::Key::F11
#define F_KEY_F12             ::SmolEngine::Key::F12
#define F_KEY_F13             ::SmolEngine::Key::F13
#define F_KEY_F14             ::SmolEngine::Key::F14
#define F_KEY_F15             ::SmolEngine::Key::F15
#define F_KEY_F16             ::SmolEngine::Key::F16
#define F_KEY_F17             ::SmolEngine::Key::F17
#define F_KEY_F18             ::SmolEngine::Key::F18
#define F_KEY_F19             ::SmolEngine::Key::F19
#define F_KEY_F20             ::SmolEngine::Key::F20
#define F_KEY_F21             ::SmolEngine::Key::F21
#define F_KEY_F22             ::SmolEngine::Key::F22
#define F_KEY_F23             ::SmolEngine::Key::F23
#define F_KEY_F24             ::SmolEngine::Key::F24
#define F_KEY_F25             ::SmolEngine::Key::F25
/* Keypa*/

#define F_KEY_KP_0            ::SmolEngine::Key::KP0
#define F_KEY_KP_1            ::SmolEngine::Key::KP1
#define F_KEY_KP_2            ::SmolEngine::Key::KP2
#define F_KEY_KP_3            ::SmolEngine::Key::KP3
#define F_KEY_KP_4            ::SmolEngine::Key::KP4
#define F_KEY_KP_5            ::SmolEngine::Key::KP5
#define F_KEY_KP_6            ::SmolEngine::Key::KP6
#define F_KEY_KP_7            ::SmolEngine::Key::KP7
#define F_KEY_KP_8            ::SmolEngine::Key::KP8
#define F_KEY_KP_9            ::SmolEngine::Key::KP9
#define F_KEY_KP_DECIMAL      ::SmolEngine::Key::KPDecimal
#define F_KEY_KP_DIVIDE       ::SmolEngine::Key::KPDivide
#define F_KEY_KP_MULTIPLY     ::SmolEngine::Key::KPMultiply
#define F_KEY_KP_SUBTRACT     ::SmolEngine::Key::KPSubtract
#define F_KEY_KP_ADD          ::SmolEngine::Key::KPAdd
#define F_KEY_KP_ENTER        ::SmolEngine::Key::KPEnter
#define F_KEY_KP_EQUAL        ::SmolEngine::Key::KPEqual

#define F_KEY_LEFT_SHIFT      ::SmolEngine::Key::LeftShift
#define F_KEY_LEFT_CONTROL    ::SmolEngine::Key::LeftControl
#define F_KEY_LEFT_ALT        ::SmolEngine::Key::LeftAlt
#define F_KEY_LEFT_SUPER      ::SmolEngine::Key::LeftSuper
#define F_KEY_RIGHT_SHIFT     ::SmolEngine::Key::RightShift
#define F_KEY_RIGHT_CONTROL   ::SmolEngine::Key::RightControl
#define F_KEY_RIGHT_ALT       ::SmolEngine::Key::RightAlt
#define F_KEY_RIGHT_SUPER     ::SmolEngine::Key::RightSuper
#define F_KEY_MENU            ::SmolEngine::Key::Menu

#define F_MOUSE_BUTTON_0      ::SmolEngine::Mouse::Button0
#define F_MOUSE_BUTTON_1      ::SmolEngine::Mouse::Button1
#define F_MOUSE_BUTTON_2      ::SmolEngine::Mouse::Button2
#define F_MOUSE_BUTTON_3      ::SmolEngine::Mouse::Button3
#define F_MOUSE_BUTTON_4      ::SmolEngine::Mouse::Button4
#define F_MOUSE_BUTTON_5      ::SmolEngine::Mouse::Button5
#define F_MOUSE_BUTTON_6      ::SmolEngine::Mouse::Button6
#define F_MOUSE_BUTTON_7      ::SmolEngine::Mouse::Button7
#define F_MOUSE_BUTTON_LAST   ::SmolEngine::Mouse::ButtonLast
#define F_MOUSE_BUTTON_LEFT   ::SmolEngine::Mouse::ButtonLeft
#define F_MOUSE_BUTTON_RIGHT  ::SmolEngine::Mouse::ButtonRight
#define F_MOUSE_BUTTON_MIDDLE ::SmolEngine::Mouse::ButtonMiddle

	class Gfx_Input
	{
	public:

		// Helpers
		static bool IsKeyPressed(KeyCode key);
		static bool IsKeyReleased(KeyCode key);
		static bool IsMouseButtonPressed(MouseCode button);
		static bool IsMouseButtonReleased(MouseCode button);

		// Getters
		static float GetMouseX();
		static float GetMouseY();
		static std::pair<float, float> GetMousePosition();
	};
}