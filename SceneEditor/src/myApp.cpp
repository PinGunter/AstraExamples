#include <myApp.h>
#include <Utils.h>
#include <myScene.h>
#include <WireframePL.h>

void DefaultApp::init(const std::vector<Astra::Scene*>& scenes, Astra::Renderer* renderer, Astra::GuiController* gui)
{
	// init base app, copies scenes renderer and gui
	// also inits the scenes and sets callbacks
	Astra::App::init(scenes, renderer, gui);

	// renderer init
	auto size = AstraDevice.getWindowSize();
	_renderer->createSwapchain(AstraDevice.getSurface(), size[0], size[1]);
	_renderer->createDepthBuffer();
	_renderer->createRenderPass();
	_renderer->createFrameBuffers();
	_renderer->createOffscreenRender(_alloc);
	_renderer->createPostDescriptorSet();
	_renderer->createPostPipeline();
	_renderer->updatePostDescriptorSet();

	// gui init
	_gui = gui;
	_gui->init(AstraDevice.getWindow(), _renderer);

	// Scene -> GPU information || Uniforms
	// camera uniforms
	createUBO();

	// aceleration structures
	for (Astra::Scene* s : _scenes)
	{
		if (s->isRt())
		{
			((DefaultSceneRT*)s)->createBottomLevelAS();
			((DefaultSceneRT*)s)->createTopLevelAS();
		}
	}

	// descriptor sets
	createDescriptorSetLayout();
	updateDescriptorSet();
	createRtDescriptorSet();
	updateRtDescriptorSet();
	// pipelines
	createPipelines();
}

void DefaultApp::run()
{
	while (!glfwWindowShouldClose(_window))
	{
		App::run(); // update the scene

		glfwPollEvents();
		if (isMinimized())
		{
			continue;
		}

		if (_needsReset)
		{
			resetScene(_fullReset);
		}

		_rendering = true;
		auto cmdList = _renderer->beginFrame();
		updateUBO(cmdList);

		// offscren render

		if (_selectedPipeline == 0)
		{
			_renderer->render(cmdList, _scenes[_currentScene], _rtPipeline, { _rtDescSet, _descSet }, _gui);
		}
		else if (_selectedPipeline == 1)
		{
			_renderer->render(cmdList, _scenes[_currentScene], _rasterPipeline, { _descSet }, _gui);
		}
		else if (_selectedPipeline == 2)
		{
			_renderer->render(cmdList, _scenes[_currentScene], _wireframePipeline, { _descSet }, _gui);
		}

		_renderer->endFrame(cmdList);
		_rendering = false;

		AstraDevice.waitIdle();
		for (auto& model_pair : _newModels)
		{
			addModelToScene(model_pair.first, model_pair.second);
		}
		_newModels.clear();

		for (auto& inst : _newInstances)
		{
			addInstanceToScene(inst);
		}
		_newInstances.clear();
	}
	destroy();
}

