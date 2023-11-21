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
	PBR_Deferred,
	Shadow,
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
	std::array<Texture, DirectionalLight::maxNumDirectionalLights> shadowMaps;
	const uint32_t directionalLightCount = 1;

	std::array<PointLight, PointLight::maxNumPointLights> pointLights;
	std::array<Texture, PointLight::maxNumPointLights> pointShadowCubeMaps;
	const uint32_t pointLightCount = 4;
	float farPlaneForPointShadow;

	std::array<SpotLight, SpotLight::maxNumSpotLights> spotLights;

	const uint32_t spotLightCount = 0;
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
	void BuildDirectionalShadowResources(uint32_t lightIndex); // textures and framebuffers
	void BuildPointShadowResources(uint32_t lightIndex); // textures and framebuffers
	void BuildMaterials();
	void BuildShaders();
	void BuildFramebuffers();

	void BuildG_Buffers();

	void InitializeSceneConstant();

	void LinkPrograms(const std::string& shaderName, const std::vector<uint32_t>& shaderIDs);
	void UseProgram(uint32_t programID);

	void BuildRenderItems();
	void DrawRenderItems(RenderLayer renderLayer, uint32_t programID, bool isEnvironmentMap = false);
	void DrawShadowMap(RenderLayer renderLayer, uint32_t programID, uint32_t lightIndex);
	void DrawShadowCubeMap(RenderLayer renderLayer, uint32_t programID, uint32_t lightIndex);

	void DrawG_Buffers(RenderLayer renderLayer, uint32_t programID);
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
	bool isCameraMove = false;

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
	std::vector<RenderItem> mPBRDeferredRenderItems;
	std::vector<RenderItem> mInstancingRenderItems;
	std::vector<RenderItem> mEnvironmentRenderItems;
	
	std::unordered_map<RenderLayer, std::vector<RenderItem>> mAllRenderItems;

	std::string mShaderDirectoryName = "E:\\SeoulTech_CG_Lab_projects\\resources\\shaders\\";
	std::string mTextureDirectoryName = "E:\\SeoulTech_CG_Lab_projects\\resources\\textures\\";
	std::string mModelDirectoryName = "E:\\SeoulTech_CG_Lab_projects\\resources\\models\\";
	std::string mImageDirectoryName = "E:\\SeoulTech_CG_Lab_projects\\resources\\images\\";

	ImageBasedLight mImageBasedLight;
	
	// shadow resources
	Framebuffer mShadowMapFramebuffer;
	uint32_t mShadowMapWidth = 0;
	uint32_t mShadowMapHeight = 0;

	// deferred rendering framebuffer
	Framebuffer mDeferredFramebuffer;
	std::vector<Texture> mG_Buffer; // not position map for research, only albedo map, normal map, metallic map, roughness map, ao map

	// mouse variables
	float mLastMousePosX = 0.0f;
	float mLastMousePosY = 0.0f;
	float mXOffset = 0.0f;
	float mYOffset = 0.0f;
	bool mFirstMouse = true;

	float mDeltaTime = 0.0f;
	float mLastFrame = 0.0f;
};