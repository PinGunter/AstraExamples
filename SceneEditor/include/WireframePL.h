#pragma once
#include <Pipeline.h>

class WireframePipeline : public Astra::RasterPipeline {
public:
	void createPipeline(VkDevice vkdev, const std::vector<VkDescriptorSetLayout>& descsetsLayouts, VkRenderPass rp) override;
};