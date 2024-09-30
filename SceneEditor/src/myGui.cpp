#include <myGui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <basicShapes.h>
#include <glm/gtc/type_ptr.hpp>


using namespace Astra;
void BasicGui::startDockableWindow()
{
	// Keeping the unique ID of the dock space
	auto dockspaceID = ImGui::GetID("DockSpace");

	// The dock need a dummy window covering the entire viewport.
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	// All flags to dummy window
	ImGuiWindowFlags host_window_flags = 0;
	host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	host_window_flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
	host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	host_window_flags |= ImGuiWindowFlags_NoBackground;

	// Starting dummy window
	char label[32];
	ImFormatString(label, IM_ARRAYSIZE(label), "DockSpaceViewport_%08X", viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin(label, nullptr, host_window_flags);
	ImGui::PopStyleVar(3);

	// The central node is transparent, so that when UI is draw after, the image is visible
	// Auto Hide Bar, no title of the panel
	// Center is not dockable, that is for the scene
	ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar
		| ImGuiDockNodeFlags_NoDockingOverCentralNode;

	// Building the splitting of the dock space is done only once
	if (!ImGui::DockBuilderGetNode(dockspaceID))
	{
		ImGui::DockBuilderRemoveNode(dockspaceID);
		ImGui::DockBuilderAddNode(dockspaceID, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

		ImGuiID dock_main_id = dockspaceID;

		// Slitting all 4 directions, targetting (320 pixel * DPI) panel width, (180 pixel * DPI) panel height.
		const float xRatio = glm::clamp<float>(180.0f / viewport->WorkSize[0], 0.01f, 0.499f);
		const float yRatio = glm::clamp<float>(180.0f / viewport->WorkSize[1], 0.01f, 0.499f);
		ImGuiID     id_left, id_right, id_up, id_down;

		// Note, for right, down panels, we use the n / (1 - n) formula to correctly split the space remaining from the left, up panels.
		id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, xRatio, nullptr, &dock_main_id);
		id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, xRatio / (1 - xRatio), nullptr, &dock_main_id);
		id_up = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, yRatio, nullptr, &dock_main_id);
		id_down = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, yRatio / (1 - yRatio), nullptr, &dock_main_id);

		ImGui::DockBuilderDockWindow("Dock_left", id_left);
		ImGui::DockBuilderDockWindow("Dock_right", id_right);
		ImGui::DockBuilderDockWindow("Dock_up", id_up);
		ImGui::DockBuilderDockWindow("Dock_down", id_down);
		ImGui::DockBuilderDockWindow("Scene", dock_main_id);  // Center

		ImGui::DockBuilderFinish(dock_main_id);
	}

	// Setting the panel to blend with alpha
	ImVec4 col = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(col.x, col.y, col.z, 0.8f));

	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
	ImGui::PopStyleColor();
	ImGui::End();

	// The panel
	//if (alpha < 1)
	ImGui::SetNextWindowBgAlpha(0.8f);  // For when the panel becomes a floating window

}

void BasicGui::CubeCreator(DefaultApp * app)
{
	ImGui::Begin("Shapes");

	//static float dim[3] = { 10.0f };
	//ImGui::SliderFloat3("Dimensions", dim, 0.0f, 10.0f);
	if (ImGui::Button("Add Cube")) {
		auto geo = BasicShapes::boxGeometry();
		WaveFrontMaterial mat{};
		mat.diffuse = glm::vec3(1, 0, 0);
		mat.illum = 2;
		mat.textureId = -1;
		Astra::Mesh mesh;
		mesh.fromGeoMat(geo, mat);
		app->addShape(mesh);
		//Astra::MeshInstance instance(mesh.meshId);
		//app->addInstanceToScene(instance);
	}

	ImGui::End();
}

