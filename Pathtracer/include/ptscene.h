#pragma once
#include <Scene.h>

class PtScene : public Astra::SceneRT {
protected:
	bool _updated{false};
	int _frames = 0;
public:
	void update(const Astra::CommandList& cmdList, float delta) override;
	void draw(Astra::RenderContext<PushConstantRay>& renderContext) override;
};