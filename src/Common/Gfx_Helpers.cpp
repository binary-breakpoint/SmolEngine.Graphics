#include "Gfx_Precompiled.h"
#include "Common/Gfx_Helpers.h"
#include "Common/Gfx_Input.h"
#include "Common/Gfx_Window.h"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <GLFW/glfw3.h>

namespace Dia
{
	uint32_t Gfx_Helpers::GetFormatSize(Format format)
	{
		switch (format)
		{
		case Format::R8_UNORM:
			return sizeof(uint8_t);
		case Format::R8G8_UNORM:
			return sizeof(uint8_t) * 2;
		case Format::R8G8B8A8_UNORM:
			return sizeof(uint8_t) * 4;
		case Format::B8G8R8A8_UNORM:
			return sizeof(uint8_t) * 4;
		case Format::D16_UNORM:
			return sizeof(uint16_t);
		case Format::D16_UNORM_S8_UINT:
			return sizeof(uint16_t) + sizeof(uint8_t);
		case Format::D32_SFLOAT_S8_UINT:
			return sizeof(float) + sizeof(uint8_t);
		case Format::D32_SFLOAT:
			return sizeof(uint32_t);
		case Format::R8_SNORM:
			return sizeof(uint8_t);
		case Format::R8G8_SNORM:
			return sizeof(uint8_t) * 2;
		case Format::R8G8B8A8_SNORM:
			return sizeof(uint8_t) * 4;
		case Format::R16_UNORM:
			return sizeof(uint16_t);
		case Format::R16G16_UNORM:
			return sizeof(uint16_t) * 2;
		case Format::R16G16B16A16_UNORM:
			return sizeof(uint16_t) * 4;
		case Format::R16_SNORM:
			return sizeof(uint16_t);
		case Format::R16G16_SNORM:
			return sizeof(uint16_t) * 2;
		case Format::R16G16B16A16_SNORM:
			return sizeof(uint16_t) * 4;
		case Format::R16_SFLOAT:
			return sizeof(uint16_t);
		case Format::R16G16_SFLOAT:
			return sizeof(uint16_t) * 2;
		case Format::R16G16B16A16_SFLOAT:
			return sizeof(uint16_t) * 4;
		case Format::R16G16B16_SFLOAT:
			return sizeof(uint16_t) * 3;
		case Format::R32_SFLOAT:
			return sizeof(float);
		case Format::R32G32_SFLOAT:
			return sizeof(float) * 2;
		case Format::R32G32B32A32_SFLOAT:
			return sizeof(float) * 4;
		case Format::R32G32B32_SFLOAT: 
			return sizeof(float) * 3;
		case Format::R16_UINT:
			return sizeof(uint16_t);
		case Format::R16G16_UINT:
			return sizeof(uint16_t) * 2;
		case Format::R16G16B16A16_UINT:
			return sizeof(uint16_t) * 4;
		case Format::R32_UINT:
			return sizeof(uint32_t);
		case Format::R32G32_UINT:
			return sizeof(uint32_t) * 2;
		case Format::R32G32B32A32_UINT:
			return sizeof(uint32_t) * 4;
		case Format::R16_SINT:
			return sizeof(uint16_t);;
		case Format::R16G16_SINT:
			return sizeof(uint16_t) * 2;
		case Format::R16G16B16A16_SINT:
			return sizeof(uint16_t) * 4;
		case Format::R32_SINT:
			return sizeof(uint32_t);
		case Format::R32G32_SINT:
			return sizeof(uint32_t) * 2;
		case Format::R32G32B32A32_SINT:
			return sizeof(uint32_t) * 4;;
		default:
			return 0;
		}
	}

	bool Gfx_Helpers::DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		// From glm::decompose in matrix_decompose.inl

		using namespace glm;
		using T = float;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3], Pdum3 = {};

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.
#if 0
		Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
		if (dot(Row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
				scale[i] *= static_cast<T>(-1);
				Row[i] *= static_cast<T>(-1);
			}
		}
