#pragma once


#include "st_pipeline.h"
#include "st_device.h"
#include "st_game_object.h"
#include "st_camera.h"
#include "st_frame_info.h"
#include <memory>


namespace st {
	class SimpleRenderSystem {
	public:


		SimpleRenderSystem(StDevice& device,VkRenderPass renderPass,VkDescriptorSetLayout graphicSetLayout);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem &operator=(const SimpleRenderSystem &)=delete;

		void renderGameObjects(FrameInfo & frameInfo,std::vector<StGameObject>& gameObjects);
		void computeHistogram(VkCommandBuffer& commandBuffer, VkDescriptorSet* descriptorSet);
	private:

		void createGraphicPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createComputePipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass rendrPass);
		



		StDevice& stDevice;

		std::unique_ptr<StPipeline> stPipeline;
		VkPipelineLayout graphicPipelineLayout;
		VkPipelineLayout computePipelineLayout;

	};
}  