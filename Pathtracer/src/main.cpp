#include <ptApp.h>
#include <Utils.h>
#include <nvh/fileoperations.hpp>
#include <ptScene.h>

int main() {
	Astra::DeviceCreateInfo createInfo{};
	AstraDevice.initDevice(createInfo);

	PtApp app;

	Astra::Renderer* renderer = new Astra::Renderer();
	Astra::Camera cam;
	Astra::CameraController* cameraController = new Astra::OrbitCameraController(cam);

	Astra::SceneRT* scene = new PtScene();


	Astra::Light* lightBulb = new Astra::PointLight(glm::vec3(1), 3.0f);

	cameraController->setLookAt(glm::vec3(5, 1.5, 12), glm::vec3(0.0), glm::vec3(0, 1, 0));

	scene->loadModel(nvh::findFile("assets/cornell.obj", Astra::defaultSearchPaths));
	scene->addLight(lightBulb);
	scene->setCamera(cameraController);

	try {
		app.init({ scene }, renderer);
		app.run();
	}
	catch (const std::exception& exc) {
		app.destroy();
		Astra::Log("Exception ocurred: " + std::string(exc.what()), Astra::ERR);
	}
	AstraDevice.destroy();

	delete renderer;
	delete cameraController;
	delete lightBulb;
}