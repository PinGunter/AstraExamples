#include <myApp.h>
#include <Utils.h>
#include <myPipelines.h>
#include <Globals.h>


void DefaultApp::run()
{
	while (!Astra::Input.windowShouldClose())
	{

		Astra::Input.pollEvents();
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

		// update scene
		_scenes[_currentScene]->update(cmdList);

		// offscren render

		if (_selectedPipeline == RT)
		{
			_renderer->render(cmdList, _scenes[_currentScene], _pipelines[_selectedPipeline], { _rtDescSet, _descSet }, _gui);
		}
		else if (_selectedPipeline == RASTER || _selectedPipeline == WIRE || _selectedPipeline == NORMALS)
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

		for (auto& inst : _instToRemove) {
			removeInstance(inst);
		}
		_instToRemove.clear();
	}
	destroy();
}

void DefaultApp::createPipelines()
{
	// raytracing pipeline
	Astra::Pipeline* rtPl = new Astra::RayTracingPipeline();
	((Astra::RayTracingPipeline*)rtPl)->create(AstraDevice.getVkDevice(), { _rtDescSetLayout, _descSetLayout }, _alloc);

	// basic raster
	Astra::Pipeline* rasterPl = new Astra::OffscreenRaster();
	((Astra::OffscreenRaster*)rasterPl)->create(AstraDevice.getVkDevice(), { _descSetLayout }, _renderer->getOffscreenRenderPass());

	// wireframe
	Astra::Pipeline* wirePl = new WireframePipeline();
	((WireframePipeline*)wirePl)->create(AstraDevice.getVkDevice(), { _descSetLayout }, _renderer->getOffscreenRenderPass());

	// normals
	Astra::Pipeline* normalPl = new NormalPipeline();
	((NormalPipeline*)normalPl)->create(AstraDevice.getVkDevice(), { _descSetLayout }, _renderer->getOffscreenRenderPass());

	_pipelines = { rtPl, rasterPl, wirePl, normalPl };
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
	Astra::App::setCurrentSceneIndex(i);
	if (_status == Astra::AppStatus::Running)
	{
		scheduleReset(true);
	}
}

void DefaultApp::resetScene(bool recreatePipelines)
{
	Astra::AppRT::resetScene(recreatePipelines);
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
		resetScene(_scenes[_currentScene]->getTextures().size() != currentTxtSize);
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

void DefaultApp::removeInstance(int instance)
{
	if (_rendering) {
		_instToRemove.push_back(instance);
	}
	else {
		int currentTxtSize = _scenes[_currentScene]->getTextures().size();
		_scenes[_currentScene]->removeInstance(_scenes[_currentScene]->getInstances()[instance]);
		resetScene(_scenes[_currentScene]->getTextures().size() != currentTxtSize);
	}
}
