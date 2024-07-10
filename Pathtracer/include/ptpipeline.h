#pragma once
#include <Pipeline.h>
class PathtracingPipeline : public Astra::RayTracingPipeline {
protected:
	void createSBT(nvvk::ResourceAllocatorDma& alloc, const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& rtProperties) override;

public:
	void create(VkDevice vkdev, const std::vector<VkDescriptorSetLayout>& descsetsLayouts, nvvk::ResourceAllocatorDma& alloc) override;
};