#include <nvpsystem.hpp>
#include <nvh/fileoperations.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <myApp.h>
#include <Scene.h>
#include <myGui.h>
#include <Utils.h>

int main(int argc, char** argv)
{
	// Device Initialization
	Astra::DeviceCreateInfo createInfo{};
	AstraDevice.initDevice(createInfo);

	// App creation
	DefaultApp app;
	Astra::Renderer* renderer = new Astra::Renderer();
	Astra::GuiController* gui = new BasicGui();
	Astra::Camera cam;
	Astra::CameraController* camera = new Astra::FreeCameraController(cam);

	// Scene creation
	Astra::SceneRT* sencilla = new Astra::SceneRT();
	Astra::SceneRT* polis_100 = new Astra::SceneRT();
	Astra::SceneRT* polis_5m = new Astra::SceneRT();

	// escena sencilla 
	Astra::Light* sun = new Astra::DirectionalLight(glm::vec3(1.0f), 0.6f, glm::vec3(1.0f));

	sencilla->loadModel(nvh::findFile("assets/escenas/escena1.obj", Astra::defaultSearchPaths));

	sencilla->addLight(sun);
	camera->setLookAt(glm::vec3(5, 1.5, 12), glm::vec3(0.0), glm::vec3(0, 1, 0));
	sencilla->setCamera(camera);

	polis_100->loadModel(nvh::findFile("assets/plane.obj", Astra::defaultSearchPaths));
	polis_100->addLight(sun);
	polis_100->setCamera(camera);

	polis_5m->loadModel(nvh::findFile("assets/plane.obj", Astra::defaultSearchPaths));
	polis_5m->addLight(sun);
	polis_5m->setCamera(camera);


	try
	{
		app.init({ sencilla, polis_5m, polis_100 }, renderer, gui);
		app.run();
	}
	catch (const std::exception& exc)
	{
		app.destroy();
		Astra::Log("Exception ocurred: " + std::string(exc.what()), Astra::LOG_LEVELS::ERR);
	}

	AstraDevice.destroy();

	delete camera;
	delete renderer;

	delete gui;
}
