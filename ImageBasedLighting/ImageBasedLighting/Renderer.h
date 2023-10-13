#pragma once
#include "../../cores/BasicGeometryGenerator.h"
#include "../../cores/Camera.h"
#include "../../cores/Framebuffer.h"
#include "../../cores/ImageBasedLight.h"
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
	Environment,
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
	void BuildShadowResources(); // textures and framebuffers
	void BuildMaterials();
	void BuildShaders();
	void BuildFramebuffers();

	void InitializeSceneConstant();

	void LinkPrograms(const std::string& shaderName, const std::vector<uint32_t>& shaderIDs);
	void UseProgram(uint32_t programID);

	void BuildRenderItems();
	void DrawRenderItems(RenderLayer renderLayer, uint32_t programID, bool isEnvironmentMap = false);
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

	std::unordered_map<std::string, Framebuffer> mFramebuffers;

	SceneConstant mSceneConstant{};

	Menu mMenu{};

	bool mShowImGuiWindow = true;

	std::vector<RenderItem> mOpaqueRenderItems;
	std::vector<RenderItem> mPBRRenderItems;
	std::vector<RenderItem> mInstancingRenderItems;
	std::vector<RenderItem> mEnvironmentRenderItems;
	
	std::unordered_map<RenderLayer, std::vector<RenderItem>> mAllRenderItems;

	std::string mShaderDirectoryName = "E:\\SeoulTech_CG_Lab_projects\\resources\\shaders\\";
	std::string mTextureDirectoryName = "E:\\SeoulTech_CG_Lab_projects\\resources\\textures\\";
	std::string mModelDirectoryName = "E:\\SeoulTech_CG_Lab_projects\\resources\\models\\";
	std::string mImageDirectoryName = "E:\\SeoulTech_CG_Lab_projects\\resources\\images\\";

	ImageBasedLight mImageBasedLight;

	// mouse variables
	float mLastMousePosX = 0.0f;
	float mLastMousePosY = 0.0f;
	float mXOffset = 0.0f;
	float mYOffset = 0.0f;
	bool mFirstMouse = true;

	float mDeltaTime = 0.0f;
	float mLastFrame = 0.0f;
};