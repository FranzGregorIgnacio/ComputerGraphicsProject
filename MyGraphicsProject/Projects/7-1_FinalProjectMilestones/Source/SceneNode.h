#pragma once

#include "Ray.h"
#include <vector>
#include <glm/glm.hpp>
#include <string>

class SceneManager;
class ShaderManager;
class ShapeMeshes;


class SceneNode {
public:
    enum class MeshType {
        Box,
        Sphere,
        Cylinder,
        Plane,
        Pyramid,
        Custom
    };
    SceneNode();
    ~SceneNode();

    void SetTransform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
    void SetMaterial(const std::string& materialTag);
    void SetTexture(const std::string& textureTag, int slot);
    void SetMeshDrawFunction(void (*drawFunc)(ShapeMeshes*));
    void AddChild(SceneNode* child);
    void Render(SceneManager* sceneManager, ShaderManager* shaderManager, ShapeMeshes* meshes, const glm::mat4& parentTransform);
    bool Intersects(const Ray& ray, const glm::mat4& parentTransform, float& outDistance) const;
    void CheckRayHit(const Ray& ray, const glm::mat4& parentTransform, SceneNode*& closestNode, float& closestDistance);
    void SetHighlighted(bool value) { m_isHighlighted = value; }
    bool IsHighlighted() const { return m_isHighlighted; }
    void SetMeshType(MeshType type) { m_meshType = type; }
    MeshType GetMeshType() const { return m_meshType; }
    const std::vector<SceneNode*>& GetChildren() const { return m_children; }




private:
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;

    std::string m_materialTag;
    std::string m_textureTag;
    int m_textureSlot;

    void (*m_drawFunction)(ShapeMeshes*);

    std::vector<SceneNode*> m_children;

    glm::vec3 m_localMin = glm::vec3(-0.5f);
    glm::vec3 m_localMax = glm::vec3(0.5f);
    MeshType m_meshType = MeshType::Custom;
    bool m_isHighlighted = false;
};
