#pragma once
#include "Common/Gfx_Memory.h"
#include "Common/Gfx_Events.h"

namespace Dia
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
#define F_KEY_SPACE           ::Dia::Key::Space
#define F_KEY_APOSTROPHE      ::Dia::Key::Apostrophe    /* ' */
#define F_KEY_COMMA           ::Dia::Key::Comma         /* , */
#define F_KEY_MINUS           ::Dia::Key::Minus         /* - */
#define F_KEY_PERIOD          ::Dia::Key::Period        /* . */
#define F_KEY_SLASH           ::Dia::Key::Slash         /* / */
#define F_KEY_0               ::Dia::Key::D0
#define F_KEY_1               ::Dia::Key::D1
#define F_KEY_2               ::Dia::Key::D2
#define F_KEY_3               ::Dia::Key::D3
#define F_KEY_4               ::Dia::Key::D4
#define F_KEY_5               ::Dia::Key::D5
#define F_KEY_6               ::Dia::Key::D6
#define F_KEY_7               ::Dia::Key::D7
#define F_KEY_8               ::Dia::Key::D8
#define F_KEY_9               ::Dia::Key::D9
#define F_KEY_SEMICOLON       ::Dia::Key::Semicolon     /* ; */
#define F_KEY_EQUAL           ::Dia::Key::Equal         /* = */
#define F_KEY_A               ::Dia::Key::A
#define F_KEY_B               ::Dia::Key::B
#define F_KEY_C               ::Dia::Key::C
#define F_KEY_D               ::Dia::Key::D
#define F_KEY_E               ::Dia::Key::E
#define F_KEY_F               ::Dia::Key::F
#define F_KEY_G               ::Dia::Key::G
#define F_KEY_H               ::Dia::Key::H
#define F_KEY_I               ::Dia::Key::I
#define F_KEY_J               ::Dia::Key::J
#define F_KEY_K               ::Dia::Key::K
#define F_KEY_L               ::Dia::Key::L
#define F_KEY_M               ::Dia::Key::M
#define F_KEY_N               ::Dia::Key::N
#define F_KEY_O               ::Dia::Key::O
#define F_KEY_P               ::Dia::Key::P
#define F_KEY_Q               ::Dia::Key::Q
#define F_KEY_R               ::Dia::Key::R
#define F_KEY_S               ::Dia::Key::S
#define F_KEY_T               ::Dia::Key::T
#define F_KEY_U               ::Dia::Key::U
#define F_KEY_V               ::Dia::Key::V
#define F_KEY_W               ::Dia::Key::W
#define F_KEY_X               ::Dia::Key::X
#define F_KEY_Y               ::Dia::Key::Y
#define F_KEY_Z               ::Dia::Key::Z
#define F_KEY_LEFT_BRACKET    ::Dia::Key::LeftBracket   /* [ */
#define F_KEY_BACKSLASH       ::Dia::Key::Backslash     /* \ */
#define F_KEY_RIGHT_BRACKET   ::Dia::Key::RightBracket  /* ] */
#define F_KEY_GRAVE_ACCENT    ::Dia::Key::GraveAccent   /* ` */
#define F_KEY_WORLD_1         ::Dia::Key::World1        /* non-US #1 */
#define F_KEY_WORLD_2         ::Dia::Key::World2        /* non-US #2 */

