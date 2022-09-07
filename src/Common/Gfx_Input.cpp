#include "Gfx_Precompiled.h"
#include "Common/Gfx_Input.h"

#include <GLFW/glfw3.h>

namespace SmolEngine
{
	bool Gfx_Input::IsKeyPressed(KeyCode key)
	{
		GLFWwindow* window = Gfx_Context::GetSingleton()->GetWindow()->GetNativeWindow();
		auto state = glfwGetKey(window, static_cast<int32_t>(key));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Gfx_Input::IsKeyReleased(KeyCode key)
	{
		GLFWwindow* window = Gfx_Context::GetSingleton()->GetWindow()->GetNativeWindow();
		auto state = glfwGetKey(window, static_cast<int32_t>(key));
		return state == GLFW_RELEASE;
	}

	bool Gfx_Input::IsMouseButtonPressed(MouseCode button)
	{
		GLFWwindow* window = Gfx_Context::GetSingleton()->GetWindow()->GetNativeWindow();
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}

	bool Gfx_Input::IsMouseButtonReleased(MouseCode button)
	{
		GLFWwindow* window = Gfx_Context::GetSingleton()->GetWindow()->GetNativeWindow();
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_RELEASE;
	}

	float Gfx_Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return x;
	}

	float Gfx_Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return y;
	}

	std::pair<float, float> Gfx_Input::GetMousePosition()
	{
		GLFWwindow* window = Gfx_Context::GetSingleton()->GetWindow()->GetNativeWindow();
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}
}