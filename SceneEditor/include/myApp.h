#pragma once
#include <AppRT.h>


class DefaultApp : public Astra::AppRT {
protected:
	enum Pipeline { RT, RASTER, WIRE, NORMALS };
	// models and instances to load or remove after frame execution, when gpu is ready
	std::vector< Astra::MeshInstance> _newInstances;
	std::vector<std::pair<std::string, glm::mat4>> _newModels;
	std::vector<int> _instToRemove;
	bool _rendering = false;
	bool _needsReset = false;
	bool _fullReset = false;


	// camera and input controls
	bool _mouseButtons[3] = { 0 };
	int _lastMousePos[2] = { 0 };
	int _inputMods{ 0 };

	void createPipelines() override;

	void onMouseMotion(int x, int y) override;
	void onMouseButton(int button, int action, int mods) override;
	void onMouseWheel(int x, int y) override;
	void onKeyboard(int key, int scancode, int action, int mods) override;
	void onFileDrop(int count, const char** paths) override;

	void resetScene(bool recreatePipelines = false) override;
	void scheduleReset(bool recreatePipelines = false);

public:
	void init(const std::vector< Astra::Scene*>& scenes, Astra::Renderer* renderer, Astra::GuiController* gui = nullptr) override;
	void run() override;

	// add models / instances in runtime
	void addModelToScene(const std::string& filepath, const glm::mat4& transform = glm::mat4(1.0f));
	void addInstanceToScene(const Astra::MeshInstance& instance);
	void removeInstance(int instance);

	void setCurrentSceneIndex(int i) override;

	int& getSelectedPipelineRef();
};