#endif

		rotation.y = asin(-Row[0][2]);
		if (cos(rotation.y) != 0) {
			rotation.x = atan2(Row[1][2], Row[2][2]);
			rotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else {
			rotation.x = atan2(-Row[2][0], Row[1][1]);
			rotation.z = 0;
		}


		return true;
	}

	glm::mat4 Gfx_Helpers::ComposeTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
	{
		glm::mat4 rot = rotate(glm::mat4(1.0f), rotation.x, { 1, 0, 0 }) *
			rotate(glm::mat4(1.0f), rotation.y, { 0, 1, 0 }) *
			rotate(glm::mat4(1.0f), rotation.z, { 0, 0, 1 });

		return glm::translate(glm::mat4(1.0f), translation) *
			rot * glm::scale(glm::mat4(1.0f), { scale });
	}

	glm::mat3x4 Gfx_Helpers::ComposeTransform3x4(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
	{
		return glm::mat3x4(glm::transpose(ComposeTransform(translation, rotation, scale)));
	}

	glm::mat4 Gfx_Helpers::ComposeTransform2D(const glm::vec2& translation, const glm::vec2& rotation, const glm::vec2& scale)
	{
		glm::mat4 rot = rotate(glm::mat4(1.0f), rotation.x, { 0, 0, 1.0f });
		return  glm::translate(glm::mat4(1.0f), { translation, 0 }) *
			rot * glm::scale(glm::mat4(1.0f), { scale, 0 });
	}

	std::optional<std::string> Gfx_Helpers::OpenFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		Gfx_Window* win = Gfx_Context::GetSingleton()->GetWindow();
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)win->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::nullopt;
	}

	std::optional<std::string> Gfx_Helpers::SaveFile(const char* filter, const std::string& initialName)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		if (!initialName.empty())
		{
			for (uint32_t i = 0; i < static_cast<uint32_t>(initialName.size()); ++i)
			{
				szFile[i] = initialName[i];
			}
		}

		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		Gfx_Window* win = Gfx_Context::GetSingleton()->GetWindow();
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)win->GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = strchr(filter, '\0') + 1;

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::nullopt;
	}

	bool Gfx_Helpers::IsPathValid(const std::string& path)
	{
		return std::filesystem::exists(path);
	}

	glm::vec3 Gfx_Helpers::ScreenToWorld(const glm::vec2& mousePos, float width, float height, const glm::mat4& viewProj)
	{
		// these positions must be in range [-1, 1] (!!!), not [0, width] and [0, height]
		float mouseX = mousePos.x / (width * 0.5f) - 1.0f;
		float mouseY = mousePos.y / (height * 0.5f) - 1.0f;

		glm::mat4 invVP = glm::inverse(viewProj);
		glm::vec4 screenPos = glm::vec4(mouseX, -mouseY, 1.0f, 1.0f);
		glm::vec4 worldPos = invVP * screenPos;
		return glm::normalize(glm::vec3(worldPos));
	}

	glm::vec3 Gfx_Helpers::CastRay(const glm::vec3& startPos, const glm::vec2& mousePos, float width, float height, float distance, const glm::mat4& viewProj)
	{
		glm::vec3 rayDir = ScreenToWorld(mousePos, width, height, viewProj);
		return startPos + rayDir * distance;
	}

	glm::vec3 Gfx_Helpers::CastRay(const glm::vec3& startPos, float distance, const glm::mat4& viewProj)
	{
		auto data = Gfx_Context::GetSingleton()->GetWindow()->GetData();
		float w = static_cast<float>(data->myWidth);
		float h = static_cast<float>(data->myHeight);
		glm::vec2 mousePos = { Gfx_Input::GetMouseX(), Gfx_Input::GetMouseY() };

		return CastRay(startPos, mousePos, w, h, distance, viewProj);
	}

	std::string Gfx_Helpers::GetCachedPath(const std::string& filePath, CachedPathType type)
	{
		const std::string& fileName = std::filesystem::path(filePath).filename().string();
		const std::string& resourcesPath = std::filesystem::path(filePath).parent_path().string();

		switch (type)
		{
		case CachedPathType::Shader:

			return resourcesPath + "/spirv/" + fileName + ".spirv";

		case CachedPathType::Pipeline:

			return resourcesPath + "/pipeline/ " + fileName + ".pipeline_cache";
		}

		return "";
	}
}