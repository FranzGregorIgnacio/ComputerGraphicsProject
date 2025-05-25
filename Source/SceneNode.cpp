#include "SceneNode.h"
#include "SceneManager.h"
#include "ShaderManager.h"


#include <glm/gtc/matrix_transform.hpp>

SceneNode::SceneNode() :
    m_position(0.0f), m_rotation(0.0f), m_scale(1.0f),
    m_textureSlot(0), m_drawFunction(nullptr) {}

SceneNode::~SceneNode() {
    for (SceneNode* child : m_children) {
        delete child;
    }
    m_children.clear();
}

void SceneNode::SetTransform(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
    m_position = position;
    m_rotation = rotation;
    m_scale = scale;
}

void SceneNode::SetMaterial(const std::string& materialTag) {
    m_materialTag = materialTag;
}

void SceneNode::SetTexture(const std::string& textureTag, int slot) {
    m_textureTag = textureTag;
    m_textureSlot = slot;
}

void SceneNode::SetMeshDrawFunction(void (*drawFunc)(ShapeMeshes*)) {
    m_drawFunction = drawFunc;
}

void SceneNode::AddChild(SceneNode* child) {
    m_children.push_back(child);
}

void SceneNode::Render(SceneManager* sceneManager, ShaderManager* shaderManager, ShapeMeshes* meshes, const glm::mat4& parentTransform) {
    glm::mat4 transform = parentTransform;
    transform = glm::translate(transform, m_position);
    transform = glm::rotate(transform, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
    transform = glm::scale(transform, m_scale);

    if (shaderManager) {
        shaderManager->setMat4Value("model", transform);
        shaderManager->setIntValue("bUseTexture", true);
        shaderManager->setSampler2DValue(m_textureTag, m_textureSlot);
    }

    if (sceneManager) {
        sceneManager->SetShaderMaterial(m_materialTag);
        sceneManager->SetShaderTexture(m_textureTag, m_textureSlot);
    }


    if (m_drawFunction && meshes) {
        m_drawFunction(meshes);
    }

    for (SceneNode* child : m_children) {
        child->Render(sceneManager, shaderManager, meshes, transform);
    }
}