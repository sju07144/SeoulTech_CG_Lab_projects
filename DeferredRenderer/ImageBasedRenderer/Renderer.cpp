#include "Renderer.h"

Renderer* Renderer::renderer = nullptr;
const float Renderer::mRadius = 2.6f;
const float Renderer::mDegreeDelta = 45.0f;

void _FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	return Renderer::GetRendererPointer()->FramebufferSizeCallback(window, width, height);
}
void _MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
	return Renderer::GetRendererPointer()->MouseCallback(window, xposIn, yposIn);
}
void _KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	return Renderer::GetRendererPointer()->KeyCallback(window, key, scancode, action, mods);
}

Renderer::Renderer()
	: mWindowWidth(800), mWindowHeight(600),
	mViewportWidth(800), mViewportHeight(600),
	mShadowMapWidth(512), mShadowMapHeight(512)
{
	renderer = this;

	float aspectRatio = static_cast<float>(mViewportWidth) / mViewportHeight;

	mCamera.LookAt(
		0.0f, 0.0f, 3.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);

	mCamera.SetLens(0.25f * glm::pi<float>(), aspectRatio, 0.1f, 100.0f);
	mCamera.SetOrthoLens(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);

	mMenu.enableImageBasedLighting = true;
}
Renderer::~Renderer()
{
	for (auto& imageBasedLight: mImageBasedLights)
		imageBasedLight.DeleteResources();

	for (auto& framebuffer : mFramebuffers)
		framebuffer.second.DeleteFramebuffer();

	 for (auto& mesh : mBasicMeshes)
	 	mesh.second.DeleteMesh();

	 for (auto& texture : mBasicTextures)
		 texture.second.DeleteTexture();

	renderer = nullptr;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void Renderer::Initialize()
{
	if (!InitializeWindow())
	{
		std::cerr << "Window creation failed!" << std::endl;
		return;
	}

	// Initialize imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	const char* glsl_version = "#version 430";
	ImGui_ImplGlfw_InitForOpenGL(mWindow, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImGui::StyleColorsDark();

	// Configure model directory name;
	std::filesystem::path datasetDirectory(mDatasetDirectoryName);
	for (const auto& entry : std::filesystem::directory_iterator(datasetDirectory))
	{
		if (std::filesystem::is_directory(entry))
		{
			const auto& filename = entry.path().filename().string();
			if (filename.ends_with(".glb")) // ends_with function is enabled only since C++20.
				mModelDirectoryNames.push_back(filename);
		}
	}

	// Create meshes
	BasicGeometryGenerator geoGenerator;
	auto quad = geoGenerator.CreateQuad(-1.0f, -1.0f, 2.0f, 2.0f, 0.0f);
	quad.ConfigureMesh();
	mBasicMeshes.insert({ "quad", std::move(quad) });

	// Initialize scene constants
	InitializeSceneConstant();

	// Load Textures
	BuildTextures();

	// Build materials
	BuildMaterials();

	// Create vertex and fragment shader
	BuildShaders();

	// Create Framebuffers.
	BuildFramebuffers();

	// Create G-Buffers.
	BuildG_Buffers();

	// for (uint32_t lightIndex = 0; lightIndex < mSceneConstant.directionalLightCount; lightIndex++)
	//	BuildDirectionalShadowResources(lightIndex);

	// for (uint32_t lightIndex = 0; lightIndex < mSceneConstant.pointLightCount; lightIndex++)
	// 	BuildPointShadowResources(lightIndex);

	BuildImageBasedLightsAndDraw();

	BuildRenderItems();
}

void Renderer::RenderLoop()
{
	uint32_t modelIndex = 0, imageBasedLightIndex = 0;
	auto& currentImageBasedLight = mImageBasedLights[0];
	std::string imageDirectoryName = mDatasetDirectoryName + "\\" + mModelDirectoryNames[modelIndex];
	std::string currentImageFileName = " ";

	auto& PBRDeferredRenderItems = mAllRenderItems[RenderLayer::PBR_Deferred];
	auto& environmentRenderItems = mAllRenderItems[RenderLayer::Environment];

	auto& quadRenderItem = PBRDeferredRenderItems[0];
	auto& environmentRenderItem = environmentRenderItems[0];

	while (!glfwWindowShouldClose(mWindow))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		mDeltaTime = currentFrame - mLastFrame;
		mLastFrame = currentFrame;

		// Load albedo, normal, metallic, roughness, ao maps.
		LoadG_Buffers(imageDirectoryName, mTheta, mPhi);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ProcessKeyboardInput();
		UpdateData();
		DrawScene();

		currentImageFileName = "HDR" + std::to_string(imageBasedLightIndex + 1) + "_IBL_IBR_"
			+ std::to_string(static_cast<uint32_t>(mTheta)) + "_" + std::to_string(static_cast<uint32_t>(mPhi)) + ".png";
		SaveScreenshotToPNG(imageDirectoryName + "\\" + currentImageFileName, mWindowWidth, mWindowHeight);

		std::cout << "Save Completed " << mModelDirectoryNames[modelIndex] << "\\" << currentImageFileName << std::endl;

		imageBasedLightIndex++;
		if (imageBasedLightIndex == mNumImageBasedLights)
		{
			imageBasedLightIndex = 0;
			currentImageBasedLight = mImageBasedLights[imageBasedLightIndex];
			quadRenderItem.irradianceMap = currentImageBasedLight.GetIrradianceMap();
			quadRenderItem.prefilterMap = currentImageBasedLight.GetPreFilteredEnvironmentMap();
			quadRenderItem.brdfLUT = currentImageBasedLight.GetBRDFLookUpTable();
			environmentRenderItem.environmentMap = currentImageBasedLight.GetCubeMap();

			mPhi += mDegreeDelta;
			if (mPhi == 360.0f)
			{
				mTheta += mDegreeDelta;
				mPhi = 0.0f;
				if (mTheta == 225.0f)
				{
					modelIndex++;
					if (modelIndex == static_cast<uint32_t>(mModelDirectoryNames.size()))
						break;

					imageDirectoryName = mDatasetDirectoryName + "\\" + mModelDirectoryNames[modelIndex];
					mTheta = 0.0f;
				}
			}
		}
		else
		{
			currentImageBasedLight = mImageBasedLights[imageBasedLightIndex];
			quadRenderItem.irradianceMap = currentImageBasedLight.GetIrradianceMap();
			quadRenderItem.prefilterMap = currentImageBasedLight.GetPreFilteredEnvironmentMap();
			quadRenderItem.brdfLUT = currentImageBasedLight.GetBRDFLookUpTable();
			environmentRenderItem.environmentMap = currentImageBasedLight.GetCubeMap();
		}

		glfwSwapBuffers(mWindow);
		glfwPollEvents();
	}
}

void Renderer::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
void Renderer::MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	if (mFirstMouse)
	{
		mLastMousePosX = xpos;
		mLastMousePosY = ypos;
		mFirstMouse = false;
	}

	mXOffset = xpos - mLastMousePosX;
	mYOffset = mLastMousePosY - ypos; // reversed since y-coordinates go from bottom to top

	// camera can move if mouse doesn't locate in dear imgui window and left mouse button is clicking.
	ImGuiIO& io = ImGui::GetIO();
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS && !io.WantCaptureMouse && isCameraMove)
	{
		float dx = glm::radians(0.25f * mXOffset);
		float dy = glm::radians(0.25f * mYOffset);

		mCamera.Pitch(dy);
		mCamera.RotateY(dx);
	}

	mLastMousePosX = xpos;
	mLastMousePosY = ypos;
}
void Renderer::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		mShowImGuiWindow = !mShowImGuiWindow;
}

