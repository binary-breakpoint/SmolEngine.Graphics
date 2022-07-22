#pragma once

#include <glm/glm.hpp>

#include <array>

namespace Dia
{
	class Gfx_Frustum
	{
	public:
		void SetRadius(float value);
		void Update(const glm::mat4& matrix);
		bool CheckSphere(const glm::vec3& pos) const;
		const std::array<glm::vec4, 6>& GetPlanes() const;

	private:
		enum side
		{
			LEFT = 0,
			RIGHT = 1,
			TOP = 2,
			BOTTOM = 3,
			BACK = 4,
			FRONT = 5
		};

		std::array<glm::vec4, 6> m_Planes;
		float m_Radius = 25.0f;
	};
}