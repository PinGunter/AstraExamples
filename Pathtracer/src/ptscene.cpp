#include <ptScene.h>
#include <Utils.h>
#include <glm/gtc/type_ptr.hpp>

void PtScene::update(const Astra::CommandList& cmdList, float delta)
{
	_updated = 0;
	// updating lights
	_lightsUniform = {};
	//_lightsUniform.nLights = _lights.size();
	for (int i = 0; i < _lights.size(); i++)
	{
		_updated |= _lights[i]->update(delta);
		_lightsUniform.lights[i] = _lights[i]->getLightSource();
	}
	updateLightsUBO(cmdList);

	// updating camera
	_updated |= _camera->update(delta);
	updateCameraUBO(cmdList);

	// updating instances
	for (auto& i : _instances)
	{
		_updated |= i.update(delta);
	}	
	
	// acceleration structure updates
	std::vector<int> asupdates;
	for (int i = 0; i < _instances.size(); i++)
	{
		if (_instances[i].update(delta))
		{
			asupdates.push_back(i);
		}
	}

	for (int i : asupdates)
	{
		updateTopLevelAS(i);
	}
}

void PtScene::draw(Astra::RenderContext<PushConstantRay>& renderContext)
{
	++_frames;
	_frames = _updated ? -1 : _frames;
	renderContext.pushConstant.frame = _frames;
	renderContext.pushConstant.clearColor2 = glm::vec4(_clearColor2,1.0f);
	Astra::SceneRT::draw(renderContext);
}

float* PtScene::getClearColor2()
{
	return glm::value_ptr(_clearColor2);
}
