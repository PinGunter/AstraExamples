#include <gui.h>
#include <glm/gtc/type_ptr.hpp>
#include <ptscene.h>
void PtGui::draw(Astra::App* app)
{
	ImGui::Begin("Sky Color");

	ImGui::ColorEdit3("Clear Color", glm::value_ptr(app->getRenderer()->getClearColorRef()));
	ImGui::ColorEdit3("Clear Color2", ((PtScene*)(app->getCurrentScene()))->getClearColor2());
	ImGui::End();
}
