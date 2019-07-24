#include "VulkanSync.h"

#include "Vulkan.h"

VulkanFence::VulkanFence(VkDevice _Device, bool _StateInitialized) : VulkanObject(_Device)
{
	VkFenceCreateInfo FenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	FenceCreateInfo.flags = _StateInitialized;
	
	GS_VK_CHECK(vkCreateFence(m_Device, &FenceCreateInfo, ALLOCATOR, &Fence), "Failed to create Fence!")
}

VulkanFence::~VulkanFence()
{
	vkDestroyFence(m_Device, Fence, ALLOCATOR);
}

VulkanSemaphore::VulkanSemaphore(VkDevice _Device) : VulkanObject(_Device)
{
	VkSemaphoreCreateInfo CreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	GS_VK_CHECK(vkCreateSemaphore(m_Device, &CreateInfo, ALLOCATOR, &Semaphore), "Failed to create Semaphore!")
}

VulkanSemaphore::~VulkanSemaphore()
{
	vkDestroySemaphore(m_Device, Semaphore, ALLOCATOR);
}