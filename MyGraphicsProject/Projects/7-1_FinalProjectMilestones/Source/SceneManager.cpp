///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"
#include "SceneNode.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif


#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager, Camera* pCamera)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
	m_pCamera = pCamera;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag, int object)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		//The following finds the unit and texture ID
		int textureID = -1;
		int textureSlot = -1;
		textureSlot = FindTextureSlot(textureTag);
		textureID = FindTextureID(textureTag);

		glActiveTexture(GL_TEXTURE0 + textureSlot);
		glBindTexture(GL_TEXTURE_2D, textureID);

		GLint textureUniformLoc = glGetUniformLocation(programID, textureTag.c_str());
		GLint objectIDUniformLoc = glGetUniformLocation(programID, "object");

		glUniform1i(objectIDUniformLoc, object);
		glUniform1i(textureUniformLoc, textureSlot);

		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

void SceneManager::DefineObjectMaterials() {
	//The following creates the materials for the textures
	OBJECT_MATERIAL water;
	water.ambientStrength = 0.3f;
	water.ambientColor = glm::vec3(0.0f, 0.2f, 0.5f);
	water.diffuseColor = glm::vec3(0.0f, 0.5f, 0.8f);
	water.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	water.shininess = 64.0f;
	water.tag = "floorTexture";

	OBJECT_MATERIAL stone;
	stone.ambientStrength = 0.2f;
	stone.ambientColor = glm::vec3(0.3f, 0.3f, 0.3f);
	stone.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	stone.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	stone.shininess = 8.0f;
	stone.tag = "stoneTexture";

	OBJECT_MATERIAL lanternSupport;
	lanternSupport.ambientStrength = 0.25f;
	lanternSupport.ambientColor = glm::vec3(0.5f, 0.5f, 0.5f);
	lanternSupport.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);
	lanternSupport.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	lanternSupport.shininess = 16.0f;
	lanternSupport.tag = "lanternSupportTexture";

	OBJECT_MATERIAL flame;
	flame.ambientStrength = 1.0f;
	flame.ambientColor = glm::vec3(1.0f, 0.5f, 0.0f);
	flame.diffuseColor = glm::vec3(1.0f, 0.6f, 0.1f);
	flame.specularColor = glm::vec3(1.0f, 0.5f, 0.0f);
	flame.shininess = 32.0f;
	flame.tag = "lampFlameTexture";

	OBJECT_MATERIAL lampBase;
	lampBase.ambientStrength = 0.2f;
	lampBase.ambientColor = glm::vec3(0.4f, 0.3f, 0.2f);
	lampBase.diffuseColor = glm::vec3(0.5f, 0.4f, 0.3f);
	lampBase.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	lampBase.shininess = 16.0f;
	lampBase.tag = "lampBaseTexture";

	OBJECT_MATERIAL lampTop;
	lampTop.ambientStrength = 0.3f;
	lampTop.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	lampTop.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);
	lampTop.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	lampTop.shininess = 8.0f;
	lampTop.tag = "lampTopTexture";

	OBJECT_MATERIAL toriiSupport;
	toriiSupport.ambientStrength = 0.25f;
	toriiSupport.ambientColor = glm::vec3(0.4f, 0.1f, 0.1f);
	toriiSupport.diffuseColor = glm::vec3(0.9f, 0.2f, 0.1f);
	toriiSupport.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
	toriiSupport.shininess = 32.0f;
	toriiSupport.tag = "toriiSupportTexture";

	OBJECT_MATERIAL toriiRoof;
	toriiRoof.ambientStrength = 0.25f;
	toriiRoof.ambientColor = glm::vec3(0.1f, 0.1f, 0.1f);
	toriiRoof.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	toriiRoof.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
	toriiRoof.shininess = 16.0f;
	toriiRoof.tag = "toriiRoofTexture";

	OBJECT_MATERIAL shrinewall;
	shrinewall.ambientStrength = 0.3f;
	shrinewall.ambientColor = glm::vec3(0.5f, 0.4f, 0.3f);
	shrinewall.diffuseColor = glm::vec3(0.6f, 0.5f, 0.4f);
	shrinewall.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
	shrinewall.shininess = 16.0f;
	shrinewall.tag = "shrineWallTexture";

	OBJECT_MATERIAL shrineroof;
	shrineroof.ambientStrength = 0.3f;
	shrineroof.ambientColor = glm::vec3(0.4f, 0.3f, 0.2f);
	shrineroof.diffuseColor = glm::vec3(0.5f, 0.4f, 0.3f);
	shrineroof.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	shrineroof.shininess = 16.0f;
	shrineroof.tag = "shrineRoofTexture";

	OBJECT_MATERIAL lantern;
	lantern.ambientStrength = 0.25f;
	lantern.ambientColor = glm::vec3(0.5f, 0.5f, 0.5f);
	lantern.diffuseColor = glm::vec3(0.6f, 0.6f, 0.6f);
	lantern.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	lantern.shininess = 16.0f;
	lantern.tag = "lanternTexture";

	//Store Material
	m_objectMaterials.push_back(water);
	m_objectMaterials.push_back(stone);
	m_objectMaterials.push_back(lanternSupport);
	m_objectMaterials.push_back(flame);
	m_objectMaterials.push_back(lampBase);
	m_objectMaterials.push_back(lampTop);
	m_objectMaterials.push_back(toriiSupport);
	m_objectMaterials.push_back(toriiRoof);
	m_objectMaterials.push_back(shrinewall);
	m_objectMaterials.push_back(shrineroof);
	m_objectMaterials.push_back(lantern);
}

