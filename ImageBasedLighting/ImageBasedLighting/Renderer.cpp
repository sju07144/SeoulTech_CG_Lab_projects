#include "Renderer.h"

Renderer* Renderer::renderer = nullptr;

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
	: mWindowWidth(1280), mWindowHeight(720),
	mViewportWidth(1280), mViewportHeight(720)
{
	renderer = this;

	float aspectRatio = static_cast<float>(mViewportWidth) / mViewportHeight;

	mCamera.LookAt(
		0.0f, 0.0f, 3.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);

	mCamera.SetLens(0.25f * glm::pi<float>(), aspectRatio, 0.1f, 100.0f);
}
Renderer::~Renderer()
{
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

	// Create meshes
	BasicGeometryGenerator geoGenerator;
	auto box = geoGenerator.CreateBox(2.0f, 2.0f, 2.0f);
	box.ConfigureMesh();
	mBasicMeshes.insert({ "box", std::move(box) });

	auto sphere = geoGenerator.CreateSphere(1.0f, 64, 64);
	sphere.ConfigureMesh();
	mBasicMeshes.insert({ "sphere", std::move(sphere) });

	auto quad = geoGenerator.CreateQuad(-1.0f, -1.0f, 2.0f, 2.0f, 0.0f);
	quad.ConfigureMesh();
	mBasicMeshes.insert({ "quad", std::move(quad) });

	auto grid = geoGenerator.CreateGrid(30.0f, 30.0f, 90, 90);
	grid.ConfigureMesh();
	mBasicMeshes.insert({ "grid", std::move(grid) });

	// Load models
	// model은 LoadModel 호출만으로 configuremesh, buildtexture 동시에 호출
	mMiniModel.LoadModel(mModelDirectoryName + "Cerberus_by_Andrew_Maximov\\Cerberus_LP.fbx");

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

	BuildShadowResources();

	mImageBasedLight.SetDirectoryAndFileName(mShaderDirectoryName, mTextureDirectoryName + +"wooden_lounge_4k.hdr");
	mImageBasedLight.BuildResources();

	BuildRenderItems();
}

