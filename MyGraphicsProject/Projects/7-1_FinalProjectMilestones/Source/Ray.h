#pragma once
#include <glm/glm.hpp>

class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray(const glm::vec3& origin, const glm::vec3& direction);

    // Generates a ray from screen coordinates
    static Ray fromMouse(float mouseX, float mouseY, int screenWidth, int screenHeight,
        const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

    // Tests if this ray intersects an AABB
    bool intersectsAABB(const glm::vec3& min, const glm::vec3& max, float& tNear) const;

};
