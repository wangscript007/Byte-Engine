#pragma once

#include "Core.h"

#include "RAPI/UniformBuffer.h"

#include "RAPI/Vulkan/Vulkan.h"

class VulkanUniformBuffer : public RAPI::UniformBuffer
{
	VkBuffer buffer = nullptr;
	VkDeviceMemory memory = nullptr;

	byte* mappedMemoryPointer = nullptr;
public:
	VulkanUniformBuffer(class VulkanRenderDevice* _Device, const RAPI::UniformBufferCreateInfo& _BCI);
	~VulkanUniformBuffer();

	void Destroy(class RenderDevice* renderDevice) override;

	void UpdateBuffer(const RAPI::UniformBufferUpdateInfo& uniformBufferUpdateInfo) const override;

	VkBuffer GetVkBuffer() const { return buffer; }
};