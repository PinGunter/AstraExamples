#include <WireframePL.h>
#include <nvpsystem.hpp>
#include <nvvk/pipeline_vk.hpp>
#include <nvh/fileoperations.hpp>
#include <host_device.h>


void WireframePipeline::createPipeline(VkDevice vkdev, const std::vector<VkDescriptorSetLayout>& descsetsLayouts, VkRenderPass rp)
{
	std::vector<std::string> defaultSearchPaths = {
	NVPSystem::exePath() + PROJECT_RELDIRECTORY,
	NVPSystem::exePath() + PROJECT_RELDIRECTORY "..",
	std::string(PROJECT_NAME),
	};

	VkPushConstantRange pushConstantRanges = { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantRaster) };

	// Creating the Pipeline Layout
	VkPipelineLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	createInfo.setLayoutCount = static_cast<uint32_t>(descsetsLayouts.size());
	createInfo.pSetLayouts = descsetsLayouts.data();
	createInfo.pushConstantRangeCount = 1;
	createInfo.pPushConstantRanges = &pushConstantRanges;
	vkCreatePipelineLayout(vkdev, &createInfo, nullptr, &_layout);


	// Pipeline: completely generic, no vertices
	nvvk::GraphicsPipelineGeneratorCombined pipelineGenerator(vkdev, _layout, rp);
	pipelineGenerator.addShader(nvh::loadFile("spv/AstraCore/wireframe.vert.spv", true, defaultSearchPaths, true), VK_SHADER_STAGE_VERTEX_BIT);
	pipelineGenerator.addShader(nvh::loadFile("spv/AstraCore/wireframe.frag.spv", true, defaultSearchPaths, true), VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineGenerator.rasterizationState.cullMode = VK_CULL_MODE_NONE;
	pipelineGenerator.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
	pipelineGenerator.addBindingDescription({ 0, sizeof(Vertex) });
	pipelineGenerator.addAttributeDescriptions({
		{0, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(Vertex, pos))},
		{1, 0, VK_FORMAT_R32G32B32_SFLOAT, static_cast<uint32_t>(offsetof(Vertex, color))},
		});
	_pipeline = pipelineGenerator.createPipeline();
}