void Renderer::RenderLoop()
{
	// DrawImageBasedLightRenderItems();
	mImageBasedLight.Draw(
		mProgramIDs["equirectangularToCube"],
		mProgramIDs["irradianceMap"],
		mProgramIDs["prefilterMap"],
		mProgramIDs["brdf"]
	);

	while (!glfwWindowShouldClose(mWindow))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		mDeltaTime = currentFrame - mLastFrame;
		mLastFrame = currentFrame;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ProcessKeyboardInput();
		UpdateData();
		DrawScene();

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
	if (state == GLFW_PRESS && !io.WantCaptureMouse)
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
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
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
	glfwWindowHint(GLFW_SAMPLES, 4);
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
void Renderer::UpdateData()
{
	UpdateSceneConstants();
}
void Renderer::DrawScene()
{
	int windowWidth, windowHeight;
	glfwGetFramebufferSize(mWindow, &windowWidth, &windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// glEnable(GL_CULL_FACE);
	// glCullFace(GL_BACK);

	DrawRenderItems(RenderLayer::PBR, mProgramIDs["pbr"]);
	DrawRenderItems(RenderLayer::Environment, mProgramIDs["cubeMapHDR"], mMenu.enableEnvironment);

	if (mShowImGuiWindow)
	{
		ImGui::Begin("Setting");                         

		ImGui::Checkbox("IsUsingTexture", &mMenu.isUsingTexture); 
		ImGui::Checkbox("IsUsingNormalMap", &mMenu.isUsingNormalMap);
		ImGui::Checkbox("EnableEnvironment", &mMenu.enableEnvironment);
		ImGui::Checkbox("EnableImageBasedLighting", &mMenu.enableImageBasedLighting);
     
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
	mSceneConstant.view = mCamera.GetView();
	mSceneConstant.projection = mCamera.GetProjection();

	mSceneConstant.cameraPos = mCamera.GetPosition();
}

void Renderer::BuildTextures()
{
	TextureInfo textureSetup;
	textureSetup.wrapSType = GL_REPEAT;
	textureSetup.wrapTType = GL_REPEAT;
	textureSetup.minFilterType = GL_LINEAR_MIPMAP_LINEAR;
	textureSetup.magFilterType = GL_LINEAR;
	textureSetup.isMipmap = true;

	Texture woodTexture(mTextureDirectoryName + "wood.jpg");
	std::string texName = "woodAlbedoMap";
	woodTexture.CreateTexture2D(GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
	mBasicTextures.insert({ texName, std::move(woodTexture) });

	std::string directoryName = mTextureDirectoryName + "rustediron1-alt2-bl\\";
	std::array<std::string, 4> textureNames = { "_basecolor", "_metallic", "_normal", "_roughness" };
	texName = "rustediron2";
	for (int i = 0; i < 4; i++)
	{
		std::string fileName(directoryName + texName + textureNames[i] + ".png");
		Texture rustediron2Texture(fileName);
		rustediron2Texture.CreateTexture2D(GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
		mBasicTextures.insert({ texName + textureNames[i], std::move(rustediron2Texture) });
	}

	directoryName = mTextureDirectoryName + "red-scifi-metal-bl\\";
	textureNames = { "_albedo", "_metallic", "_normal-ogl", "_roughness" };
	texName = "red-scifi-metal";
	for (int i = 0; i < 4; i++)
	{
		std::string fileName(directoryName + texName + textureNames[i] + ".png");
		Texture redScifiMetalTexture(fileName);
		redScifiMetalTexture.CreateTexture2D(GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, true);
		mBasicTextures.insert({ texName + textureNames[i], std::move(redScifiMetalTexture) });
	}

	
}
void Renderer::BuildShadowResources()
{
}
void Renderer::BuildMaterials()
{
	Material woodBox;
	std::string materialName = "woodBox";
	woodBox.ka = glm::vec3(0.1f, 0.1f, 0.1f);
	woodBox.kd = glm::vec3(1.0f, 1.0f, 1.0f);
	woodBox.ks = glm::vec3(0.3f, 0.3f, 0.3f);
	woodBox.metallic = 0.3f;
	woodBox.roughness = 0.5f;
	woodBox.ao = 0.1f;
	mBasicMaterials.insert({ materialName, std::move(woodBox) });

	Material pbrSphere;
	materialName = "pbrSphere";
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

	Shader opaqueVertexShader;
	Shader opaqueFragmentShader;
	opaqueVertexShader.CompileShader(mShaderDirectoryName + "opaque.vert", GL_VERTEX_SHADER);
	opaqueFragmentShader.CompileShader(mShaderDirectoryName + "opaque.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(opaqueVertexShader.GetShaderID());
	shaderIDs.push_back(opaqueFragmentShader.GetShaderID());
	LinkPrograms("opaque", shaderIDs);
	shaderIDs.clear();

	Shader pbrVertexShader;
	Shader pbrFragmentShader;
	pbrVertexShader.CompileShader(mShaderDirectoryName + "pbr.vert", GL_VERTEX_SHADER);
	pbrFragmentShader.CompileShader(mShaderDirectoryName + "pbr.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(pbrVertexShader.GetShaderID());
	shaderIDs.push_back(pbrFragmentShader.GetShaderID());
	LinkPrograms("pbr", shaderIDs);
	shaderIDs.clear();

	Shader cubeMapVertexShader;
	Shader cubeMapFragmentShader;
	cubeMapVertexShader.CompileShader(mShaderDirectoryName + "cubemapHDR.vert", GL_VERTEX_SHADER);
	cubeMapFragmentShader.CompileShader(mShaderDirectoryName + "cubemapHDR.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(cubeMapVertexShader.GetShaderID());
	shaderIDs.push_back(cubeMapFragmentShader.GetShaderID());
	LinkPrograms("cubeMapHDR", shaderIDs);
	shaderIDs.clear();

	Shader equirectangularToCubeFragmentShader;
	equirectangularToCubeFragmentShader.CompileShader(mShaderDirectoryName + "equirectangularToCube.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(cubeMapVertexShader.GetShaderID());
	shaderIDs.push_back(equirectangularToCubeFragmentShader.GetShaderID());
	LinkPrograms("equirectangularToCube", shaderIDs);
	shaderIDs.clear();

	Shader irradianceMapFragmentShader;
	std::vector<uint32_t> irradianceMapShaderIDs;
	irradianceMapFragmentShader.CompileShader(mShaderDirectoryName + "irradianceMap.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(cubeMapVertexShader.GetShaderID());
	shaderIDs.push_back(irradianceMapFragmentShader.GetShaderID());
	LinkPrograms("irradianceMap", shaderIDs);
	shaderIDs.clear();

	Shader prefilterMapFragmentShader;
	prefilterMapFragmentShader.CompileShader(mShaderDirectoryName + "prefilterMap.frag", GL_FRAGMENT_SHADER);
	shaderIDs.push_back(cubeMapVertexShader.GetShaderID());
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
}

void Renderer::BuildFramebuffers()
{
	
}

void Renderer::InitializeSceneConstant()
{
	mSceneConstant.view = mCamera.GetView();
	mSceneConstant.projection = mCamera.GetProjection();

	mSceneConstant.cameraPos = mCamera.GetPosition();

	mSceneConstant.ambientLight = glm::vec4{ 0.05f, 0.05f, 0.05f, 1.0f };
	mSceneConstant.directionalLights[0].direction = glm::vec3{ -0.2f, -1.0f, -0.3f };
	mSceneConstant.directionalLights[0].diffuse = glm::vec3{ 10.0f, 10.0f, 10.0f };
	mSceneConstant.directionalLights[0].specular = glm::vec3{ 0.5f, 0.5f, 0.5f };

	mSceneConstant.pointLights[0].position = glm::vec3(-10.0f, 10.0f, 10.0f);
	mSceneConstant.pointLights[1].position = glm::vec3(10.0f, 10.0f, 10.0f);
	mSceneConstant.pointLights[2].position = glm::vec3(-10.0f, -10.0f, 10.0f);
	mSceneConstant.pointLights[3].position = glm::vec3(10.0f, -10.0f, 10.0f);

	for (int i = 0; i < 4; i++)
		mSceneConstant.pointLights[i].diffuse = glm::vec3(300.0f, 300.0f, 300.0f);
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

	renderItem.mesh = &mBasicMeshes["box"];
	glm::mat4 world = glm::mat4(1.0f);
	world = glm::translate(world, glm::vec3(3.0f, 2.0f, 3.0f));
	renderItem.world = world;
	renderItem.material = &mBasicMaterials["woodBox"];
	renderItem.albedoMaps.push_back(&mBasicTextures["wood"]);
	mOpaqueRenderItems.push_back(std::move(renderItem));

	mAllRenderItems.insert({ RenderLayer::Opaque, mOpaqueRenderItems });

	auto& miniModelComponents = mMiniModel.GetModelComponentsByReference();
	for (auto& modelComponent : miniModelComponents)
	{
		renderItem.mesh = &modelComponent.mesh;
		world = glm::mat4(1.0f);
		world = glm::scale(world, glm::vec3(0.05f, 0.05f, 0.05f));
		world = glm::rotate(world, -glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		world = glm::rotate(world, -glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		world = glm::translate(world, glm::vec3(8.0f, 4.0f, 8.0f));
		renderItem.world = world;

		for (auto& albedoMap : modelComponent.albedoMaps)
			renderItem.albedoMaps.push_back(&albedoMap);
		for (auto& normalMap : modelComponent.normalMaps)
			renderItem.normalMaps.push_back(&normalMap);
		for (auto& metallicMap : modelComponent.metallicMaps)
			renderItem.metallicMaps.push_back(&metallicMap);
		for (auto& roughnessMap : modelComponent.roughnessMaps)
			renderItem.roughnessMaps.push_back(&roughnessMap);

		// renderItem.irradianceMap = &mBasicTextures["irradianceMap"];
		// renderItem.prefilterMap = &mBasicTextures["prefilterMap"];
		// renderItem.brdfLUT = &mBasicTextures["brdfLUT"];
		renderItem.irradianceMap = mImageBasedLight.GetIrradianceMap();
		renderItem.prefilterMap = mImageBasedLight.GetPreFilteredEnvironmentMap();
		renderItem.brdfLUT = mImageBasedLight.GetBRDFLookUpTable();
		mPBRRenderItems.push_back(renderItem);
	}

	int nrRows = 7;
	int nrColumns = 7;
	float spacing = 2.5f;

	for (int row = 0; row < nrRows; row++)
	{
		for (int col = 0; col < nrColumns; col++)
		{
			world = glm::mat4(1.0f);
			world = glm::translate(world, glm::vec3(
				static_cast<float>(col - (nrColumns / 2)) * spacing,
				static_cast<float>(row - (nrRows / 2)) * spacing,
				0.0f
			));
			renderItem.mesh = &mBasicMeshes["sphere"];
			renderItem.world = world;
			renderItem.material = &mBasicMaterials["pbrSphere"];
			renderItem.albedoMaps.push_back(&mBasicTextures["rustediron2_basecolor"]);
			renderItem.normalMaps.push_back(&mBasicTextures["rustediron2_normal"]);
			renderItem.metallicMaps.push_back(&mBasicTextures["rustediron2_metallic"]);
			renderItem.roughnessMaps.push_back(&mBasicTextures["rustediron2_roughness"]);
			// renderItem.irradianceMap = &mBasicTextures["irradianceMap"];
			// renderItem.prefilterMap = &mBasicTextures["prefilterMap"];
			// renderItem.brdfLUT = &mBasicTextures["brdfLUT"];
			renderItem.irradianceMap = mImageBasedLight.GetIrradianceMap();
			renderItem.prefilterMap = mImageBasedLight.GetPreFilteredEnvironmentMap();
			renderItem.brdfLUT = mImageBasedLight.GetBRDFLookUpTable();
			mPBRRenderItems.push_back(std::move(renderItem));
		}
	}

	world = glm::mat4(1.0f);
	world = glm::translate(world, glm::vec3(0.0f, 0.0f, -15.0f));
	renderItem.mesh = &mBasicMeshes["grid"];
	renderItem.world = world;
	renderItem.material = &mBasicMaterials["pbrSphere"];
	renderItem.albedoMaps.push_back(&mBasicTextures["red-scifi-metal_albedo"]);
	renderItem.normalMaps.push_back(&mBasicTextures["red-scifi-metal_normal-ogl"]);
	renderItem.metallicMaps.push_back(&mBasicTextures["red-scifi-metal_metallic"]);
	renderItem.roughnessMaps.push_back(&mBasicTextures["red-scifi-metal_roughness"]);
	// renderItem.irradianceMap = &mBasicTextures["irradianceMap"];
	// renderItem.prefilterMap = &mBasicTextures["prefilterMap"];
	// renderItem.brdfLUT = &mBasicTextures["brdfLUT"];
	renderItem.irradianceMap = mImageBasedLight.GetIrradianceMap();
	renderItem.prefilterMap = mImageBasedLight.GetPreFilteredEnvironmentMap();
	renderItem.brdfLUT = mImageBasedLight.GetBRDFLookUpTable();
	mPBRRenderItems.push_back(std::move(renderItem));

	mAllRenderItems.insert({ RenderLayer::PBR, mPBRRenderItems });

	renderItem.mesh = &mBasicMeshes["box"];
	world = glm::mat4(1.0f);
	renderItem.world = world;
	// renderItem.environmentMap = &mBasicTextures["cubeMap"];
	renderItem.environmentMap = mImageBasedLight.GetCubeMap();
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

		for (auto renderItem : renderItems)
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

	SetBool(programID, "isUsingTexture", mMenu.isUsingTexture);
	SetBool(programID, "isUsingNormalMap", mMenu.isUsingNormalMap);
	SetBool(programID, "enableImageBasedLighting", mMenu.enableImageBasedLighting);

	SetMat4(programID, "sceneConstant.view", mSceneConstant.view);
	SetMat4(programID, "sceneConstant.projection", mSceneConstant.projection);
	SetVec3(programID, "sceneConstant.cameraPos", mSceneConstant.cameraPos);
	
	SetVec4(programID, "sceneConstant.ambientLight", mSceneConstant.ambientLight);

	SetVec3(programID, "sceneConstant.directionalLights[0].direction", mSceneConstant.directionalLights[0].direction);
	SetVec3(programID, "sceneConstant.directionalLights[0].diffuse", mSceneConstant.directionalLights[0].diffuse);
	SetVec3(programID, "sceneConstant.directionalLights[0].specular", mSceneConstant.directionalLights[0].specular);

	for (auto i = 0; i != mSceneConstant.pointLights.size(); i++)
	{
		SetVec3(programID, "sceneConstant.pointLights[" + std::to_string(i) + "].position", mSceneConstant.pointLights[i].position);
		SetVec3(programID, "sceneConstant.pointLights[" + std::to_string(i) + "].diffuse", mSceneConstant.pointLights[i].diffuse);
	}

	for (auto renderItem : renderItems)
	{
		SetMat4(programID, "world", renderItem.world);
		SetVec3(programID, "material.ka", renderItem.material->ka);
		SetVec3(programID, "material.kd", renderItem.material->kd);
		SetVec3(programID, "material.ks", renderItem.material->ks);
		SetFloat(programID, "material.metallic", renderItem.material->metallic);
		SetFloat(programID, "material.roughness", renderItem.material->roughness);
		SetFloat(programID, "material.ao", renderItem.material->ao);

		const auto& albedoMaps = renderItem.albedoMaps;
		uint32_t i = 0;
		uint32_t albedoMapCount = 0;
		for (auto albedoMap : albedoMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, albedoMap->GetTexture());
			SetInt(programID, "albedoMap" + std::to_string(albedoMapCount++), i);
			i++;
		}

		const auto& specularMaps = renderItem.specularMaps;
		uint32_t specularMapCount = 0;
		for (auto specularMap : specularMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, specularMap->GetTexture());
			SetInt(programID, "specularMap" + std::to_string(specularMapCount++), i);
			i++;
		}

		const auto& normalMaps = renderItem.normalMaps;
		uint32_t normalMapCount = 0;
		for (auto normalMap : normalMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, normalMap->GetTexture());
			SetInt(programID, "normalMap" + std::to_string(normalMapCount++), i);
			i++;
		}

		const auto& metallicMaps = renderItem.metallicMaps;
		uint32_t metallicMapCount = 0;
		for (auto metallicMap : metallicMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, metallicMap->GetTexture());
			SetInt(programID, "metallicMap" + std::to_string(metallicMapCount++), i);
			i++;
		}

		const auto& roughnessMaps = renderItem.roughnessMaps;
		uint32_t roughnessMapCount = 0;
		for (auto roughnessMap : roughnessMaps)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, roughnessMap->GetTexture());
			SetInt(programID, "roughnessMap" + std::to_string(roughnessMapCount++), i);
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

		auto primitiveType = renderItem.mesh->GetPrimitiveType();
		auto indexCount = renderItem.mesh->GetIndexCount();
		auto indexFormat = renderItem.mesh->GetIndexFormat();
		auto vertexAttribArray = renderItem.mesh->GetVertexAttribArray();

		glBindVertexArray(vertexAttribArray);
		glDrawElements(primitiveType, indexCount, indexFormat, nullptr);
		glBindVertexArray(0);
	}
}