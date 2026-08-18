#pragma once
#include "st_camera.h"
// lib
#include <vulkan/vulkan.h>
namespace st {
	struct FrameInfo {
		uint32_t frameIndex;
		VkCommandBuffer commandBuffer;
		StCamera &camera;
		VkDescriptorSet globalDescriptorSet;
	};
} 