/* FunctFn keys */
#define F_KEY_ESCAPE          ::Dia::Key::Escape
#define F_KEY_ENTER           ::Dia::Key::Enter
#define F_KEY_TAB             ::Dia::Key::Tab
#define F_KEY_BACKSPACE       ::Dia::Key::Backspace
#define F_KEY_INSERT          ::Dia::Key::Insert
#define F_KEY_DELETE          ::Dia::Key::Delete
#define F_KEY_RIGHT           ::Dia::Key::Right
#define F_KEY_LEFT            ::Dia::Key::Left
#define F_KEY_DOWN            ::Dia::Key::Down
#define F_KEY_UP              ::Dia::Key::Up
#define F_KEY_PAGE_UP         ::Dia::Key::PageUp
#define F_KEY_PAGE_DOWN       ::Dia::Key::PageDown
#define F_KEY_HOME            ::Dia::Key::Home
#define F_KEY_END             ::Dia::Key::End
#define F_KEY_CAPS_LOCK       ::Dia::Key::CapsLock
#define F_KEY_SCROLL_LOCK     ::Dia::Key::ScrollLock
#define F_KEY_NUM_LOCK        ::Dia::Key::NumLock
#define F_KEY_PRINT_SCREEN    ::Dia::Key::PrintScreen
#define F_KEY_PAUSE           ::Dia::Key::Pause
#define F_KEY_F1              ::Dia::Key::F1
#define F_KEY_F2              ::Dia::Key::F2
#define F_KEY_F3              ::Dia::Key::F3
#define F_KEY_F4              ::Dia::Key::F4
#define F_KEY_F5              ::Dia::Key::F5
#define F_KEY_F6              ::Dia::Key::F6
#define F_KEY_F7              ::Dia::Key::F7
#define F_KEY_F8              ::Dia::Key::F8
#define F_KEY_F9              ::Dia::Key::F9
#define F_KEY_F10             ::Dia::Key::F10
#define F_KEY_F11             ::Dia::Key::F11
#define F_KEY_F12             ::Dia::Key::F12
#define F_KEY_F13             ::Dia::Key::F13
#define F_KEY_F14             ::Dia::Key::F14
#define F_KEY_F15             ::Dia::Key::F15
#define F_KEY_F16             ::Dia::Key::F16
#define F_KEY_F17             ::Dia::Key::F17
#define F_KEY_F18             ::Dia::Key::F18
#define F_KEY_F19             ::Dia::Key::F19
#define F_KEY_F20             ::Dia::Key::F20
#define F_KEY_F21             ::Dia::Key::F21
#define F_KEY_F22             ::Dia::Key::F22
#define F_KEY_F23             ::Dia::Key::F23
#define F_KEY_F24             ::Dia::Key::F24
#define F_KEY_F25             ::Dia::Key::F25
/* Keypa*/

#define F_KEY_KP_0            ::Dia::Key::KP0
#define F_KEY_KP_1            ::Dia::Key::KP1
#define F_KEY_KP_2            ::Dia::Key::KP2
#define F_KEY_KP_3            ::Dia::Key::KP3
#define F_KEY_KP_4            ::Dia::Key::KP4
#define F_KEY_KP_5            ::Dia::Key::KP5
#define F_KEY_KP_6            ::Dia::Key::KP6
#define F_KEY_KP_7            ::Dia::Key::KP7
#define F_KEY_KP_8            ::Dia::Key::KP8
#define F_KEY_KP_9            ::Dia::Key::KP9
#define F_KEY_KP_DECIMAL      ::Dia::Key::KPDecimal
#define F_KEY_KP_DIVIDE       ::Dia::Key::KPDivide
#define F_KEY_KP_MULTIPLY     ::Dia::Key::KPMultiply
#define F_KEY_KP_SUBTRACT     ::Dia::Key::KPSubtract
#define F_KEY_KP_ADD          ::Dia::Key::KPAdd
#define F_KEY_KP_ENTER        ::Dia::Key::KPEnter
#define F_KEY_KP_EQUAL        ::Dia::Key::KPEqual

#define F_KEY_LEFT_SHIFT      ::Dia::Key::LeftShift
#define F_KEY_LEFT_CONTROL    ::Dia::Key::LeftControl
#define F_KEY_LEFT_ALT        ::Dia::Key::LeftAlt
#define F_KEY_LEFT_SUPER      ::Dia::Key::LeftSuper
#define F_KEY_RIGHT_SHIFT     ::Dia::Key::RightShift
#define F_KEY_RIGHT_CONTROL   ::Dia::Key::RightControl
#define F_KEY_RIGHT_ALT       ::Dia::Key::RightAlt
#define F_KEY_RIGHT_SUPER     ::Dia::Key::RightSuper
#define F_KEY_MENU            ::Dia::Key::Menu

#define F_MOUSE_BUTTON_0      ::Dia::Mouse::Button0
#define F_MOUSE_BUTTON_1      ::Dia::Mouse::Button1
#define F_MOUSE_BUTTON_2      ::Dia::Mouse::Button2
#define F_MOUSE_BUTTON_3      ::Dia::Mouse::Button3
#define F_MOUSE_BUTTON_4      ::Dia::Mouse::Button4
#define F_MOUSE_BUTTON_5      ::Dia::Mouse::Button5
#define F_MOUSE_BUTTON_6      ::Dia::Mouse::Button6
#define F_MOUSE_BUTTON_7      ::Dia::Mouse::Button7
#define F_MOUSE_BUTTON_LAST   ::Dia::Mouse::ButtonLast
#define F_MOUSE_BUTTON_LEFT   ::Dia::Mouse::ButtonLeft
#define F_MOUSE_BUTTON_RIGHT  ::Dia::Mouse::ButtonRight
#define F_MOUSE_BUTTON_MIDDLE ::Dia::Mouse::ButtonMiddle

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