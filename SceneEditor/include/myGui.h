#pragma once
#include <GuiController.h>

class BasiGui : public Astra::GuiController {
	bool _showGuizmo{ false };
	int _node{ 0 };
	int _light{ 0 };
	int _ncopies{ 1 };
	bool _handlingNodes{ true };
public:
	void draw(Astra::App* app) override;
};