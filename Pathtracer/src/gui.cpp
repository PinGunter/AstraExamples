#include "gui.h"
#include <ptapp.h>

void PtGui::draw(Astra::App* app)
{
	auto myapp = (PathtracerApp*)app;
	ImGui::Text("Select pipeline");
	ImGui::RadioButton("PathTracing", &myapp->getSelectedPipelineRef(), 0);
	ImGui::RadioButton("Raster", &myapp->getSelectedPipelineRef(), 1 );
}
