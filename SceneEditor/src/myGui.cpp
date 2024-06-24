#include <myGui.h>
#include <myApp.h>
#include <glm/gtc/type_ptr.hpp>
#include <Utils.h>

using namespace Astra;
void BasiGui::draw(App* app)
{
	DefaultApp* dapp = static_cast<DefaultApp*>(app);
	DefaultSceneRT* scene = (DefaultSceneRT*)dapp->getCurrentScene();
	ImGuiIO& io = ImGui::GetIO();

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
			ImGui::Text("Resolution: %d x %d", (int)io.DisplaySize.x, (int)io.DisplaySize.y);
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

	if (!scene->getInstances().empty())
		if (ImGui::Checkbox("Visible", &scene->getInstances()[_node].getVisibleRef())) {
			scene->updateTopLevelAS(_node);
		}

	if (ImGui::Button("New instance")) {
		dapp->addInstanceToScene(MeshInstance(scene->getInstances()[_node].getMeshIndex(), scene->getInstances()[_node].getTransform(), scene->getInstances()[_node].getName() + " copy" + std::to_string(_ncopies++)));
	}
	if (ImGui::Button("Remove instance")) {
		if (scene->getInstances().size() == 1) {
			// last element
			// we cannot have an empty raytracing scene since the AS have to have something. They cant be empty
			// we could trick it by using a 0 mask on collisisions but the memory would still be there
			// since having no instantes is something that should not be common we are not going to allow it
			ImGui::OpenPopup("Empty RT Scene Warning");
		}

		else {
			dapp->removeInstance(_node);
			_node = 0;
		}
	}

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Empty RT Scene Warning", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("There can't be empty scenes when using a raytracing app!");
		ImGui::Separator();
		ImGuiStyle& style = ImGui::GetStyle();
		auto label = "OK";
		float size = ImGui::CalcTextSize(label).x + 120 + style.FramePadding.x * 2.0f;
		float avail = ImGui::GetContentRegionAvail().x;

		float off = (avail - size) * 0.5;
		if (off > 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
		if (ImGui::Button(label, { 120, 0 })) { ImGui::CloseCurrentPopup(); }
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}

	ImGui::ShowDemoWindow();


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
