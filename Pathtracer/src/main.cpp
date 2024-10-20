#include <ptApp.h>
#include <Utils.h>
#include <nvh/fileoperations.hpp>
#include <ptScene.h>
#include <gui.h>
#include <glm/ext/matrix_transform.hpp>

int main() {
	Astra::DeviceCreateInfo createInfo{};
	AstraDevice.initDevice(createInfo);

	PtApp app;

	Astra::Renderer* renderer = new Astra::Renderer();
	Astra::Camera cam;
	Astra::CameraController* cameraController = new Astra::FreeCameraController(cam);

	Astra::SceneRT* scene = new PtScene();


	Astra::Light* lightBulb = new Astra::PointLight(glm::vec3(1), 3.0f);

	cameraController->setLookAt(glm::vec3(5, 1.5, 12), glm::vec3(0.0), glm::vec3(0, 1, 0));

	scene->loadModel(nvh::findFile("assets/cornell.obj", Astra::defaultSearchPaths));
	//scene->loadModel("C:\\Users\\pingu\\AstraExamples\\SceneEditor\\assets\\plane.obj");
	//scene->loadModel(nvh::findFile("assets/bombilla.obj", Astra::defaultSearchPaths));
	//scene->loadModel("C:\\Users\\pingu\\AstraExamples\\SceneEditor\\assets\\coche.obj");
	//scene->loadModel(nvh::findFile("assets/streetlight.obj", Astra::defaultSearchPaths), glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), -3.14f/2.0f, glm::vec3(0.f, 1.f, 0.f)));
	scene->addLight(lightBulb);

	scene->setCamera(cameraController);

	Astra::GuiController* gui = new PtGui();

	try {
		app.init({ scene }, renderer, gui);
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
	delete gui;
}