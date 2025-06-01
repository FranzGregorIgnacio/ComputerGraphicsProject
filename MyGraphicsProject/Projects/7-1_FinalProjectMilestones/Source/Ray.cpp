#include "Ray.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <algorithm>

Ray::Ray(const glm::vec3& origin, const glm::vec3& direction)
    : origin(origin), direction(glm::normalize(direction)) {}

Ray Ray::fromMouse(float mouseX, float mouseY, int screenWidth, int screenHeight,
    const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;
    glm::vec4 clipCoords = glm::vec4(x, y, -1.0f, 1.0f);

    glm::vec4 eyeCoords = glm::inverse(projectionMatrix) * clipCoords;
    eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);

    glm::vec3 worldDir = glm::normalize(glm::vec3(glm::inverse(viewMatrix) * eyeCoords));
    glm::vec3 camOrigin = glm::vec3(glm::inverse(viewMatrix)[3]);

    return Ray(camOrigin, worldDir);
}

bool Ray::intersectsAABB(const glm::vec3& min, const glm::vec3& max, float& tNear) const {
    float tmin = (min.x - origin.x) / direction.x;
    float tmax = (max.x - origin.x) / direction.x;
    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (min.y - origin.y) / direction.y;
    float tymax = (max.y - origin.y) / direction.y;
    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    float tzmin = (min.z - origin.z) / direction.z;
    float tzmax = (max.z - origin.z) / direction.z;
    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;

    tNear = tmin;

    // Optional: discard hits behind the ray
    return tmax >= 0.0f;
}

