#pragma once
#include <GuiController.h>
#include <myApp.h>

class BasicGui : public Astra::GuiController {
	bool _showGuizmo{ false };
	int _node{ 0 };
	int _light{ 0 };
	int _ncopies{ 1 };
	bool _handlingNodes{ true };
	void startDockableWindow();
	void ShapeCreator(DefaultApp* app);
	void ModelAndInstances(DefaultApp* app);
public:
	void draw(Astra::App* app) override;
};