/***********************************************************
  *  LoadSceneTextures()
  *
  *  This method is used for preparing the 3D scene by loading
  *  the shapes, textures in memory to support the 3D scene
  *  rendering
  ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	/*** STUDENTS - add the code BELOW for loading the textures that ***/
	/*** will be used for mapping to objects in the 3D scene. Up to  ***/
	/*** 16 textures can be loaded per scene. Refer to the code in   ***/
	/*** the OpenGL Sample for help.                                 ***/
	CreateGLTexture("watertexture.jpg", "floorTexture");
	CreateGLTexture("grasstexture.png", "grassTexture");
	CreateGLTexture("lanternflamebase.jpg", "lampBaseTexture");
	CreateGLTexture("lanternflame.png", "lampFlameTexture");
	CreateGLTexture("stonebase.jpg", "stoneTexture");
	CreateGLTexture("cracks.png", "crackTexture");
	CreateGLTexture("lanternstone.png", "lanternSupportTexture");
	CreateGLTexture("woodplanktexture.jpeg", "plankTexture");
	CreateGLTexture("docksupport.jpg", "supportTexture");
	CreateGLTexture("dockgroundsupport.jpeg", "groundSupportTexture");
	CreateGLTexture("dirtpath.jpg", "dirtTexture");
	CreateGLTexture("toriiwood.jpg", "toriiTexture");
	CreateGLTexture("toriiroof.jpg", "toriiRoofTexture");
	CreateGLTexture("shrinewall.jpg", "shrineWallTexture");
	CreateGLTexture("shrineroof.jpg", "shrineRoofTexture");
	CreateGLTexture("stonekanjitexture.jpg", "kanjiTexture");
	// after the texture image data is loaded into memory, the
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
}


