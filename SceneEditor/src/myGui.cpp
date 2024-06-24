#include <myGui.h>
#include <myApp.h>
#include <glm/gtc/type_ptr.hpp>
#include <myScene.h>

using namespace Astra;
void BasiGui::draw(App* app)
{
	DefaultApp* dapp = dynamic_cast<DefaultApp*>(app);
	DefaultSceneRT* scene = (DefaultSceneRT*)dapp->getCurrentScene();

	ImGui::Begin("Inspector");

	if (ImGui::BeginTabBar("###Tabbar")) {
		if (ImGui::BeginTabItem("Renderer")) {
			ImGui::ColorEdit3("Clear Color", glm::value_ptr(app->getRenderer()->getClearColorRef()));

			ImGui::SliderInt("Max Ray bounces", &app->getRenderer()->getMaxDepthRef(), 0, 30);


			ImGui::Separator();

			ImGui::Text("Select rendering pipeline");
			ImGui::RadioButton("RayTracing", &dapp->getSelectedPipelineRef(), 0);
			ImGui::RadioButton("Raster", &dapp->getSelectedPipelineRef(), 1);
			ImGui::RadioButton("Wireframe", &dapp->getSelectedPipelineRef(), 2);

			ImGui::EndTabItem();

		}
		if (ImGui::BeginTabItem("Performance")) {
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::End();


	ImGui::Begin("Scene");

	ImGui::Text((std::string("Current Scene ") + std::to_string(app->getCurrentSceneIndex())).c_str());
	ImGui::SameLine();
	if (ImGui::Button("Switch")) {
		app->setCurrentSceneIndex((app->getCurrentSceneIndex() + 1) % 2);
		_node = 0;
	}

	if (ImGui::BeginListBox("Instances")) {

		for (int i = 0; i < scene->getInstances().size(); i++) {
			auto& inst = scene->getInstances()[i];
			if (ImGui::Selectable(inst.getName().c_str(), i == _node)) {
				_node = i;
				_handlingNodes = true;
			}
		}
		ImGui::EndListBox();
	}

	if (ImGui::BeginListBox("Lights")) {

		for (int i = 0; i < scene->getLights().size(); i++) {
			auto light = scene->getLights()[i];
			if (ImGui::Selectable(light->getName().c_str(), i == _light)) {
				_light = i;
				_handlingNodes = false;
			}
		}
		ImGui::EndListBox();
	}

	auto light = scene->getLights()[_light];
	auto lightType = light->getType();
	ImGui::ColorEdit3("Light Color", glm::value_ptr(light->getColorRef()));
	ImGui::SliderFloat("Intensity", &light->getIntensityRef(), 0.0f, lightType == DIRECTIONAL ? 1.0f : 100.0f);
	ImGui::Text((std::string("Light Type: ") + std::to_string(light->getType())).c_str());
	if (lightType == LightType::DIRECTIONAL) {
		std::string x = std::to_string(((Astra::DirectionalLight*)light)->getDirection().x);
		std::string y = std::to_string(((Astra::DirectionalLight*)light)->getDirection().y);
		std::string z = std::to_string(((Astra::DirectionalLight*)light)->getDirection().z);

		ImGui::Text((x + ", " + y + ", " + z).c_str());
	}


	if (ImGui::Checkbox("Visible", &scene->getInstances()[_node].getVisibleRef())) {
		scene->updateTopLevelAS(_node);
	}

	if (ImGui::Button("New instance")) {
		dapp->addInstanceToScene(MeshInstance(scene->getInstances()[_node].getMeshIndex(), scene->getInstances()[_node].getTransform(), scene->getInstances()[_node].getName() + " copy" + std::to_string(_ncopies++)));
	}

	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	glm::mat4      proj = scene->getCamera()->getProjectionMatrix();
	proj[1][1] *= -1;

	if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
		_showGuizmo = !_showGuizmo;
	}

	if (_showGuizmo)
		if (_handlingNodes) {
			if (ImGuizmo::Manipulate(glm::value_ptr(scene->getCamera()->getViewMatrix()),
				glm::value_ptr(proj),
				ImGuizmo::OPERATION::UNIVERSAL,
				ImGuizmo::LOCAL,
				glm::value_ptr(app->getCurrentScene()->getInstances()[_node].getTransformRef()))) {
				scene->updateTopLevelAS(_node);
			}
		}
		else {
			ImGuizmo::Manipulate(glm::value_ptr(scene->getCamera()->getViewMatrix()),
				glm::value_ptr(proj),
				ImGuizmo::OPERATION::UNIVERSAL,
				ImGuizmo::LOCAL,
				glm::value_ptr(app->getCurrentScene()->getLights()[_light]->getTransformRef()));
		}
	ImGui::End();


}
