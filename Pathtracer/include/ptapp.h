#pragma once
#include <AppRT.h>

class PtApp : public Astra::AppRT {
protected:
	bool _rendering = false;
	float _frameTime{  };

	void onKeyboard(int key, int scancode, int action, int mods) override;
	void createPipelines() override;
public:
	void run() override;
};