SceneNode* SceneManager::CreateLantern(const glm::vec3& basePosition) {
	SceneNode* root = new SceneNode(); // Neutral root node
	root->SetTransform(basePosition, glm::vec3(0), glm::vec3(1.0f));

	// Box base
	SceneNode* base = new SceneNode();
	base->SetTransform(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0), glm::vec3(3.0f, 1.5f, 3.0f));
	base->SetMaterial("stoneTexture");
	base->SetTexture("stoneTexture", 1);
	base->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
	root->AddChild(base);

	// Pillar
	SceneNode* pillar = new SceneNode();
	pillar->SetTransform(glm::vec3(0.0f, 1.48f, 0.0f), glm::vec3(0), glm::vec3(1.0f, 4.0f, 1.0f));
	pillar->SetMaterial("stoneTexture");
	pillar->SetTexture("stoneTexture", 1);
	pillar->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawCylinderMesh(); });
	root->AddChild(pillar);

	// Cap base (inverted pyramid)
	SceneNode* capBase = new SceneNode();
	capBase->SetTransform(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 0.0f, 180.0f), glm::vec3(3.0f, 1.0f, 3.0f));
	capBase->SetMaterial("lanternSupportTexture");
	capBase->SetTexture("lanternSupportTexture", 2);
	capBase->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawPyramid4Mesh(); });
	root->AddChild(capBase);

	// Cap top
	SceneNode* capTop = new SceneNode();
	capTop->SetTransform(glm::vec3(0.0f, 7.0f, 0.0f), glm::vec3(0), glm::vec3(3.0f, 1.0f, 3.0f));
	capTop->SetMaterial("lanternSupportTexture");
	capTop->SetTexture("lanternSupportTexture", 2);
	capTop->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawPyramid4Mesh(); });
	root->AddChild(capTop);

	// Top sphere
	SceneNode* sphere = new SceneNode();
	sphere->SetTransform(glm::vec3(0.0f, 7.25f, 0.0f), glm::vec3(0), glm::vec3(0.5f));
	sphere->SetMaterial("lampTopTexture");
	sphere->SetTexture("lanternSupportTexture", 2);
	sphere->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawSphereMesh(); });
	root->AddChild(sphere);

	// Vertical supports
	std::vector<glm::vec3> supportOffsets = {
		glm::vec3(-1.0f, 6.0f, 1.0f),
		glm::vec3(1.0f, 6.0f, 1.0f),
		glm::vec3(-1.0f, 6.0f, -1.0f),
		glm::vec3(1.0f, 6.0f, -1.0f)
	};

	for (const glm::vec3& offset : supportOffsets) {
		SceneNode* support = new SceneNode();
		support->SetTransform(offset, glm::vec3(0), glm::vec3(0.6f, 1.25f, 0.6f));
		support->SetMaterial("lanternSupportTexture");
		support->SetTexture("lanternSupportTexture", 2);
		support->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
		root->AddChild(support);
	}

	// Flame cylinder
	SceneNode* flame = new SceneNode();
	flame->SetTransform(glm::vec3(0.0f, 5.8f, 0.0f), glm::vec3(0), glm::vec3(0.5f, 1.0f, 0.5f));
	flame->SetMaterial("lampFlameTexture");
	flame->SetTexture("lampFlameTexture", 4);
	flame->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawCylinderMesh(); });
	root->AddChild(flame);

	// Flame base
	SceneNode* flameBase = new SceneNode();
	flameBase->SetTransform(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0), glm::vec3(0.55f, 0.8f, 0.55f));
	flameBase->SetMaterial("lampBaseTexture");
	flameBase->SetTexture("lampBaseTexture", 3);
	flameBase->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawCylinderMesh(); });
	root->AddChild(flameBase);

	return root;
}
SceneNode* SceneManager::CreateGround() {
	SceneNode* root = new SceneNode();
	root->SetTransform(glm::vec3(0), glm::vec3(0), glm::vec3(1.0f));

	// Water Plane
	SceneNode* water = new SceneNode();
	water->SetTransform(glm::vec3(-15.0f, 0.24f, -5.0f), glm::vec3(0), glm::vec3(50.0f, 1.0f, 50.0f));
	water->SetMaterial("floorTexture");
	water->SetTexture("waterTexture", 0);
	water->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawPlaneMesh(); });
	root->AddChild(water);

	// Grass Patch
	SceneNode* grass = new SceneNode();
	grass->SetTransform(glm::vec3(15.0f, 0.25f, 20.0f), glm::vec3(0.0f, 90.0f, 0.0f), glm::vec3(25.0f, 1.0f, 20.0f));
	grass->SetMaterial("floorTexture");
	grass->SetTexture("grassTexture", 5);
	grass->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawPlaneMesh(); });
	root->AddChild(grass);

	return root;
}
SceneNode* SceneManager::CreateShrine() {
	SceneNode* root = new SceneNode();
	root->SetTransform(glm::vec3(0), glm::vec3(0), glm::vec3(1.0f));

	// ===== Path to Shrine =====
	auto path1 = new SceneNode();
	path1->SetTransform(glm::vec3(10.0f, 0.26f, 20.0f), glm::vec3(0), glm::vec3(2.5f, 1.0f, 25.0f));
	path1->SetTexture("dirtTexture", 9);
	path1->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawPlaneMesh(); });
	root->AddChild(path1);

	auto path2 = new SceneNode();
	path2->SetTransform(glm::vec3(22.5f, 0.26f, 18.0f), glm::vec3(0), glm::vec3(10.0f, 1.0f, 8.0f));
	path2->SetTexture("dirtTexture", 9);
	path2->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawPlaneMesh(); });
	root->AddChild(path2);

	// ===== Stone Base for Shrine =====
	auto base1 = new SceneNode();
	base1->SetTransform(glm::vec3(23.0f, 0.50f, 18.0f), glm::vec3(0), glm::vec3(12.0f, 0.5f, 12.0f));
	base1->SetMaterial("stoneTexture");
	base1->SetTexture("stoneTexture", 1);
	base1->SetTexture("crackTexture", 1);
	base1->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
	root->AddChild(base1);

	auto base2 = new SceneNode();
	base2->SetTransform(glm::vec3(23.0f, 0.75f, 18.0f), glm::vec3(0), glm::vec3(10.0f, 1.0f, 10.0f));
	base2->SetMaterial("stoneTexture");
	base2->SetTexture("stoneTexture", 1);
	base2->SetTexture("crackTexture", 1);
	base2->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
	root->AddChild(base2);

	// ===== Torii Gate (Left + Right Columns & Beams) =====
	std::vector<glm::vec3> toriiColumns = {
		{14.5f, 0.27f, 12.0f}, {14.5f, 0.27f, 24.0f}
	};
	for (const auto& pos : toriiColumns) {
		auto column = new SceneNode();
		column->SetTransform(pos, glm::vec3(0), glm::vec3(1.0f, 10.0f, 1.0f));
		column->SetMaterial("toriiSupport");
		column->SetTexture("toriiTexture", 10);
		column->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawCylinderMesh(); });
		root->AddChild(column);
	}

	// Horizontal Beam
	auto beam1 = new SceneNode();
	beam1->SetTransform(glm::vec3(14.5f, 8.75f, 12.0f), glm::vec3(0, 90.0f, 90.0f), glm::vec3(0.5f, 4.0f, 1.0f));
	beam1->SetMaterial("toriiSupport");
	beam1->SetTexture("toriiTexture", 10);
	beam1->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
	root->AddChild(beam1);

	auto beam2 = new SceneNode();
	beam2->SetTransform(glm::vec3(14.5f, 8.75f, 24.0f), glm::vec3(0, 90.0f, 90.0f), glm::vec3(0.5f, 4.0f, 1.0f));
	beam2->SetMaterial("toriiSupport");
	beam2->SetTexture("toriiTexture", 10);
	beam2->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
	root->AddChild(beam2);

	// Top Pyramids for Torii
	// Top Pyramids for Torii (Position, Y-axis Rotation)
	std::vector<std::pair<glm::vec3, float>> toriiPyramids = {
		{glm::vec3(14.5f, 9.0f, 11.5f), 90.0f},
		{glm::vec3(14.5f, 9.0f, 12.5f), 270.0f},
		{glm::vec3(14.5f, 9.0f, 23.5f), 90.0f},
		{glm::vec3(14.5f, 9.0f, 24.5f), 270.0f}
	};

	for (const std::pair<glm::vec3, float>& entry : toriiPyramids) {
		const glm::vec3& pos = entry.first;
		float yrot = entry.second;

		SceneNode* pyramid = new SceneNode();
		pyramid->SetTransform(pos, glm::vec3(0, yrot, 90.0f), glm::vec3(1.0f, 3.0f, 1.0f));
		pyramid->SetMaterial("toriiSupport");
		pyramid->SetTexture("toriiTexture", 10);
		pyramid->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawPyramid4Mesh(); });
		root->AddChild(pyramid);
	}

	// ===== Roof Beams =====
	auto roofBeam1 = new SceneNode();
	roofBeam1->SetTransform(glm::vec3(14.5f, 8.0f, 18.0f), glm::vec3(90.0f, 0, 0), glm::vec3(1.0f, 18.0f, 1.0f));
	roofBeam1->SetMaterial("toriiSupport");
	roofBeam1->SetTexture("toriiTexture", 10);
	roofBeam1->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
	root->AddChild(roofBeam1);

	auto roofBeam2 = new SceneNode();
	roofBeam2->SetTransform(glm::vec3(14.5f, 11.0f, 18.0f), glm::vec3(90.0f, 0, 0), glm::vec3(2.0f, 19.0f, 1.5f));
	roofBeam2->SetMaterial("toriiSupport");
	roofBeam2->SetTexture("toriiTexture", 10);
	roofBeam2->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
	root->AddChild(roofBeam2);

	auto roofBase = new SceneNode();
	roofBase->SetTransform(glm::vec3(14.5f, 9.0f, 18.0f), glm::vec3(0), glm::vec3(0.5f, 3.0f, 1.5f));
	roofBase->SetMaterial("toriiSupport");
	roofBase->SetTexture("toriiTexture", 10);
	roofBase->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
	root->AddChild(roofBase);

	auto roofTop = new SceneNode();
	roofTop->SetTransform(glm::vec3(14.5f, 11.5f, 18.0f), glm::vec3(90.0f, 0, 0), glm::vec3(2.5f, 19.5f, 1.0f));
	roofTop->SetMaterial("toriiRoof");
	roofTop->SetTexture("toriiRoofTexture", 11);
	roofTop->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
	root->AddChild(roofTop);

	// ===== Shrine Roof =====
	auto shrineRoof = new SceneNode();
	shrineRoof->SetTransform(glm::vec3(23.0f, 8.75f, 18.0f), glm::vec3(0), glm::vec3(11.0f, 5.0f, 11.5f));
	shrineRoof->SetMaterial("shrineRoofTexture");
	shrineRoof->SetTexture("shrineRoofTexture", 12);
	shrineRoof->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawPyramid4Mesh(); });
	root->AddChild(shrineRoof);

	// ===== Center Stone w/ Kanji =====
	auto kanjiStone = new SceneNode();
	kanjiStone->SetTransform(glm::vec3(23.0f, 3.75f, 18.0f), glm::vec3(0), glm::vec3(3.0f, 5.0f, 3.0f));
	kanjiStone->SetMaterial("stoneTexture");
	kanjiStone->SetTexture("kanjiTexture", 14);
	kanjiStone->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
	root->AddChild(kanjiStone);

	// === Shrine Walls ===
	std::vector<glm::vec3> supportPosts = {
		glm::vec3(27.25f, 3.755f, 22.5f),
		glm::vec3(18.75f, 3.755f, 22.5f),
		glm::vec3(27.25f, 3.755f, 13.5f),
		glm::vec3(18.75f, 3.755f, 13.5f)
	};

	for (const auto& pos : supportPosts) {
		SceneNode* post = new SceneNode();
		post->SetTransform(pos, glm::vec3(0), glm::vec3(1.0f, 5.0f, 1.0f));
		post->SetMaterial("shrineWallTexture");
		post->SetTexture("supportTexture", 7);
		post->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
		root->AddChild(post);
	}

	// Left and Right wall panels
	std::vector<glm::vec3> wallPanels = {
		glm::vec3(23.0f, 3.755f, 22.5f),
		glm::vec3(23.0f, 3.755f, 13.5f)
	};

	for (const auto& pos : wallPanels) {
		SceneNode* panel = new SceneNode();
		panel->SetTransform(pos, glm::vec3(0), glm::vec3(7.5f, 5.0f, 0.5f));
		panel->SetMaterial("shrineWallTexture");
		panel->SetTexture("shrineWallTexture", 13);
		panel->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
		root->AddChild(panel);
	}

	// Back wall
	SceneNode* backWall = new SceneNode();
	backWall->SetTransform(glm::vec3(27.25f, 3.755f, 18.0f), glm::vec3(0), glm::vec3(0.5f, 5.0f, 8.0f));
	backWall->SetMaterial("shrineWallTexture");
	backWall->SetTexture("shrineWallTexture", 13);
	backWall->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
	root->AddChild(backWall);

	// === Shrine Lantern Bases ===
	std::vector<glm::vec3> lanternBasePositions = {
		glm::vec3(18.75f, 5.755f, 15.5f),
		glm::vec3(18.75f, 5.755f, 20.5f)
	};

	for (const auto& pos : lanternBasePositions) {
		SceneNode* lanternBase = new SceneNode();
		lanternBase->SetTransform(pos, glm::vec3(0), glm::vec3(0.25f, 1.0f, 0.25f));
		lanternBase->SetMaterial("shrineWallTexture");
		lanternBase->SetTexture("supportTexture", 7); // Same as wall posts
		lanternBase->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
		root->AddChild(lanternBase);
	}

	// === Shrine Lantern Flame Cylinders (using wall textures instead of flame) ===
	std::vector<glm::vec3> flamePositions = {
		glm::vec3(18.75f, 4.5f, 20.5f),
		glm::vec3(18.75f, 4.5f, 15.5f)
	};

	for (const auto& pos : flamePositions) {
		SceneNode* flame = new SceneNode();
		flame->SetTransform(pos, glm::vec3(0), glm::vec3(0.5f, 1.0f, 0.5f));
		flame->SetMaterial("shrineWallTexture");
		flame->SetTexture("shrineWallTexture", 13);
		flame->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawCylinderMesh(); });
		root->AddChild(flame);
	}



	return root;
}
SceneNode* SceneManager::CreateDock(const glm::vec3& centerPosition) {
	SceneNode* root = new SceneNode();
	//Variables for step generation
	int numSteps = 6;
	float stepWidth = 4.0f;
	float stepHeight = -0.25f;
	float stepDepth = 0.5f;
	float stepSpacing = 0.35f;
	float supportRadius = 0.25f;
	float supportHeight = 2.0f;
	float startY = 0.0f;
	float startZ = 0.5f;

	root->SetTransform(centerPosition, glm::vec3(0), glm::vec3(1.0f));

	// Main Dock Planks
	float plankZ = 0.0f;
	for (int i = 0; i < 19; i++) {
		SceneNode* plank = new SceneNode();
		plank->SetTransform(glm::vec3(0.0f, 0.0f, plankZ), glm::vec3(0), glm::vec3(5.0f, 0.25f, 0.5f));
		plank->SetMaterial("shrineWallTexture");
		plank->SetTexture("plankTexture", 6);
		plank->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
		root->AddChild(plank);
		plankZ -= 0.5f;
	}

	// Dock Supports
	float supportZ = -0.5f;
	for (int i = 0; i < 5; i++) {
		for (float x : {-2.0f, 2.0f}) {
			SceneNode* support = new SceneNode();
			support->SetTransform(glm::vec3(x, -1.625f, supportZ), glm::vec3(0), glm::vec3(0.25f, 2.0f, 0.25f));
			support->SetMaterial("shrineWallTexture");
			support->SetTexture("supportTexture", 7);
			support->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawCylinderMesh(); });
			root->AddChild(support);
		}
		supportZ -= 2.0f;
	}

	// Stairs for Dock

	for (int i = 0; i < numSteps; ++i) {
		if (i % 2 == 0) {
			float y = startY + i * stepHeight;
			float z = startZ + i * stepSpacing;

			// Steps
			SceneNode* step = new SceneNode();
			step->SetTransform(glm::vec3(0.0f, y, z), glm::vec3(0), glm::vec3(stepWidth, 0.25f, stepDepth));
			step->SetMaterial("shrineWallTexture");
			step->SetTexture("plankTexture", 6);
			step->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawBoxMesh(); });
			root->AddChild(step);

			// Supports
			float offsetX = stepWidth / 2.0f - supportRadius;
			float supportY = y - supportHeight + 0.375;


			for (float x : {-offsetX, offsetX}) {
				SceneNode* support = new SceneNode();
				support->SetTransform(
					glm::vec3(x, supportY, z),
					glm::vec3(0),
					glm::vec3(supportRadius, supportHeight, supportRadius)
				);
				support->SetMaterial("shrineWallTexture");
				support->SetTexture("supportTexture", 7);
				support->SetMeshDrawFunction([](ShapeMeshes* mesh) { mesh->DrawCylinderMesh(); });
				root->AddChild(support);
			}

		}
	}

	return root;
}




