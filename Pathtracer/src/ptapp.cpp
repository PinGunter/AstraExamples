#include "ptapp.h"
#include <Pipeline.h>
#include <ptpipeline.h>

void PathtracerApp::createPipelines()
{
	Astra::RasterPipeline* raster = new Astra::OffscreenRaster();
	raster->create(AstraDevice.getVkDevice(), { _descSetLayout }, _renderer->getOffscreenRenderPass());
	PathtracingPipeline* ptPipeline = new PathtracingPipeline();
	ptPipeline->create(AstraDevice.getVkDevice(), { _rtDescSetLayout, _descSetLayout }, _alloc);
	_pipelines = {  ptPipeline, raster };
}

void PathtracerApp::run()
{
	while (!Astra::Input.windowShouldClose()) {
		Astra::Input.pollEvents();

		auto cmdList = _renderer->beginFrame();
		_scenes[_currentScene]->update(cmdList);
		
		if (_pipelines[_selectedPipeline]->doesRayTracing()) {
			_renderer->render(cmdList, _scenes[_currentScene], _pipelines[_selectedPipeline], { _rtDescSet, _descSet }, _gui);
		}
		else {
			_renderer->render(cmdList, _scenes[_currentScene], _pipelines[_selectedPipeline], { _descSet }, _gui);
		}

		_renderer->endFrame(cmdList);
	}
	destroy();
}