void BasicGui::ModelAndInstances(DefaultApp* dapp) {
	Astra::SceneRT* scene = ((SceneRT*)dapp->getCurrentScene());
	static int selModel = 0;
	if (ImGui::BeginListBox("Models")) {

		for (int i = 0; i < scene->getModels().size(); i++) {
			auto& model = scene->getModels()[i];
			std::string label = "Model " + std::to_string(i);
			if (ImGui::Selectable(label.c_str(), i == selModel)) {
				selModel = i;
				_handlingNodes = true;
			}
		}
		ImGui::EndListBox();
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
	if (!scene->getInstances().empty())
		if (ImGui::Checkbox("Visible", &scene->getInstances()[_node].getVisibleRef())) {
			scene->updateTopLevelAS(_node);
		}

	if (ImGui::Button("New instance")) {
		dapp->addInstanceToScene(MeshInstance(scene->getModels()[selModel].meshId));
	}
	ImGui::SameLine();
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
}

void BasicGui::draw(App* app)
{
	DefaultApp* dapp = static_cast<DefaultApp*>(app);
	SceneRT* scene = (SceneRT*)dapp->getCurrentScene();
	ImGuiIO& io = ImGui::GetIO();

	startDockableWindow();

	CubeCreator(dapp);

	ImGui::Begin("Renderer");


	ImGui::ColorEdit3("Clear Color", glm::value_ptr(app->getRenderer()->getClearColorRef()));

	ImGui::SliderInt("Max Ray bounces", &app->getRenderer()->getMaxDepthRef(), 1, 30);
	ImGui::Checkbox("Raytraced shadows", &app->getRenderer()->getUseShadowsRef());

	ImGui::Separator();

	ImGui::Text("Select rendering pipeline");
	ImGui::RadioButton("RayTracing", &dapp->getSelectedPipelineRef(), 0);
	ImGui::RadioButton("Raster", &dapp->getSelectedPipelineRef(), 1);
	ImGui::RadioButton("Wireframe", &dapp->getSelectedPipelineRef(), 2);
	ImGui::RadioButton("Normals", &dapp->getSelectedPipelineRef(), 3);
	ImGui::RadioButton("Greyscale", &dapp->getSelectedPipelineRef(), 4);



	ImGui::End();

	ImGui::Begin("Performance");
	ImGui::Text("Resolution: %d x %d", (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	ImGui::Text("Frame time %.3f ms/frame", dapp->getFrameTime() * 1000);
	ImGui::Text((std::string("Recording stats: ") + (dapp->getRecordingStats() ? "Yes" : "No")).c_str());

	if (ImGui::Button(
		(std::string((dapp->getRecordingStats()) ? "Stop" : "Start") + " tracking stats").c_str()
	)) {
		dapp->setRecordingStats(!dapp->getRecordingStats());
	}
	if (ImGui::Button("Save stats")) {
		dapp->saveStats();
	}


	ImGui::End();


	ImGui::Begin("Scene");

	ImGui::Text((std::string("Current Scene ") + std::to_string(app->getCurrentSceneIndex())).c_str());
	ImGui::SameLine();
	if (ImGui::Button("Switch")) {
		app->setCurrentSceneIndex((app->getCurrentSceneIndex() + 1) % app->getScenesCount());
		_node = 0;
		_light = 0;
	}

	ModelAndInstances(dapp);

	ImGui::Separator();

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
	if (!scene->getLights().empty()) {

		auto light = scene->getLights()[_light];
		auto lightType = light->getType();
		ImGui::ColorEdit3("Light Color", glm::value_ptr(light->getColorRef()));
		ImGui::InputFloat("Intensity", &light->getIntensityRef());
		ImGui::Text((std::string("Light Type: ") + std::to_string(light->getType())).c_str());

		if (lightType == DIRECTIONAL) {
			if (ImGui::Button("Rotate -Z")) {
				((DirectionalLight*)light)->rotate(glm::vec3(0, 0, 1), glm::radians(-5.0f));
			}
			ImGui::SameLine();
			if (ImGui::Button("Rotate +Z")) {
				((DirectionalLight*)light)->rotate(glm::vec3(0, 0, 1), glm::radians(5.0f));
			}
		}
	}

	if (ImGui::Button("Add Point Light")) {
		auto newLight = new PointLight(glm::vec3(1.0f), 60);
		scene->addLight(newLight);
	}
	ImGui::SameLine();

	if (ImGui::Button("Add Directional Light")) {
		auto newLight = new DirectionalLight(glm::vec3(1.0f), 0.6f, glm::vec3(1));
		scene->addLight(newLight);
	}

	if (ImGui::Button("Remove Light")) {
		scene->removeLight(scene->getLights()[_light]);
		_light = 0;
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
			if (!scene->getLights().empty())
				ImGuizmo::Manipulate(glm::value_ptr(scene->getCamera()->getViewMatrix()),
					glm::value_ptr(proj),
					ImGuizmo::OPERATION::TRANSLATE,
					ImGuizmo::LOCAL,
					glm::value_ptr(app->getCurrentScene()->getLights()[_light]->getTransformRef()));
		}
	ImGui::End();


}

