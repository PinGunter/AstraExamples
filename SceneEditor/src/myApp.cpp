#include <myApp.h>
#include <Utils.h>
#include <myPipelines.h>
#include <Globals.h>
#include <fstream>
#include <filesystem>

void DefaultApp::computeStats(int& triCount, int& lightCount, float& avgFrameTime)
{
	triCount = 0;
	avgFrameTime = 0;
	lightCount = _scenes[_currentScene]->getLights().size();
	for (auto ft : _ftArray) {
		avgFrameTime += ft;
	}
	avgFrameTime /= _ftArray.size();

	for (auto& instances : _scenes[_currentScene]->getInstances()) {
		if (instances.getVisible()) {
			auto& model = _scenes[_currentScene]->getModels()[instances.getMeshIndex()];
			triCount += model.indices.size() / 3.0f;
		}
	}
}

void DefaultApp::run()
{
	while (!Astra::Input.windowShouldClose())
	{
		auto startT = std::chrono::high_resolution_clock::now();
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
		_scenes[_currentScene]->update(cmdList, _frameTime);

		// offscren render

		if (_pipelines[_selectedPipeline]->doesRayTracing())
		{
			_renderer->render(cmdList, _scenes[_currentScene], _pipelines[_selectedPipeline], { _rtDescSet, _descSet }, _gui);
		}
		else
		{
			_renderer->render(cmdList, _scenes[_currentScene], _pipelines[_selectedPipeline], { _descSet }, _gui);
		}

		_renderer->endFrame(cmdList);
		_rendering = false;

		AstraDevice.waitIdle();
		for (auto& model_pair : _newModelLoads)
		{
			addModelLoadToScene(model_pair.first, model_pair.second);
		}
		_newModelLoads.clear();
		for (auto& model : _newModels)
		{
			addShape(model);
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

		_frameTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startT).count() / 1e6; // microseconds and then divide by 1000.0f for better precision
		if (_recordingStats) {
			_ftArray.push_back(_frameTime * 1000);
		}
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
		std::filesystem::path path(string_path);
		// check for obj object
		if (path.extension().string() == ".obj")
		{
			addModelLoadToScene(string_path);
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

float DefaultApp::getFrameTime()
{
	return _frameTime;
}

void DefaultApp::saveRtStats()
{
	std::ofstream file("rtStats.data");
	file << "RayTracing Stats" << std::endl;

	float avg;
	int triCount, nLights;
	computeStats(triCount, nLights, avg);
	auto windowSize = AstraDevice.getWindowSize();
	file << "Resolution: " << windowSize[0] << " x " << windowSize[1] << std::endl;
	file << "Average Frame Time (ms): " << avg << std::endl;
	file << "Triangle Count: " << triCount << std::endl;
	file << "Number of lights: " << nLights << std::endl;
	file << "Raytraced shadows: " << (_renderer->getUseShadows() ? "Yes" : "No") << std::endl;
	file << "Raytraced reflections and refractions: " << _renderer->getMaxDepth() << " ray bounces" << std::endl;

	file << "============RAW FRAME TIME DATA==============" << std::endl;
	for (auto ft : _ftArray) {
		file << ft << std::endl;
	}

	file.close();
}

void DefaultApp::saveRasterStats()
{
	std::ofstream file("rasterStats.data");
	file << "Raster Stats" << std::endl;

	float avg;
	int triCount, nLights;
	computeStats(triCount, nLights, avg);
	auto windowSize = AstraDevice.getWindowSize();
	file << "Resolution: " << windowSize[0] << " x " << windowSize[1] << std::endl;
	file << "Average Frame Time (ms): " << avg << std::endl;
	file << "Triangle Count: " << triCount << std::endl;
	file << "Number of lights: " << nLights << std::endl;
	file << "============RAW FRAME TIME DATA==============" << std::endl;
	for (auto ft : _ftArray) {
		file << ft << std::endl;
	}
	file.close();
}

void DefaultApp::onKeyboard(int key, int scancode, int action, int mods)
{
	if (key == Astra::Keys::Key_Escape) {
		Astra::Input.setWindowShouldClose(true);
	}
}

bool DefaultApp::getRecordingStats()
{
	return _recordingStats;
}

void DefaultApp::setRecordingStats(bool b)
{
	_recordingStats = b;
	if (_recordingStats) // if re-recording we should clear the data
	{
		_ftArray.clear();
	}
}

void DefaultApp::saveStats()
{
	if (_pipelines[_selectedPipeline]->doesRayTracing()) {
		saveRtStats();
	}
	else {
		saveRasterStats();
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

void DefaultApp::addModelLoadToScene(const std::string& filepath, const glm::mat4& transform)
{
	if (_rendering)
	{
		_newModelLoads.push_back(std::make_pair(filepath, transform));
	}
	else
	{
		int currentTxtSize = _scenes[_currentScene]->getTextures().size();
		_scenes[_currentScene]->loadModel(filepath, transform);
		resetScene(_scenes[_currentScene]->getTextures().size() != currentTxtSize);
	}
}

void DefaultApp::addShape(Astra::Mesh& model) {
	if (_rendering)
	{
		_newModels.push_back(model);
	}
	else
	{
		_scenes[_currentScene]->addShape(model);
		resetScene(true);
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
