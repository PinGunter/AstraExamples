#pragma once
#include <Pipeline.h>

class WireframePipeline : public Astra::RasterPipeline {
public:
	void create(VkDevice vkdev, const std::vector<VkDescriptorSetLayout>& descsetsLayouts, VkRenderPass rp) override;
};

class NormalPipeline : public Astra::RasterPipeline {
public:
	void create(VkDevice vkdev, const std::vector<VkDescriptorSetLayout>& descsetsLayouts, VkRenderPass rp) override;
};