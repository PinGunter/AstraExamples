#pragma once
#include <Scene.h>

class DefaultSceneRT : public Astra::Scene {
protected:
	nvvk::RaytracingBuilderKHR _rtBuilder;
	std::vector<VkAccelerationStructureInstanceKHR> _asInstances;


public:
	void init(nvvk::ResourceAllocator* alloc) override;
	void update() override;
	void createBottomLevelAS();
	void createTopLevelAS();
	void updateTopLevelAS(int instance_id);
	VkAccelerationStructureKHR getTLAS() const;
	void destroy() override;
	bool isRt() const override {
		return true;
	};
};