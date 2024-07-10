#include <ptapp.h>
#include <ptscene.h>
#include <nvh/fileoperations.hpp>
#include <nvpsystem.hpp>
#include <Utils.h>
#include <gui.h>

int main() {
	Astra::DeviceCreateInfo dci{};
	AstraDevice.initDevice(dci);
	
	PathtracerApp app;
	Astra::Renderer  * renderer = new Astra::Renderer();
	PTScene * scene = new PTScene();

	Astra::DirectionalLight * sun = new Astra::DirectionalLight(glm::vec3(1.0f), 0.6f, glm::vec3(1));
	Astra::Camera camera;
	Astra::OrbitCameraController cameraController(camera);

	scene->addLight(sun);
	scene->setCamera(&cameraController);

	scene->loadModel(nvh::findFile("assets/plane.obj", Astra::defaultSearchPaths));
	scene->loadModel(nvh::findFile("assets/cube.obj", Astra::defaultSearchPaths));

	PtGui * gui = new PtGui();

	app.init({ scene }, renderer, gui);

	try {
		app.run();
	}
	catch (...) {
		app.destroy();
		Astra::Log("Error ocurred", Astra::ERR);
	}
	AstraDevice.destroy();
}