/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	programID = m_pShaderManager->LoadShaders("vertex.glsl", "fragment.glsl");

	LoadSceneTextures();
	DefineObjectMaterials();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadTorusMesh();

	m_rootNode = new SceneNode();

	std::vector<glm::vec3> lanternPositions = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 12.0f),
		glm::vec3(0.0f, 0.0f, 24.0f),
		glm::vec3(0.0f, 0.0f, 36.0f),
		glm::vec3(18.0f, 0.0f, 0.0f),
		glm::vec3(18.0f, 0.0f, 36.0f)
	};

	for (const glm::vec3& pos : lanternPositions) {
		SceneNode* lantern = CreateLantern(pos);
		m_rootNode->AddChild(lantern); 
	}
	m_rootNode->AddChild(CreateDock(glm::vec3(10.0f, 1.875f, -4.75f)));
	m_rootNode->AddChild(CreateGround());
	m_rootNode->AddChild(CreateShrine());
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	//Due to previous uage of the boolValue in the former RenderFunctions, it has been added here so that textures do register.
	m_pShaderManager->setBoolValue("useTexture", true);
	glUseProgram(programID);

	m_pShaderManager->setVec3Value("viewPos", m_pCamera->Position);
	m_pShaderManager->setFloatValue("time", glfwGetTime());
	//Calls if rootNode exists to render based on new SceneNode implementation
	if (m_rootNode) {
		glm::mat4 identity = glm::mat4(1.0f);
		m_rootNode->Render(this, m_pShaderManager, m_basicMeshes, identity);
	}

	/****************************************************************/
}

