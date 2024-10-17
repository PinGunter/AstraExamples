#include <ptApp.h>
#include <Globals.h>
#include <ptPipeline.h>
void PtApp::onKeyboard(int key, int scancode, int action, int mods)
{
	if (key == Astra::Key_Tab && action == GLFW_PRESS) {
		_selectedPipeline = (_selectedPipeline + 1) % _pipelines.size();
	}
}

void PtApp::createPipelines()
{
	// raytracing pipeline
	Astra::Pipeline* rtPl = new PtPipeline();
	((Astra::RayTracingPipeline*)rtPl)->create(AstraDevice.getVkDevice(), { _rtDescSetLayout, _descSetLayout }, _alloc);

	// basic raster
	Astra::Pipeline* rasterPl = new Astra::OffscreenRaster();
	((Astra::OffscreenRaster*)rasterPl)->create(AstraDevice.getVkDevice(), { _descSetLayout }, _renderer->getOffscreenRenderPass());

	_pipelines = { rtPl, rasterPl };
}

void PtApp::run()
{
	while (!Astra::Input.windowShouldClose())
	{
		auto startT = std::chrono::high_resolution_clock::now();
		Astra::Input.pollEvents();
		if (isMinimized())
		{
			continue;
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
		_frameTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - startT).count() / 1e6; // microseconds and then divide by 1000.0f for better precision

		}
	
	destroy();
}