void DefaultApp::destroy()
{
	App::destroy();
	vkDestroyDescriptorSetLayout(AstraDevice.getVkDevice(), _descSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(AstraDevice.getVkDevice(), _rtDescSetLayout, nullptr);
	vkDestroyDescriptorPool(AstraDevice.getVkDevice(), _rtDescPool, nullptr);
}

void DefaultApp::createPipelines()
{
	// raytracing pipeline
	_rtPipeline = new Astra::RayTracingPipeline();
	((Astra::RayTracingPipeline*)_rtPipeline)->createPipeline(AstraDevice.getVkDevice(), { _rtDescSetLayout, _descSetLayout }, _alloc);

	// basic raster
	_rasterPipeline = new Astra::OffscreenRaster();
	((Astra::OffscreenRaster*)_rasterPipeline)->createPipeline(AstraDevice.getVkDevice(), { _descSetLayout }, _renderer->getOffscreenRenderPass());

	// wireframe
	_wireframePipeline = new WireframePipeline();
	((WireframePipeline*)_wireframePipeline)->createPipeline(AstraDevice.getVkDevice(), { _descSetLayout }, _renderer->getOffscreenRenderPass());

	_pipelines = { _rtPipeline, _rasterPipeline, _wireframePipeline };
}

void DefaultApp::createDescriptorSetLayout()
{
	// TODO rework into different descriptor for texturesss
	int nbTxt = _scenes[_currentScene]->getTextures().size();

	// Camera matrices
	_descSetLayoutBind.addBinding(SceneBindings::eGlobals, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
	// Obj descriptions
	_descSetLayoutBind.addBinding(SceneBindings::eObjDescs, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
	// Textures
	_descSetLayoutBind.addBinding(SceneBindings::eTextures, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nbTxt,
		VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

	_descSetLayout = _descSetLayoutBind.createLayout(AstraDevice.getVkDevice());
	_descPool = _descSetLayoutBind.createPool(AstraDevice.getVkDevice(), 1);
	_descSet = nvvk::allocateDescriptorSet(AstraDevice.getVkDevice(), _descPool, _descSetLayout);
}


void DefaultApp::updateDescriptorSet()
{
	std::vector<VkWriteDescriptorSet> writes;

	// Camera matrices and scene description
	VkDescriptorBufferInfo dbiUnif{ _globalsBuffer.buffer, 0, VK_WHOLE_SIZE };
	writes.emplace_back(_descSetLayoutBind.makeWrite(_descSet, SceneBindings::eGlobals, &dbiUnif));

	VkDescriptorBufferInfo dbiSceneDesc{ _scenes[_currentScene]->getObjDescBuff().buffer, 0, VK_WHOLE_SIZE };
	writes.emplace_back(_descSetLayoutBind.makeWrite(_descSet, SceneBindings::eObjDescs, &dbiSceneDesc));

	// All texture samplers
	std::vector<VkDescriptorImageInfo> diit;
	// for (int i = 0; i < _scenes.size(); i++) {

	for (auto& texture : _scenes[_currentScene]->getTextures())
	{
		diit.emplace_back(texture.descriptor);
	}
	//}
	writes.emplace_back(_descSetLayoutBind.makeWriteArray(_descSet, SceneBindings::eTextures, diit.data()));

	// Writing the information
	vkUpdateDescriptorSets(AstraDevice.getVkDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void DefaultApp::createRtDescriptorSet()
{
	const auto& device = AstraDevice.getVkDevice();
	_rtDescSetLayoutBind.addBinding(RtxBindings::eTlas, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
	_rtDescSetLayoutBind.addBinding(RtxBindings::eOutImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

	_rtDescPool = _rtDescSetLayoutBind.createPool(device);
	_rtDescSetLayout = _rtDescSetLayoutBind.createLayout(device);

	VkDescriptorSetAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocateInfo.descriptorPool = _rtDescPool;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts = &_rtDescSetLayout;
	vkAllocateDescriptorSets(device, &allocateInfo, &_rtDescSet);
}

void DefaultApp::updateRtDescriptorSet()
{
	VkAccelerationStructureKHR tlas = ((DefaultSceneRT*)_scenes[_currentScene])->getTLAS();
	VkWriteDescriptorSetAccelerationStructureKHR descASInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR };
	descASInfo.accelerationStructureCount = 1;
	descASInfo.pAccelerationStructures = &tlas;
	VkDescriptorImageInfo imageInfo{ {}, _renderer->getOffscreenColor().descriptor.imageView, VK_IMAGE_LAYOUT_GENERAL };

	std::vector<VkWriteDescriptorSet> writes;
	writes.emplace_back(_rtDescSetLayoutBind.makeWrite(_rtDescSet, RtxBindings::eTlas, &descASInfo));
	writes.emplace_back(_rtDescSetLayoutBind.makeWrite(_rtDescSet, RtxBindings::eOutImage, &imageInfo));
	vkUpdateDescriptorSets(AstraDevice.getVkDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

}

void DefaultApp::onResize(int w, int h)
{
	if (w == 0 || h == 0)
		return;

	if (_gui)
	{
		auto& imgui_io = ImGui::GetIO();
		imgui_io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
	}

	// wait until finishing tasks
	AstraDevice.waitIdle();
	AstraDevice.queueWaitIdle();

	// request swapchain image
	_renderer->requestSwapchainImage(w, h);

	_scenes[_currentScene]->getCamera()->setWindowSize(w, h);

	_renderer->createOffscreenRender(_alloc);

	_renderer->updatePostDescriptorSet();
	updateRtDescriptorSet();
	updateDescriptorSet();

	_renderer->createDepthBuffer();
	_renderer->createFrameBuffers();
}

void DefaultApp::onMouseMotion(int x, int y)
{
	auto s = _scenes[_currentScene];
	int delta[2] = { x - _lastMousePos[0], y - _lastMousePos[1] };
	_lastMousePos[0] = x;
	_lastMousePos[1] = y;
	s->getCamera()->handleMouseInput(_mouseButtons, delta, 0, _inputMods);
}

void DefaultApp::onMouseButton(int button, int action, int mods)
{
	_mouseButtons[button] = action == GLFW_PRESS;
	auto s = _scenes[_currentScene];
	int _[2];
	_[0] = _[1] = 0;
	_inputMods = mods;
	s->getCamera()->handleMouseInput(_mouseButtons, _, 0, _inputMods);
}

void DefaultApp::onMouseWheel(int x, int y)
{
	auto s = _scenes[_currentScene];
	int _[2];
	_[0] = _[1] = 0;
	s->getCamera()->handleMouseInput(_mouseButtons, _, 10 * y, _inputMods);
}

void DefaultApp::onKeyboard(int key, int scancode, int action, int mods)
{
	// pass;
}

void DefaultApp::onFileDrop(int count, const char** paths)
{
	if (count == 1)
	{
		std::string string_path(paths[0]);
		// check for obj object
		if (string_path.substr(string_path.size() - 4, string_path.size()) == ".obj")
		{
			addModelToScene(string_path);
		}
	}
	else
	{
		Astra::Log("Only one model at a time please!", Astra::INFO); // right now only one.
	}
}

void DefaultApp::setCurrentSceneIndex(int i)
{
	int nbTxt = _scenes[_currentScene]->getTextures().size();
	Astra::App::setCurrentSceneIndex(i);
	if (_status == Astra::AppStatus::Running)
	{
		scheduleReset(nbTxt != _scenes[_currentScene]->getTextures().size());
	}
}

void DefaultApp::resetScene(bool recreatePipelines)
{
	((DefaultSceneRT*)_scenes[_currentScene])->createBottomLevelAS();
	((DefaultSceneRT*)_scenes[_currentScene])->createTopLevelAS();
	_descSetLayoutBind.clear();
	_rtDescSetLayoutBind.clear();
	createDescriptorSetLayout();
	updateDescriptorSet();
	createRtDescriptorSet();
	updateRtDescriptorSet();

	if (recreatePipelines)
	{
		destroyPipelines();
		createPipelines();
	}
	_needsReset = false;
}

void DefaultApp::scheduleReset(bool recreatePipelines)
{
	_needsReset = true;
	_fullReset = recreatePipelines;
}

void DefaultApp::addModelToScene(const std::string& filepath, const glm::mat4& transform)
{
	if (_rendering)
	{
		_newModels.push_back(std::make_pair(filepath, transform));
	}
	else
	{
		int currentTxtSize = _scenes[_currentScene]->getTextures().size();
		_scenes[_currentScene]->loadModel(filepath, transform);
		resetScene(_scenes[_currentScene]->getTextures().size() > currentTxtSize);
	}
}

void DefaultApp::addInstanceToScene(const Astra::MeshInstance& instance)
{
	if (_rendering)
	{
		_newInstances.push_back(instance);
	}
	else
	{
		_scenes[_currentScene]->addInstance(instance);
		resetScene();
	}
}

int& DefaultApp::getSelectedPipelineRef()
{
	return _selectedPipeline;
}