Renderer* Renderer::GetRendererPointer()
{
	return renderer;
}

bool Renderer::InitializeWindow()
{
	// glfw: initialize and configure.
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_SAMPLES, 4); 
	glfwWindowHint(GLFW_SAMPLES, 9); // -> for antialiasing
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _APPLE_
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, "ImageBasedLighting", nullptr, nullptr);
	if (mWindow == nullptr)
	{
		std::cout << "Failed to create window!!" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(mWindow);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	glfwSetFramebufferSizeCallback(mWindow, _FramebufferSizeCallback);
	glfwSetCursorPosCallback(mWindow, _MouseCallback);
	glfwSetKeyCallback(mWindow, _KeyCallback);

	return true;
}

void Renderer::ProcessKeyboardInput()
{
	if (isCameraMove)
	{
		if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(mWindow, true);

		const float dt = mDeltaTime;

		if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
			mCamera.Walk(10.0f * dt);

		if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
			mCamera.Walk(-10.0f * dt);

		if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
			mCamera.Strafe(-10.0f * dt);

		if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
			mCamera.Strafe(10.0f * dt);

		if (glfwGetKey(mWindow, GLFW_KEY_I) == GLFW_PRESS)
			SaveScreenshotToPNG(mImageDirectoryName + "image.png", mWindowWidth, mWindowHeight);
	}	
}
void Renderer::UpdateData()
{
	UpdateSceneConstants();
}
void Renderer::DrawScene()
{
	// for (uint32_t lightIndex = 0; lightIndex < mSceneConstant.directionalLightCount; lightIndex++)
	//	DrawShadowMap(RenderLayer::Shadow, mProgramIDs["shadow"], lightIndex);

	// for (uint32_t lightIndex = 0; lightIndex < mSceneConstant.pointLightCount; lightIndex++)
	// 	DrawShadowCubeMap(RenderLayer::Shadow, mProgramIDs["pointShadow"], lightIndex);

	int windowWidth, windowHeight;
	glfwGetFramebufferSize(mWindow, &windowWidth, &windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);

	DrawRenderItems(RenderLayer::PBR_Deferred, mProgramIDs["pbr_deferred"]);
	// DrawRenderItems(RenderLayer::Environment, mProgramIDs["cubeMapHDR"], mMenu.enableEnvironment);

	if (mShowImGuiWindow)
	{
		ImGui::Begin("Setting");   

		ImGui::Checkbox("IsCameraMove", &isCameraMove);

		ImGui::Checkbox("IsUsingTexture", &mMenu.isUsingTexture); 
		ImGui::Checkbox("IsUsingNormalMap", &mMenu.isUsingNormalMap);
		ImGui::Checkbox("EnableEnvironment", &mMenu.enableEnvironment);
		ImGui::Checkbox("EnableImageBasedLighting", &mMenu.enableImageBasedLighting);
		ImGui::Checkbox("EnableShadow", &mMenu.enableShadow);
     
		ImGui::SliderFloat4("ambient", &mSceneConstant.ambientLight.x, 0.0f, 1.0f);

		auto& pbrSphere = mBasicMaterials["pbrSphere"];
		ImGui::ColorEdit3("albedo", &pbrSphere.kd.x);
		ImGui::SliderFloat("metallic", &pbrSphere.metallic, 0.0f, 1.0f);
		ImGui::SliderFloat("roughness", &pbrSphere.roughness, 0.0f, 1.0f);
		ImGui::SliderFloat("ao", &pbrSphere.ao, 0.0f, 1.0f);

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::UpdateSceneConstants()
{
	// mSceneConstant.view = mCamera.GetView();
	// mSceneConstant.projection = mCamera.GetProjection();

	mCurrentCameraPosition = glm::vec3(
		mRadius * cos(glm::radians(mTheta - 90.0f)) * cos(glm::radians(mPhi)),
		mRadius * sin(glm::radians(mTheta - 90.0f)),
		mRadius * cos(glm::radians(mTheta - 90.0f)) * sin(glm::radians(mPhi))
	);

	mCurrentCameraFront = glm::vec3(0.0f, 0.0f, 0.0f) - mCurrentCameraPosition;

	mCurrentViewMatrix = glm::lookAt(mCurrentCameraPosition,
		mCurrentCameraPosition + mCurrentCameraFront,
		mCameraUp);

	// mSceneConstant.cameraPos = mCamera.GetPosition();
	mSceneConstant.cameraPos = mCurrentCameraPosition;
	mSceneConstant.cameraFront = mCurrentCameraFront;

	mSceneConstant.view = mCurrentViewMatrix;
	mSceneConstant.projection = mProjectionMatrix;
}

void Renderer::BuildTextures()
{
	
}
void Renderer::BuildDirectionalShadowResources(uint32_t lightIndex)
{
	TextureInfo shadowMapInfo;
	shadowMapInfo.wrapSType = GL_CLAMP_TO_EDGE;
	shadowMapInfo.wrapTType = GL_CLAMP_TO_EDGE;
	shadowMapInfo.minFilterType = GL_NEAREST;
	shadowMapInfo.magFilterType = GL_NEAREST;
	shadowMapInfo.isMipmap = false;
	shadowMapInfo.nullData = true;
	shadowMapInfo.width = mShadowMapWidth;
	shadowMapInfo.height = mShadowMapHeight;
	shadowMapInfo.textureInternalFormat = GL_DEPTH_COMPONENT;
	shadowMapInfo.textureFormat = GL_DEPTH_COMPONENT;
	shadowMapInfo.isBorderColor = true;
	shadowMapInfo.borderColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	mSceneConstant.shadowMaps[lightIndex].CreateHDRTexture2D(shadowMapInfo);

	glBindFramebuffer(GL_FRAMEBUFFER, mShadowMapFramebuffer.GetFramebuffer());
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mSceneConstant.shadowMaps[lightIndex].GetTexture(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Renderer::BuildPointShadowResources(uint32_t lightIndex)
{
	TextureInfo shadowMapInfo;
	shadowMapInfo.wrapSType = GL_CLAMP_TO_EDGE;
	shadowMapInfo.wrapTType = GL_CLAMP_TO_EDGE;
	shadowMapInfo.wrapRType = GL_CLAMP_TO_EDGE;
	shadowMapInfo.minFilterType = GL_NEAREST;
	shadowMapInfo.magFilterType = GL_NEAREST;
	shadowMapInfo.isMipmap = false;
	shadowMapInfo.nullData = true;
	shadowMapInfo.width = mShadowMapWidth;
	shadowMapInfo.height = mShadowMapHeight;
	shadowMapInfo.textureInternalFormat = GL_DEPTH_COMPONENT;
	shadowMapInfo.textureFormat = GL_DEPTH_COMPONENT;
	mSceneConstant.pointShadowCubeMaps[lightIndex].CreateHDRTextureCube(shadowMapInfo);

	glBindFramebuffer(GL_FRAMEBUFFER, mShadowMapFramebuffer.GetFramebuffer());
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mSceneConstant.pointShadowCubeMaps[lightIndex].GetTexture(), 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Renderer::BuildMaterials()
{
	Material pbrSphere;
	std::string materialName = "pbrSphere";
	pbrSphere.ka = glm::vec3(0.0f, 0.0f, 0.0f);
	pbrSphere.kd = glm::vec3(1.0f, 0.0f, 0.0f);
	pbrSphere.ks = glm::vec3(0.0f, 0.0f, 0.0f);
	pbrSphere.metallic = 0.3f;
	pbrSphere.roughness = 0.5f;
	pbrSphere.ao = 0.1f;
	mBasicMaterials.insert({ materialName, std::move(pbrSphere) });
}
void Renderer::BuildShaders()
{
	std::vector<uint32_t> shaderIDs;

	// image-based renderer shader
	Shader pbrDeferredVertexShader;
	Shader pbrDeferredFragmentShader;
	pbrDeferredVertexShader.CompileShader(mShaderDirectoryName + "pbr_deferred.vert", GL_VERTEX_SHADER);
	pbrDeferredFragmentShader.CompileShader(mShaderDirectoryName + "pbr_deferred.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(pbrDeferredVertexShader.GetShaderID());
	shaderIDs.push_back(pbrDeferredFragmentShader.GetShaderID());
	LinkPrograms("pbr_deferred", shaderIDs);
	shaderIDs.clear();

	// environment shader
	Shader cubeMapVertexShader;
	Shader cubeMapFragmentShader;
	cubeMapVertexShader.CompileShader(mShaderDirectoryName + "cubemapHDR.vert", GL_VERTEX_SHADER);
	cubeMapFragmentShader.CompileShader(mShaderDirectoryName + "cubemapHDR.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(cubeMapVertexShader.GetShaderID());
	shaderIDs.push_back(cubeMapFragmentShader.GetShaderID());
	LinkPrograms("cubeMapHDR", shaderIDs);
	shaderIDs.clear();

	//image-based light shader (from equiToCube to brdf)
	Shader equirectangularToCubeVertexShader;
	Shader equirectangularToCubeFragmentShader;
	equirectangularToCubeVertexShader.CompileShader(mShaderDirectoryName + "equirectangularToCube.vert", GL_VERTEX_SHADER);
	equirectangularToCubeFragmentShader.CompileShader(mShaderDirectoryName + "equirectangularToCube.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(equirectangularToCubeVertexShader.GetShaderID());
	shaderIDs.push_back(equirectangularToCubeFragmentShader.GetShaderID());
 	LinkPrograms("equirectangularToCube", shaderIDs);
	shaderIDs.clear();

	Shader irradianceMapFragmentShader;
	irradianceMapFragmentShader.CompileShader(mShaderDirectoryName + "irradianceMap.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(equirectangularToCubeVertexShader.GetShaderID());
	shaderIDs.push_back(irradianceMapFragmentShader.GetShaderID());
	LinkPrograms("irradianceMap", shaderIDs);
	shaderIDs.clear();

	Shader prefilterMapFragmentShader;
	prefilterMapFragmentShader.CompileShader(mShaderDirectoryName + "prefilterMap.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(equirectangularToCubeVertexShader.GetShaderID());
	shaderIDs.push_back(prefilterMapFragmentShader.GetShaderID());
	LinkPrograms("prefilterMap", shaderIDs);
	shaderIDs.clear();

	Shader brdfVertexShader;
	Shader brdfFragmentShader;
	brdfVertexShader.CompileShader(mShaderDirectoryName + "brdf.vert", GL_VERTEX_SHADER);
	brdfFragmentShader.CompileShader(mShaderDirectoryName + "brdf.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(brdfVertexShader.GetShaderID());
	shaderIDs.push_back(brdfFragmentShader.GetShaderID());
	LinkPrograms("brdf", shaderIDs);
	shaderIDs.clear();

	// shadow shader
	// Shader shadowVertexShader;
	// Shader shadowFragmentShader;
	// shadowVertexShader.CompileShader(mShaderDirectoryName + "shadow.vert", GL_VERTEX_SHADER);
	// shadowFragmentShader.CompileShader(mShaderDirectoryName + "shadow.frag", GL_FRAGMENT_SHADER);
	// shaderIDs.push_back(shadowVertexShader.GetShaderID());
	// shaderIDs.push_back(shadowFragmentShader.GetShaderID());
	// LinkPrograms("shadow", shaderIDs);
	// shaderIDs.clear();

	// Shader pointShadowVertexShader;
	// Shader pointShadowGeometryShader;
	// Shader pointShadowFragmentShader;
	// pointShadowVertexShader.CompileShader(mShaderDirectoryName + "pointShadow.vert", GL_VERTEX_SHADER);
	// pointShadowGeometryShader.CompileShader(mShaderDirectoryName + "pointShadow.geom", GL_GEOMETRY_SHADER);
	// pointShadowFragmentShader.CompileShader(mShaderDirectoryName + "pointShadow.frag", GL_FRAGMENT_SHADER);
	// shaderIDs.push_back(pointShadowVertexShader.GetShaderID());
	// shaderIDs.push_back(pointShadowGeometryShader.GetShaderID());
	// shaderIDs.push_back(pointShadowFragmentShader.GetShaderID());
	// LinkPrograms("pointShadow", shaderIDs);
	// shaderIDs.clear();
}

void Renderer::BuildFramebuffers()
{
	// mShadowMapFramebuffer.CreateFramebuffer(mShadowMapWidth, mShadowMapHeight, 0, false, true);
}

void Renderer::BuildG_Buffers()
{
	Texture albedoMap;
	albedoMap.CreateTexture2D(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		false, true, mWindowWidth, mWindowHeight, GL_RGB);
	mG_Buffer.insert({ "albedoMap", std::move(albedoMap) });

	Texture normalMap;
	normalMap.CreateTexture2D(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		false, true, mWindowWidth, mWindowHeight, GL_RGB);
	mG_Buffer.insert({ "normalMap", std::move(normalMap) });

	Texture metallicMap;
	metallicMap.CreateTexture2D(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		false, true, mWindowWidth, mWindowHeight, GL_RGB);
	mG_Buffer.insert({ "metallicMap", std::move(metallicMap) });

	Texture roughnessMap;
	roughnessMap.CreateTexture2D(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		false, true, mWindowWidth, mWindowHeight, GL_RGB);
	mG_Buffer.insert({ "roughnessMap", std::move(roughnessMap) });

	Texture metallicRoughnessMap;
	metallicRoughnessMap.CreateTexture2D(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		false, true, mWindowWidth, mWindowHeight, GL_RGB);
	mG_Buffer.insert({ "metallicRoughnessMap", std::move(metallicRoughnessMap) });

	Texture aoMap;
	aoMap.CreateTexture2D(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		false, true, mWindowWidth, mWindowHeight, GL_RGB);
	mG_Buffer.insert({ "aoMap", std::move(aoMap) });

	Texture maskMap;
	maskMap.CreateTexture2D(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST,
		false, true, mWindowWidth, mWindowHeight, GL_RGB);
	mG_Buffer.insert({ "maskMap", std::move(maskMap) });
}

void Renderer::LoadG_Buffers(const std::string& directoryName, float degree0, float degree1)
{
	LoadTexture("albedo", GL_UNSIGNED_BYTE, directoryName, degree0, degree1);
	LoadTexture("normal", GL_UNSIGNED_BYTE, directoryName, degree0, degree1);
	LoadTexture("metallic", GL_UNSIGNED_BYTE, directoryName, degree0, degree1);
	LoadTexture("roughness", GL_UNSIGNED_BYTE, directoryName, degree0, degree1);
	LoadTexture("metallicRoughness", GL_UNSIGNED_BYTE, directoryName, degree0, degree1);
	LoadTexture("ao", GL_UNSIGNED_BYTE, directoryName, degree0, degree1);
	LoadTexture("mask", GL_UNSIGNED_BYTE, directoryName, degree0, degree1);
}
void Renderer::LoadTexture(std::string textureName, GLenum textureType, const std::string& directoryName, float degree0, float degree1)
{
	// Load image, create texture and generate mipmaps.
	int _width, _height, _nrChannels;
	std::string textureFileName;
	stbi_set_flip_vertically_on_load(true);

	glBindTexture(GL_TEXTURE_2D, mG_Buffer[textureName + "Map"].GetTexture());

	if (textureName == "ao")
		textureName = "AO";
	else if (textureName == "metallicRoughness")
		textureName = "Metallic-Roughness";
	else
		textureName[0] = std::toupper(textureName[0]);

	textureFileName = directoryName + "\\" + textureName + "_" + std::to_string(static_cast<uint32_t>(degree0)) + "_" + std::to_string(static_cast<uint32_t>(degree1)) + ".png";

	if (textureType == GL_UNSIGNED_BYTE)
	{
		unsigned char* data = stbi_load(textureFileName.c_str(), &_width, &_height, &_nrChannels, 0);
		if (data)
		{
			GLenum format = GL_RGB;
			if (_nrChannels == 1)
				format = GL_RED;
			else if (_nrChannels == 3)
				format = GL_RGB;
			else if (_nrChannels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_2D, 0, format, _width, _height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load " << textureName << " texture" << std::endl;
			stbi_image_free(data);
		}
	}
	else if (textureType == GL_FLOAT)
	{
		float* data = stbi_loadf(textureFileName.c_str(), &_width, &_height, &_nrChannels, 0);
		if (data)
		{
			GLenum internalFormat, format;
			if (_nrChannels == 1)
			{
				internalFormat = GL_R16F;
				format = GL_RED;
			}
			else if (_nrChannels == 3)
			{
				internalFormat = GL_RGB16F;
				format = GL_RGB;
			}
			else if (_nrChannels == 4)
			{
				internalFormat = GL_RGBA16F;
				format = GL_RGBA;
			}

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, format, GL_FLOAT, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load " << textureName << " texture" << std::endl;
			stbi_image_free(data);
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::BuildImageBasedLightsAndDraw()
{
	uint32_t equirectangularToCubeShaders = mProgramIDs["equirectangularToCube"];
	uint32_t irradianceMapShaders = mProgramIDs["irradianceMap"]; // irradiacneMap -> irradianceMap
	uint32_t prefilterMapShaders = mProgramIDs["prefilterMap"];
	uint32_t brdfShaders = mProgramIDs["brdf"];

	std::string hdrDirectoryName = mDatasetDirectoryName + "\\hdr\\";
	std::array<std::string, 6> hdrFileNames = {
		"blue_photo_studio.hdr",
		"dancing_hall.hdr",
		"office.hdr",
		"pine_attic.hdr",
		"studio_small_03.hdr",
		"thatch_chapel.hdr"
	};
	for (int i = 0; i < 6; i++)
	{
		mImageBasedLights[i].SetDirectoryAndFileName(mShaderDirectoryName, hdrDirectoryName + hdrFileNames[i]);
		mImageBasedLights[i].BuildResources();
		mImageBasedLights[i].Draw(
			equirectangularToCubeShaders,
			irradianceMapShaders,
			prefilterMapShaders,
			brdfShaders
		);
	}
}

void Renderer::InitializeSceneConstant()
{
	// mSceneConstant.view = mCamera.GetView();
	// mSceneConstant.projection = mCamera.GetProjection();
	// mSceneConstant.projection = mCamera.GetOrthoProjection();

	mCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	mCurrentCameraPosition = glm::vec3(
		mRadius * cos(glm::radians(mTheta - 90.0f)) * cos(glm::radians(mPhi)),
		mRadius * sin(glm::radians(mTheta - 90.0f)),
		mRadius * cos(glm::radians(mTheta - 90.0f)) * sin(glm::radians(mPhi))
	);
	mCurrentCameraFront = glm::vec3(0.0f, 0.0f, 0.0f) - mCurrentCameraPosition;
	
	mCurrentViewMatrix = glm::lookAt(mCurrentCameraPosition,
		mCurrentCameraPosition + mCurrentCameraFront,
		mCameraUp);
	mProjectionMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(mWindowWidth) / static_cast<float>(mWindowHeight), 0.1f, 100.0f);

	mSceneConstant.screenSize = glm::vec2(mWindowWidth, mWindowHeight);
	// mSceneConstant.cameraPos = mCamera.GetPosition();
	mSceneConstant.cameraPos = mCurrentCameraPosition;
	mSceneConstant.cameraFront = mCurrentCameraFront;

	mSceneConstant.view = mCurrentViewMatrix;
	mSceneConstant.projection = mProjectionMatrix;
	
	/*
	mSceneConstant.ambientLight = glm::vec4{ 0.05f, 0.05f, 0.05f, 1.0f };
	mSceneConstant.directionalLights[0].direction = glm::vec3{ 0.0f, -1.0f, -1.0f };
	mSceneConstant.directionalLights[0].diffuse = glm::vec3{ 10.0f, 10.0f, 10.0f };
	mSceneConstant.directionalLights[0].specular = glm::vec3{ 0.5f, 0.5f, 0.5f };
	
	float nearPlane = 1.0f, farPlane = 30.5f;
	glm::vec3 tempLightPosition = glm::vec3(0.0f, 3.0f, 3.0f);
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
	glm::mat4 lightView = glm::lookAt(
		tempLightPosition,
		tempLightPosition + mSceneConstant.directionalLights[0].direction,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	mSceneConstant.directionalLights[0].lightSpaceMatrix = lightProjection * lightView;

	mSceneConstant.pointLights[0].position = glm::vec3(-10.0f, 10.0f, 10.0f);
	mSceneConstant.pointLights[1].position = glm::vec3(10.0f, 10.0f, 10.0f);
	mSceneConstant.pointLights[2].position = glm::vec3(-10.0f, -10.0f, 10.0f);
	mSceneConstant.pointLights[3].position = glm::vec3(10.0f, -10.0f, 10.0f);

	nearPlane = 1.0f;
	farPlane = 25.0f;
	mSceneConstant.farPlaneForPointShadow = farPlane;
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), static_cast<float>(mShadowMapWidth) / static_cast<float>(mShadowMapHeight), nearPlane, farPlane);

	for (int i = 0; i < 4; i++)
	{
		PointLight& pointLight = mSceneConstant.pointLights[i];
		glm::vec3 lightPos = pointLight.position;
		pointLight.lightSpaceMatrices[0] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		pointLight.lightSpaceMatrices[1] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		pointLight.lightSpaceMatrices[2] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		pointLight.lightSpaceMatrices[3] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		pointLight.lightSpaceMatrices[4] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		pointLight.lightSpaceMatrices[5] = shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		pointLight.diffuse = glm::vec3(300.0f, 300.0f, 300.0f);
	}
	*/
}

void Renderer::LinkPrograms(const std::string& shaderName, const std::vector<uint32_t>& shaderIDs)
{
	uint32_t programID = glCreateProgram();

	for (const auto& shaderID : shaderIDs)
		glAttachShader(programID, shaderID);
	glLinkProgram(programID);
	CheckCompileErrors(programID, "PROGRAM");

	for (const auto& shaderID : shaderIDs)
		glDetachShader(programID, shaderID);

	mProgramIDs.insert({ shaderName, programID });
}
void Renderer::UseProgram(uint32_t programID)
{
	glUseProgram(programID);
}

void Renderer::BuildRenderItems()
{
	RenderItem renderItem;
	glm::mat4 world(1.0f);

	renderItem.mesh = &mBasicMeshes["quad"];
	float scalingSize = static_cast<float>(1.0 / renderItem.mesh->GetDiagnalLength());
	glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scalingSize, scalingSize, scalingSize));
	glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), renderItem.mesh->GetBoundingBoxCenter());
	glm::mat4 normalizeMatrix = scalingMatrix * translateMatrix;
	renderItem.world = normalizeMatrix;
	renderItem.material = &mBasicMaterials["pbrSphere"];
	renderItem.albedoMaps.push_back(&mG_Buffer["albedoMap"]);
	renderItem.normalMaps.push_back(&mG_Buffer["normalMap"]);
	renderItem.metallicMaps.push_back(&mG_Buffer["metallicMap"]);
	renderItem.roughnessMaps.push_back(&mG_Buffer["roughnessMap"]);
	renderItem.metallicRoughnessMaps.push_back(&mG_Buffer["metallicRoughnessMap"]);
	renderItem.aoMaps.push_back(&mG_Buffer["aoMap"]);
	renderItem.maskMaps.push_back(&mG_Buffer["maskMap"]);
	renderItem.irradianceMap = mImageBasedLights[0].GetIrradianceMap();
	renderItem.prefilterMap = mImageBasedLights[0].GetPreFilteredEnvironmentMap();
	renderItem.brdfLUT = mImageBasedLights[0].GetBRDFLookUpTable();

	for (auto& shadowMap : mSceneConstant.shadowMaps)
		renderItem.shadowMaps.push_back(&shadowMap);

	for (auto& shadowCubeMap : mSceneConstant.pointShadowCubeMaps)
		renderItem.shadowCubeMaps.push_back(&shadowCubeMap);

	mPBRDeferredRenderItems.push_back(std::move(renderItem));

	mAllRenderItems.insert({ RenderLayer::PBR_Deferred, mPBRDeferredRenderItems });

	renderItem.mesh = &mBasicMeshes["box"];
	world = glm::mat4(1.0f);
	renderItem.world = world;
	renderItem.environmentMap = mImageBasedLights[0].GetPreFilteredEnvironmentMap();
	mEnvironmentRenderItems.push_back(std::move(renderItem));

	mAllRenderItems.insert({ RenderLayer::Environment, mEnvironmentRenderItems });
}
void Renderer::DrawRenderItems(RenderLayer renderLayer, uint32_t programID, bool isEnvironmentMap)
{
	const auto& renderItems = mAllRenderItems[renderLayer];

	if (isEnvironmentMap)
	{
		glDepthFunc(GL_LEQUAL);

		UseProgram(programID);

		glm::mat4 view = glm::mat4(glm::mat3(mSceneConstant.view));
		SetMat4(programID, "sceneConstant.view", view);
		SetMat4(programID, "sceneConstant.projection", mSceneConstant.projection);
		SetVec3(programID, "sceneConstant.cameraPos", mSceneConstant.cameraPos);

		for (const auto& renderItem : renderItems)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, renderItem.environmentMap->GetTexture());
			SetInt(programID, "environmentMap", 0);

			auto primitiveType = renderItem.mesh->GetPrimitiveType();
			auto indexCount = renderItem.mesh->GetIndexCount();
			auto indexFormat = renderItem.mesh->GetIndexFormat();
			auto vertexAttribArray = renderItem.mesh->GetVertexAttribArray();

			glBindVertexArray(vertexAttribArray);
			glDrawElements(primitiveType, indexCount, indexFormat, nullptr);
			glBindVertexArray(0);
		}

		glDepthFunc(GL_LESS);
		return;
	}

	UseProgram(programID);

	SetBool(programID, "enableImageBasedLighting", mMenu.enableImageBasedLighting);
	SetBool(programID, "enableShadow", mMenu.enableShadow);

	SetMat4(programID, "sceneConstant.view", mSceneConstant.view);
	SetMat4(programID, "sceneConstant.projection", mSceneConstant.projection);
	SetVec3(programID, "sceneConstant.cameraPos", mSceneConstant.cameraPos);
	SetVec3(programID, "sceneConstant.cameraFront", mSceneConstant.cameraFront);

	SetVec2(programID, "sceneConstant.screenSize", mSceneConstant.screenSize);
	
	SetVec4(programID, "sceneConstant.ambientLight", mSceneConstant.ambientLight);

	/*
	SetVec3(programID, "sceneConstant.directionalLights[0].direction", mSceneConstant.directionalLights[0].direction);
	SetVec3(programID, "sceneConstant.directionalLights[0].diffuse", mSceneConstant.directionalLights[0].diffuse);
	SetVec3(programID, "sceneConstant.directionalLights[0].specular", mSceneConstant.directionalLights[0].specular);

	SetMat4(programID, "sceneConstant.directionalLights[0].lightSpaceMatrix", mSceneConstant.directionalLights[0].lightSpaceMatrix);

	for (auto i = 0; i != mSceneConstant.pointLights.size(); i++)
	{
		SetVec3(programID, "sceneConstant.pointLights[" + std::to_string(i) + "].position", mSceneConstant.pointLights[i].position);
		SetVec3(programID, "sceneConstant.pointLights[" + std::to_string(i) + "].diffuse", mSceneConstant.pointLights[i].diffuse);
	}

	SetFloat(programID, "sceneConstant.farPlane", mSceneConstant.farPlaneForPointShadow);
	*/

	for (const auto& renderItem : renderItems)
	{
		SetMat4(programID, "world", renderItem.world);
		SetVec3(programID, "material.ka", renderItem.material->ka);
		SetVec3(programID, "material.kd", renderItem.material->kd);
		SetVec3(programID, "material.ks", renderItem.material->ks);
		SetFloat(programID, "material.metallic", renderItem.material->metallic);
		SetFloat(programID, "material.roughness", renderItem.material->roughness);
		SetFloat(programID, "material.ao", renderItem.material->ao);

		uint32_t i = 0;

		const auto& albedoMaps = renderItem.albedoMaps;
		uint32_t albedoMapCount = 0;
		for (const auto& albedoMap : albedoMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, albedoMap->GetTexture());
			SetInt(programID, "albedoMap" + std::to_string(albedoMapCount++), i);
			i++;
		}

		// const auto& specularMaps = renderItem.specularMaps;
		// uint32_t specularMapCount = 0;
		// for (const auto& specularMap : specularMaps)
		// {
		// 	glActiveTexture(GL_TEXTURE0 + i);
		// 	glBindTexture(GL_TEXTURE_2D, specularMap->GetTexture());
		// 	SetInt(programID, "specularMap" + std::to_string(specularMapCount++), i);
		// 	i++;
		// }

		const auto& normalMaps = renderItem.normalMaps;
		uint32_t normalMapCount = 0;
		for (const auto& normalMap : normalMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, normalMap->GetTexture());
			SetInt(programID, "normalMap" + std::to_string(normalMapCount++), i);
			i++;
		}

		const auto& metallicMaps = renderItem.metallicMaps;
		uint32_t metallicMapCount = 0;
		for (const auto& metallicMap : metallicMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, metallicMap->GetTexture());
			SetInt(programID, "metallicMap" + std::to_string(metallicMapCount++), i);
			i++;
		}

		const auto& roughnessMaps = renderItem.roughnessMaps;
		uint32_t roughnessMapCount = 0;
		for (const auto& roughnessMap : roughnessMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, roughnessMap->GetTexture());
			SetInt(programID, "roughnessMap" + std::to_string(roughnessMapCount++), i);
			i++;
		}

		const auto& metallicRoughnessMaps = renderItem.metallicRoughnessMaps;
		uint32_t metallicRoughnessMapCount = 0;
		for (const auto& metallicRoughnessMap : metallicRoughnessMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, metallicRoughnessMap->GetTexture());
			SetInt(programID, "metallicRoughnessMap" + std::to_string(metallicRoughnessMapCount++), i);
			i++;
		}

		const auto& aoMaps = renderItem.aoMaps;
		uint32_t aoMapCount = 0;
		for (const auto& aoMap : aoMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, aoMap->GetTexture());
			SetInt(programID, "aoMap" + std::to_string(aoMapCount++), i);
			i++;
		}

		const auto& maskMaps = renderItem.maskMaps;
		uint32_t maskMapCount = 0;
		for (const auto& maskMap : maskMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, maskMap->GetTexture());
			SetInt(programID, "maskMap" + std::to_string(maskMapCount++), i);
			i++;
		}

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, renderItem.irradianceMap->GetTexture());
		SetInt(programID, "irradianceMap", i);
		i++;

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, renderItem.prefilterMap->GetTexture());
		SetInt(programID, "prefilterMap", i);
		i++;

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, renderItem.brdfLUT->GetTexture());
		SetInt(programID, "brdfLUT", i);
		i++;

		const auto& shadowMaps = renderItem.shadowMaps;
		uint32_t shadowMapCount = 0;
		for (const auto& shadowMap : shadowMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, shadowMap->GetTexture());
			SetInt(programID, "shadowMaps[" + std::to_string(shadowMapCount++) + "]", i);
			i++;
		}

		const auto& shadowCubeMaps = renderItem.shadowCubeMaps;
		uint32_t shadowCubeMapCount = 0;
		for (const auto& shadowCubeMap : shadowCubeMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubeMap->GetTexture());
			SetInt(programID, "shadowCubeMaps[" + std::to_string(shadowCubeMapCount++) + "]", i);
			i++;
		}

		auto primitiveType = renderItem.mesh->GetPrimitiveType();
		auto indexCount = renderItem.mesh->GetIndexCount();
		auto indexFormat = renderItem.mesh->GetIndexFormat();
		auto vertexAttribArray = renderItem.mesh->GetVertexAttribArray();

		glBindVertexArray(vertexAttribArray);
		glDrawElements(primitiveType, indexCount, indexFormat, nullptr);
		glBindVertexArray(0);
	}
}

