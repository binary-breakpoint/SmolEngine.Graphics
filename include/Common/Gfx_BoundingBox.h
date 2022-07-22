#pragma once
#include "Memory.h"

#include <glm/glm.hpp>
#include <array>

namespace Dia
{
    struct Gfx_BoundingBox
    {
        Gfx_BoundingBox();

        void Reset();
        void MinPoint(const glm::vec3& val);
        void MaxPoint(const glm::vec3& val);
        void Transform(const glm::mat4x4& model);

        const glm::vec3& MinPoint(bool transformed = false) const;
        const glm::vec3& MaxPoint(bool transformed = false) const;
        const glm::vec3& Center(bool transformed = false) const;
        const glm::vec3& Extent(bool transformed = false) const;

    private:
        Ref<Gfx_BoundingBox> m_Original;
        std::array<glm::vec3,2> m_Boundaries;
        glm::vec3 m_Center;
        glm::vec3 m_Extent;
	};
}