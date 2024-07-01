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
	Astra::Scene* scene = new Astra::SceneRT();
	Astra::SceneRT* scene2 = new Astra::SceneRT();
	Astra::Renderer* renderer = new Astra::Renderer();
	Astra::GuiController* gui = new BasiGui();

	// Scene creation
	Astra::Camera cam;
	Astra::CameraController* camera = new Astra::FreeCameraController(cam);
	Astra::Camera cam2;
	Astra::CameraController* camera2 = new Astra::FreeCameraController(cam2);

	Astra::Light* pointLight = new Astra::PointLight(glm::vec3(1.0f), 60.0f);
	pointLight->translate(glm::vec3(10, 15, 20));
	pointLight->setColor(glm::vec3(1, 0.7, 0.7));
	Astra::Light* pointLight2 = new Astra::PointLight(glm::vec3(1.0f), 60.0f);
	pointLight2->translate(glm::vec3(-10, 15, -20));
	pointLight2->setColor(glm::vec3(0.7, 0.7, 1));
	Astra::Light* sun = new Astra::DirectionalLight(glm::vec3(1.0f), .6f, glm::vec3(1.0f));
	sun->translate(glm::vec3(10, 15, 20));

	// Setup camera
	auto windowSize = AstraDevice.getWindowSize();
	camera->setWindowSize(windowSize[0], windowSize[1]);
	camera->setLookAt(glm::vec3(5.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	camera2->setWindowSize(windowSize[0], windowSize[1]);
	camera2->setLookAt(glm::vec3(5.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	camera2->setFov(30);

	scene->setCamera(camera);
	scene->addLight(pointLight);
	scene->addLight(pointLight2);
	//scene->addLight(sun);

	scene2->setCamera(camera2);
	scene2->addLight(sun);

	scene->loadModel(nvh::findFile("assets/mono2.obj", defaultSearchPaths, true));
	scene->loadModel(nvh::findFile("assets/espejo.obj", defaultSearchPaths, true), glm::rotate(glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)), glm::vec3(-3, 2, -3)), -glm::pi<float>() / 4.0f, glm::vec3(0, 1, 0)));
	scene->loadModel(nvh::findFile("assets/plane.obj", defaultSearchPaths, true), glm::translate(glm::mat4(1.0f), glm::vec3(0, -1, 0)));

	scene2->loadModel(nvh::findFile("assets/cube.obj", defaultSearchPaths, true), glm::translate(glm::mat4(1.0f), glm::vec3(0, 0.5, 0)));
	scene2->loadModel(nvh::findFile("assets/plane.obj", defaultSearchPaths, true));

	try
	{
		app.init({ scene, scene2 }, renderer, gui);
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
	delete scene;
	//delete scene2;
	delete sun;
	//delete camera2;
	delete pointLight;
	delete pointLight2;
	delete gui;
}
