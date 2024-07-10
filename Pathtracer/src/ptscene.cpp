#include "ptscene.h"

bool PTScene::getCameraUpdate()
{
	return _cameraUpdate;
}

void PTScene::update(const Astra::CommandList& cmdList)
{
	// updating lights
	_lightsUniform = {};
	//_lightsUniform.nLights = _lights.size();
	for (int i = 0; i < _lights.size(); i++)
	{
		_lights[i]->update();
		_lightsUniform.lights[i] = _lights[i]->getLightSource();
	}
	updateLightsUBO(cmdList);

	// updating camera
	_cameraUpdate = _camera->update();
	updateCameraUBO(cmdList);

	// updating instances
	for (auto& i : _instances)
	{
		i.update();
	}

	std::vector<int> asupdates;
	for (int i = 0; i < _instances.size(); i++)
	{
		if (_instances[i].update())
		{
			asupdates.push_back(i);
		}
	}

	for (int i : asupdates)
	{
		updateTopLevelAS(i);
	}
}

void PTScene::draw(Astra::RenderContext<PushConstantRay>& renderContext)
{
	renderContext.pushConstant.nLights = _lights.size();
	if (_cameraUpdate) {
		renderContext.pushConstant.frame = -1;
	}
	else {
		renderContext.pushConstant.frame ++;
	}
	renderContext.pushConstants();
}

void PTScene::draw(Astra::RenderContext<PushConstantRaster>& renderContext)
{
	Scene::draw(renderContext);
}
