#include <myApp.h>
#include <Utils.h>
#include <WireframePL.h>

void DefaultApp::init(const std::vector<Astra::Scene*>& scenes, Astra::Renderer* renderer, Astra::GuiController* gui)
{
	// init base app, copies scenes renderer and gui
	// also inits the scenes and sets callbacks
	Astra::AppRT::init(scenes, renderer, gui);

	// Scene -> GPU information || Uniforms
	// camera uniforms
	//createUBO();

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

		if (_selectedPipeline == RT)
		{
			_renderer->render(cmdList, _scenes[_currentScene], _pipelines[_selectedPipeline], { _rtDescSet, _descSet }, _gui);
		}
		else if (_selectedPipeline == RASTER)
		{
			_renderer->render(cmdList, _scenes[_currentScene], _pipelines[_selectedPipeline], { _descSet }, _gui);
		}
		else if (_selectedPipeline == WIRE)
		{
			_renderer->render(cmdList, _scenes[_currentScene], _pipelines[_selectedPipeline], { _descSet }, _gui);
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
	Astra::Pipeline* rtPl = new Astra::RayTracingPipeline();
	((Astra::RayTracingPipeline*)rtPl)->createPipeline(AstraDevice.getVkDevice(), { _rtDescSetLayout, _descSetLayout }, _alloc);

	// basic raster
	Astra::Pipeline* rasterPl = new Astra::OffscreenRaster();
	((Astra::OffscreenRaster*)rasterPl)->createPipeline(AstraDevice.getVkDevice(), { _descSetLayout }, _renderer->getOffscreenRenderPass());

	// wireframe
	Astra::Pipeline* wirePl = new WireframePipeline();
	((WireframePipeline*)wirePl)->createPipeline(AstraDevice.getVkDevice(), { _descSetLayout }, _renderer->getOffscreenRenderPass());

	_pipelines = { rtPl, rasterPl, wirePl };
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
	((Astra::DefaultSceneRT*)_scenes[_currentScene])->createBottomLevelAS();
	((Astra::DefaultSceneRT*)_scenes[_currentScene])->createTopLevelAS();
	_descSetLayoutBind.clear();
	_rtDescSetLayoutBind.clear();
	createDescriptorSetLayout();
	updateDescriptorSet();
	createRtDescriptorSetLayout();
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
