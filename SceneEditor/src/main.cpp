#include <nvpsystem.hpp>
#include <nvh/fileoperations.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <myApp.h>
#include <Scene.h>
#include <myGui.h>
#include <Utils.h>

// search paths for finding files
std::vector<std::string> defaultSearchPaths = {
	NVPSystem::exePath() + PROJECT_RELDIRECTORY,
	NVPSystem::exePath() + PROJECT_RELDIRECTORY "..",
	std::string(PROJECT_NAME),
};

int main(int argc, char** argv)
{
	// Device Initialization
	Astra::DeviceCreateInfo createInfo{};
	AstraDevice.initDevice(createInfo);

	// App creation
	DefaultApp app;
	Astra::DefaultSceneRT* scene = new Astra::DefaultSceneRT();
	Astra::DefaultSceneRT* scene2 = new Astra::DefaultSceneRT();
	Astra::Renderer* renderer = new Astra::Renderer();
	Astra::GuiController* gui = new BasiGui();

	// Scene creation
	Astra::Camera cam;
	Astra::CameraController* camera = new Astra::OrbitCameraController(cam);
	Astra::Camera cam2;
	Astra::CameraController* camera2 = new Astra::OrbitCameraController(cam2);

	Astra::Light* pointLight = new Astra::PointLight(glm::vec3(1.0f), 60.0f);
	pointLight->translate(glm::vec3(10, 15, 20));

	Astra::Light* sun = new Astra::DirectionalLight(glm::vec3(1.0f), .6f, glm::vec3(1.0f));
	sun->translate(glm::vec3(10, 15, 20));

	// Setup camera
	auto windowSize = AstraDevice.getWindowSize();
	camera->setWindowSize(windowSize[0], windowSize[1]);
	camera->setLookAt(glm::vec3(5.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	camera2->setWindowSize(windowSize[0], windowSize[1]);
	camera2->setLookAt(glm::vec3(5.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	scene->setCamera(camera);
	scene->addLight(pointLight);

	scene2->setCamera(camera2);
	scene2->addLight(sun);

	scene->loadModel(nvh::findFile("media/scenes/mono2.obj", defaultSearchPaths, true));
	scene->loadModel(nvh::findFile("media/scenes/espejo.obj", defaultSearchPaths, true));
	scene->loadModel(nvh::findFile("media/scenes/plane2.obj", defaultSearchPaths, true), glm::translate(glm::mat4(1.0f), glm::vec3(0, -1, 0)));

	scene2->loadModel(nvh::findFile("media/scenes/lizardmech.obj", defaultSearchPaths, true));
	scene2->loadModel(nvh::findFile("media/scenes/plane2.obj", defaultSearchPaths, true));

	app.init({ scene, scene2 }, renderer, gui);

	try
	{
		app.run();
	}
	catch (...)
	{
		app.destroy();
		Astra::Log("Exception ocurred", Astra::LOG_LEVELS::ERR);
	}

	AstraDevice.destroy();

	delete camera;
	delete renderer;
	delete scene;
	delete scene2;
	delete sun;
	delete camera2;
	delete pointLight;
	delete gui;
}
