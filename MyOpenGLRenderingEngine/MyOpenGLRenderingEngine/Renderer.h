#pragma once
#include "../../cores/BasicGeometryGenerator.h"
#include "../../cores/Camera.h"
#include "../../cores/Mesh.h"
#include "../../cores/Model.h"
#include "../../cores/Shader.h"
#include "../../cores/Stdafx.h"
#include "../../cores/Texture.h"
#include "../../cores/Utility.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

enum class RenderLayer : int
{
	Opaque = 0,
	LightCube,
	PBR,
	Instancing,
	Sky,
	Count
};

struct SceneConstant
{
	glm::mat4 view;
	glm::mat4 projection;

	glm::vec3 cameraPos;
	float pad0;

	glm::vec4 ambientLight;

	std::array<DirectionalLight, DirectionalLight::maxNumDirectionalLights> directionalLights;
	std::array<PointLight, PointLight::maxNumPointLights> pointLights;
	std::array<SpotLight, SpotLight::maxNumSpotLights> spotLights;
};

struct RenderItem
{
	glm::mat4 world = glm::mat4(1.0f);
	Mesh* mesh = nullptr;
	Material* material = nullptr;
	bool isTexture = false;
	std::vector<Texture*> albedoMaps;
	std::vector<Texture*> specularMaps;
	std::vector<Texture*> normalMaps;
	std::vector<Texture*> metallicMaps;
	std::vector<Texture*> roughnessMaps;
};

struct Menu
{
	bool isUsingTexture = false;
	bool isUsingNormalMap = false;
};


void _FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void _MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
void _KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

class Renderer
{
public:
	Renderer();
	~Renderer();
	Renderer(const Renderer& rhs) = delete;
	Renderer operator=(const Renderer& rhs) = delete;

	void Initialize();

	void RenderLoop();

	void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
	void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
	void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static Renderer* GetRendererPointer();

private:
	bool InitializeWindow();

	void ProcessKeyboardInput();
	void UpdateData();
	void DrawScene();

	void UpdateSceneConstants();

	void BuildTextures();
	void BuildMaterials();

	void InitializeSceneConstant();

	void LinkPrograms(const std::string& shaderName, const std::vector<uint32_t>& shaderIDs);
	void UseProgram(uint32_t programID);

	void BuildRenderItems();
	void DrawRenderItems(RenderLayer renderLayer, uint32_t programID);
private:
	// Window size variables.
	uint32_t mWindowWidth;
	uint32_t mWindowHeight;

	uint32_t mViewportWidth;
	uint32_t mViewportHeight;

	// GLFW window pointer
	GLFWwindow* mWindow = nullptr;

	static Renderer* renderer;

	std::unordered_map<std::string, uint32_t> mProgramIDs;

	Camera mCamera;

	Model mMiniModel;

	std::unordered_map<std::string, Mesh> mBasicMeshes;
	std::unordered_map<std::string, Texture> mBasicTextures;
	std::unordered_map<std::string, Material> mBasicMaterials;

	SceneConstant mSceneConstant{};

	Menu mMenu{};

	bool mShowImGuiWindow = true;

	std::vector<RenderItem> mOpaqueRenderItems;
	std::vector<RenderItem> mPBRRenderItems;
	std::vector<RenderItem> mInstancingRenderItems;
	std::vector<RenderItem> mSkyRenderItems;
	std::unordered_map<RenderLayer, std::vector<RenderItem>> mAllRenderItems;

	// mouse variables
	float mLastMousePosX = 0.0f;
	float mLastMousePosY = 0.0f;
	float mXOffset = 0.0f;
	float mYOffset = 0.0f;
	bool mFirstMouse = true;

	float mDeltaTime = 0.0f;
	float mLastFrame = 0.0f;
};