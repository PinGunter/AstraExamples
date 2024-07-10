#pragma once
#include <AppRT.h>

class PathtracerApp : public Astra::AppRT {
protected:
	void createPipelines() override;

public:
	void run() override;
};