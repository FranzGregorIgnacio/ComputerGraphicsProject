#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <string>

class ShaderManager;
class ShapeMeshes;

class SceneNode {
public:
    SceneNode();
    ~SceneNode();

    void SetTransform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale);
    void SetMaterial(const std::string& materialTag);
    void SetTexture(const std::string& textureTag, int slot);
    void SetMeshDrawFunction(void (*drawFunc)(ShapeMeshes*));

    void AddChild(SceneNode* child);

    void Render(ShaderManager* shaderManager, ShapeMeshes* meshes, const glm::mat4& parentTransform);

private:
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;

    std::string m_materialTag;
    std::string m_textureTag;
    int m_textureSlot;

    void (*m_drawFunction)(ShapeMeshes*);

    std::vector<SceneNode*> m_children;
};
