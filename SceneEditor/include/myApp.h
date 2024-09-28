#pragma once
#include <AppRT.h>


class DefaultApp : public Astra::AppRT {
protected:
	enum Pipeline { RT, RASTER, WIRE, NORMALS };
	// models and instances to load or remove after frame execution, when gpu is ready
	std::vector< Astra::MeshInstance> _newInstances;
	std::vector<Astra::Mesh> _newModels;
	std::vector<std::pair<std::string, glm::mat4>> _newModelLoads;
	std::vector<int> _instToRemove;
	bool _rendering = false;
	bool _needsReset = false;
	bool _fullReset = false;

	void createPipelines() override;

	void onFileDrop(int count, const char** paths) override;

	void resetScene(bool recreatePipelines = false) override;
	void scheduleReset(bool recreatePipelines = false);

	float _frameTime;
	bool _recordingStats = false;

	std::vector<float> _ftArray;

	void computeStats(int& triCount, int& lightCount, float& avgFrameTime);
	void saveRtStats();
	void saveRasterStats();
	void onKeyboard(int key, int scancode, int action, int mods) override;

public:
	void run() override;

	// add models / instances in runtime
	void addModelLoadToScene(const std::string& filepath, const glm::mat4& transform = glm::mat4(1.0f));
	void addShape(Astra::Mesh&);
	void addInstanceToScene(const Astra::MeshInstance& instance);
	void removeInstance(int instance);

	void setCurrentSceneIndex(int i) override;

	float getFrameTime();


	bool getRecordingStats();
	void setRecordingStats(bool b);

	void saveStats();

};