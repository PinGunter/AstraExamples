#include <ptPipeline.h>
#include <nvh/fileoperations.hpp>
#include <host_device.h>
#include <Device.h>
#include <nvh/alignment.hpp>

void PtPipeline::createSBT(nvvk::ResourceAllocatorDma& alloc, const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& rtProperties)
{
	uint32_t missCount{ 1 };
	uint32_t hitCount{ 1 };
	auto handleCount = 1 + missCount + hitCount;
	uint32_t handleSize = rtProperties.shaderGroupHandleSize;

	// the sbt buffer needs to have starting groups to be aligned and handles in the group to be aligned
	uint32_t handleSizeAligned = nvh::align_up(handleSize, rtProperties.shaderGroupHandleAlignment);

	_rgenRegion.stride = nvh::align_up(handleSizeAligned, rtProperties.shaderGroupBaseAlignment);
	_rgenRegion.size = _rgenRegion.stride; // raygen size and stride have to be the same
	_missRegion.stride = handleSizeAligned;
	_missRegion.size = nvh::align_up(missCount * handleSizeAligned, rtProperties.shaderGroupBaseAlignment);
	_hitRegion.stride = handleSizeAligned;
	_hitRegion.size = nvh::align_up(hitCount * handleSizeAligned, rtProperties.shaderGroupBaseAlignment);

	// get the shader group handles
	uint32_t dataSize = handleCount * handleSize;
	std::vector<uint8_t> handles(dataSize);
	auto result = vkGetRayTracingShaderGroupHandlesKHR(AstraDevice.getVkDevice(), _pipeline, 0, handleCount, dataSize, handles.data());
	assert(result == VK_SUCCESS);

	// allocate buffer for storing the sbt
	VkDeviceSize sbtSize = _rgenRegion.size + _missRegion.size + _hitRegion.size;
	_rtSBTBuffer = alloc.createBuffer(
		sbtSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// find the sbt address of each group
	VkBufferDeviceAddressInfo info{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, _rtSBTBuffer.buffer };
	VkDeviceAddress sbtAddress = vkGetBufferDeviceAddress(AstraDevice.getVkDevice(), &info);
	_rgenRegion.deviceAddress = sbtAddress;
	_missRegion.deviceAddress = sbtAddress + _rgenRegion.size;
	_hitRegion.deviceAddress = sbtAddress + _rgenRegion.size + _missRegion.size;

	// helper to retrieve the handle data
	auto getHandle = [&](int i)
		{ return handles.data() + i * handleSize; };

	auto* pSBTBuffer = reinterpret_cast<uint8_t*>(alloc.map(_rtSBTBuffer));
	uint8_t* pData{ nullptr };
	uint32_t handleIdx{ 0 };

	// raygen. Just copy handle
	pData = pSBTBuffer;
	memcpy(pData, getHandle(handleIdx++), handleSize);

	// miss
	pData = pSBTBuffer + _rgenRegion.size;
	for (uint32_t c = 0; c < missCount; c++)
	{
		memcpy(pData, getHandle(handleIdx++), handleSize);
		pData += _missRegion.stride;
	}

	// hit
	pData = pSBTBuffer + _rgenRegion.size + _missRegion.size;
	for (uint32_t c = 0; c < hitCount; c++)
	{
		memcpy(pData, getHandle(handleIdx++), handleSize);
		pData += _hitRegion.stride;
	}

	alloc.unmap(_rtSBTBuffer);
	alloc.finalizeAndReleaseStaging();
}

void PtPipeline::create(VkDevice vkdev, const std::vector<VkDescriptorSetLayout>& descsets, nvvk::ResourceAllocatorDma& alloc)
{
	auto rtProperties = AstraDevice.getRtProperties();
	if (!AstraDevice.getRtEnabled())
	{
		throw std::runtime_error("Can't create raytracing pipeline without enabling raytracing!");
	}

	if (rtProperties.maxRayRecursionDepth <= 1)
	{
		throw std::runtime_error("Device does not support ray recursion");
	}

	enum StageIndices
	{
		eRaygen,
		eMiss,
		eClosestHit,
		eShaderGroupCount
	};

	// all stages
	std::array<VkPipelineShaderStageCreateInfo, eShaderGroupCount> stages{};
	VkPipelineShaderStageCreateInfo stage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	stage.pName = "main";

	// raygen
	stage.module = AstraDevice.createShaderModule(nvh::loadFile("spv/Pathtracer/pt.rgen.spv", true, Astra::defaultSearchPaths, true));
	stage.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	stages[eRaygen] = stage;

	// miss
	stage.module = AstraDevice.createShaderModule(nvh::loadFile("spv/Pathtracer/pt.rmiss.spv", true, Astra::defaultSearchPaths, true));
	stage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	stages[eMiss] = stage;

	// chit
	stage.module = AstraDevice.createShaderModule(nvh::loadFile("spv/Pathtracer/pt.rchit.spv", true, Astra::defaultSearchPaths, true));
	stage.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	stages[eClosestHit] = stage;

	// shader groups
	VkRayTracingShaderGroupCreateInfoKHR group{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
	group.anyHitShader = VK_SHADER_UNUSED_KHR;
	group.closestHitShader = VK_SHADER_UNUSED_KHR;
	group.generalShader = VK_SHADER_UNUSED_KHR;
	group.intersectionShader = VK_SHADER_UNUSED_KHR;

	// raygen
	group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	group.generalShader = eRaygen;
	_rtShaderGroups.push_back(group);

	// miss
	group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	group.generalShader = eMiss;
	_rtShaderGroups.push_back(group);

	// closest hit shader
	group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	group.generalShader = VK_SHADER_UNUSED_KHR;
	group.closestHitShader = eClosestHit;
	_rtShaderGroups.push_back(group);

	// push constants
	VkPushConstantRange pushConstant{ VK_SHADER_STAGE_RAYGEN_BIT_KHR |
										 VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_NV,
									 0, sizeof(PushConstantRay) };

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstant;

	// descriptor sets: one specific to rt (set=0, tlas) , other shared with raster (set=1, scene data)
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descsets.size());
	pipelineLayoutCreateInfo.pSetLayouts = descsets.data();

	if ((vkCreatePipelineLayout(vkdev, &pipelineLayoutCreateInfo, nullptr, &_layout) != VK_SUCCESS))
	{
		throw std::runtime_error("Error creating pipeline layout");
	};

	// assemble the stage shaders and recursion depth
	VkRayTracingPipelineCreateInfoKHR rayPipelineInfo{ VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR };
	rayPipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
	rayPipelineInfo.pStages = stages.data();

	// we indicate the shader groups
	// miss and raygen are their own group each
	// intersection, anyhit and chit form a hit group
	rayPipelineInfo.groupCount = static_cast<uint32_t>(_rtShaderGroups.size());
	rayPipelineInfo.pGroups = _rtShaderGroups.data();

	// recursion depth
	rayPipelineInfo.maxPipelineRayRecursionDepth = rtProperties.maxRayRecursionDepth; // rtProperties.maxRayRecursionDepth; // shadow
	rayPipelineInfo.layout = _layout;

	if ((vkCreateRayTracingPipelinesKHR(vkdev, {}, {}, 1, &rayPipelineInfo, nullptr, &_pipeline) != VK_SUCCESS))
	{
		throw std::runtime_error("Error creating pipelines");
	}

	for (auto& s : stages)
	{
		vkDestroyShaderModule(vkdev, s.module, nullptr);
	}

	// we now create the Shader Binding Table (SBT)
	createSBT(alloc, rtProperties);
}
