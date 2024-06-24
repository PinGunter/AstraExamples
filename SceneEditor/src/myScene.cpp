#include <myScene.h>
#include <Device.h>


// rt scene

void DefaultSceneRT::init(nvvk::ResourceAllocator* alloc)
{
	Scene::init(alloc);
	//if (!_objDescBuffer.buffer) {
		//throw std::runtime_error("Empty Scene!");
	//}
	_rtBuilder.setup(AstraDevice.getVkDevice(), alloc, Astra::Device::getInstance().getGraphicsQueueIndex());
}

void DefaultSceneRT::update()
{
	for (auto l : _lights) {
		l->update();
	}
	_camera->update();
	std::vector<int> asupdates;
	for (int i = 0; i < _instances.size(); i++) {
		if (_instances[i].update()) {
			asupdates.push_back(i);
		}
	}

	for (int i : asupdates) {
		updateTopLevelAS(i);
	}

}

void DefaultSceneRT::createBottomLevelAS()
{
	if (getTLAS() != VK_NULL_HANDLE) {
		_rtBuilder.destroy();
		_asInstances.clear();
	}
	std::vector<nvvk::RaytracingBuilderKHR::BlasInput> allBlas;
	allBlas.reserve(_objModels.size());

	for (const auto& obj : _objModels) {
		auto blas = AstraDevice.objectToVkGeometry(obj);

		allBlas.emplace_back(blas);
	}

	_rtBuilder.buildBlas(allBlas, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
}

void DefaultSceneRT::createTopLevelAS()
{
	if (getTLAS() != VK_NULL_HANDLE) {
		_rtBuilder.destroy();
		_asInstances.clear();
	}
	_asInstances.reserve(_instances.size());
	for (const Astra::MeshInstance& inst : _instances) {
		VkAccelerationStructureInstanceKHR rayInst{};
		rayInst.transform = nvvk::toTransformMatrixKHR(inst.getTransform());
		rayInst.instanceCustomIndex = inst.getMeshIndex(); //gl_InstanceCustomIndexEXT
		rayInst.accelerationStructureReference = _rtBuilder.getBlasDeviceAddress(inst.getMeshIndex());
		rayInst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR;
		rayInst.mask = inst.getVisible() ? 0xFF : 0x00; // only be hit if raymask & instance.mask != 0
		rayInst.instanceShaderBindingTableRecordOffset = 0; // the same hit group for all objects

		_asInstances.emplace_back(rayInst);
	}
	_rtBuilder.buildTlas(_asInstances, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
}

void DefaultSceneRT::updateTopLevelAS(int instance_id)
{
	const auto& inst = _instances[instance_id];
	VkAccelerationStructureInstanceKHR rayInst{};
	rayInst.transform = nvvk::toTransformMatrixKHR(inst.getTransform());
	rayInst.instanceCustomIndex = inst.getMeshIndex(); //gl_InstanceCustomIndexEXT
	rayInst.accelerationStructureReference = _rtBuilder.getBlasDeviceAddress(inst.getMeshIndex());
	rayInst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR;
	rayInst.mask = inst.getVisible() ? 0xFF : 0x00; // only be hit if raymask & instance.mask != 0
	rayInst.instanceShaderBindingTableRecordOffset = 0; // the same hit group for all objects
	_asInstances[instance_id] = rayInst;

	_rtBuilder.buildTlas(_asInstances, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, true);
}

VkAccelerationStructureKHR DefaultSceneRT::getTLAS() const
{
	return _rtBuilder.getAccelerationStructure();
}

void DefaultSceneRT::destroy()
{
	Scene::destroy();
	_rtBuilder.destroy();
}