void Renderer::DrawShadowMap(RenderLayer renderLayer, uint32_t programID, uint32_t lightIndex)
{
	// enable cull face for peter-panning
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	const auto& renderItems = mAllRenderItems[renderLayer];

	UseProgram(programID);

	glViewport(0, 0, mShadowMapWidth, mShadowMapHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, mShadowMapFramebuffer.GetFramebuffer());
	glClear(GL_DEPTH_BUFFER_BIT);

	SetMat4(programID, "lightSpaceMatrix", mSceneConstant.directionalLights[lightIndex].lightSpaceMatrix);

	for (auto renderItem : renderItems)
	{
		SetMat4(programID, "world", renderItem.world);

		auto primitiveType = renderItem.mesh->GetPrimitiveType();
		auto indexCount = renderItem.mesh->GetIndexCount();
		auto indexFormat = renderItem.mesh->GetIndexFormat();
		auto vertexAttribArray = renderItem.mesh->GetVertexAttribArray();

		glBindVertexArray(vertexAttribArray);
		glDrawElements(primitiveType, indexCount, indexFormat, nullptr);
		glBindVertexArray(0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);
}

void Renderer::DrawShadowCubeMap(RenderLayer renderLayer, uint32_t programID, uint32_t lightIndex)
{
	// enable cull face for peter-panning
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	const auto& renderItems = mAllRenderItems[renderLayer];

	UseProgram(programID);

	glViewport(0, 0, mShadowMapWidth, mShadowMapHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, mShadowMapFramebuffer.GetFramebuffer());
	glClear(GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < 6; i++)
	{
		SetMat4(programID, "lightSpaceMatrices[" + std::to_string(i) + "]", mSceneConstant.pointLights[lightIndex].lightSpaceMatrices[i]);
		SetVec3(programID, "lightPos", mSceneConstant.pointLights[lightIndex].position);
	}
	SetFloat(programID, "farPlane", mSceneConstant.farPlaneForPointShadow);

	for (auto renderItem : renderItems)
	{
		SetMat4(programID, "world", renderItem.world);

		auto primitiveType = renderItem.mesh->GetPrimitiveType();
		auto indexCount = renderItem.mesh->GetIndexCount();
		auto indexFormat = renderItem.mesh->GetIndexFormat();
		auto vertexAttribArray = renderItem.mesh->GetVertexAttribArray();

		glBindVertexArray(vertexAttribArray);
		glDrawElements(primitiveType, indexCount, indexFormat, nullptr);
		glBindVertexArray(0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);
}