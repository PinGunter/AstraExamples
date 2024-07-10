#pragma once
#include <Scene.h>

class PTScene : public Astra::SceneRT {
	bool _cameraUpdate = false;
public:
	bool getCameraUpdate();
	void update(const Astra::CommandList& cmdList) override;
	void draw(Astra::RenderContext<PushConstantRay>& renderContext) override;
	void draw(Astra::RenderContext<PushConstantRaster>& renderContext) override;

};