#include "SceneNode.h"
#include "Ray.h"
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
        shaderManager->setBoolValue("uHighlight", m_isHighlighted);
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

    if (m_isHighlighted && meshes) {
        glm::mat4 model = parentTransform;
        model = glm::translate(model, m_position);
        model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, m_scale);

        
    }

}

bool SceneNode::Intersects(const Ray& ray, const glm::mat4& parentTransform, float& outDistance) const {
    // Full transform
    glm::mat4 model = parentTransform;
    model = glm::translate(model, m_position);
    model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, m_scale);

    // Inverse transform ray into local space
    glm::mat4 invModel = glm::inverse(model);
    glm::vec3 localOrigin = glm::vec3(invModel * glm::vec4(ray.origin, 1.0f));
    glm::vec3 localDir = glm::normalize(glm::vec3(invModel * glm::vec4(ray.direction, 0.0f)));
    Ray localRay(localOrigin, localDir);

    // Intersect local AABB
    return localRay.intersectsAABB(m_localMin, m_localMax, outDistance);
}

// Checks the hit, try and fix this if able, too far from objects and the rays seemingly choose whatever direction at random 
void SceneNode::CheckRayHit(const Ray& ray, const glm::mat4& parentTransform,
    SceneNode*& closestNode, float& closestDistance) {
    glm::mat4 model = parentTransform;
    model = glm::translate(model, m_position);
    model = glm::rotate(model, glm::radians(m_rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(m_rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(m_rotation.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, m_scale);

    glm::mat4 invModel = glm::inverse(model);
    glm::vec3 localOrigin = glm::vec3(invModel * glm::vec4(ray.origin, 1.0f));
    glm::vec3 localDir = glm::normalize(glm::vec3(invModel * glm::vec4(ray.direction, 0.0f)));
    Ray localRay(localOrigin, localDir);

    float tHit;
    if (localRay.intersectsAABB(m_localMin, m_localMax, tHit)) {
        if (tHit > 0.0f && tHit < closestDistance) {
            closestDistance = tHit;
            closestNode = this;
        }
    }

    for (SceneNode* child : m_children) {
        child->CheckRayHit(ray, model, closestNode, closestDistance);
    }
}



