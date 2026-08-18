#include "st_swap_chain.h"
// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include "st_settings_controller.h"

namespace st {
    StSwapChain::StSwapChain(StDevice &deviceRef, VkExtent2D extent)
        : device{deviceRef}, windowExtent{extent} {
        init();
    }


    void StSwapChain::init() {

        createSwapChainHeadless();

        createImageViews();
        createRenderPass();
        createDepthResources();
        createFramebuffers();
        createSyncObjects();
    }

    StSwapChain::~StSwapChain() {

        if (swapChain != nullptr) {
            vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
            swapChain = nullptr;
        }
        for (int i = 0; i < depthImages.size(); i++) {
            vkDestroyImage(device.device(), depthImages[i], nullptr);
            vkFreeMemory(device.device(), depthImageMemorys[i], nullptr);
        }
        for (int i = 0; i < depthImageViews.size(); i++)
        {
            vkDestroyImageView(device.device(), depthImageViews[i], nullptr);
        }
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
        }
        vkDestroyRenderPass(device.device(), renderPass, nullptr);
        // cleanup synchronization objects
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyFence(device.device(), inFlightFences[i], nullptr);
        }
        for (auto& view : binSwapChainImageViews) {
            vkDestroyImageView(device.device(),view,nullptr);
        }
        binSwapChainImageViews.clear();
        for (auto& image : binSwapChainImages) {
            vkDestroyImage(device.device(),image,nullptr);
        }
        binSwapChainImages.clear();
        for (auto& mem : binSwapChainImageMemory) {
            vkFreeMemory(device.device(),mem,nullptr);
        }
        binSwapChainImageMemory.clear();


    }
    VkResult StSwapChain::acquireNextImage(uint32_t *imageIndex) {
        vkWaitForFences(
            device.device(),
            1,
            &inFlightFences[currentFrame],
            VK_TRUE,
            std::numeric_limits<uint64_t>::max());

        *imageIndex = currentFrame;
        return VK_SUCCESS;

    }

    void StSwapChain::waitForFence(uint32_t frameIndex)
    {
        vkWaitForFences(
            device.device(),
            1,
            &inFlightFences[frameIndex],
            VK_TRUE,
            std::numeric_limits<uint64_t>::max());
    }

    VkResult StSwapChain::submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {


        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = buffers;
        vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);
        if (vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
            throw std::runtime_error("failed to submit draw command buffer!");
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        return VK_SUCCESS;



    }


    void StSwapChain::createSwapChainHeadless() {
        uint32_t imageCount = MAX_FRAMES_IN_FLIGHT;


        swapChainExtent = windowExtent;                  // cubemapResolution x cubemapResolution


        // bin images: R32_UINT storage attachments consumed by histogram.comp
        binSwapChainImages.resize(imageCount);
        binSwapChainImageMemory.resize(imageCount);
        for (uint32_t i = 0; i < imageCount; i++) {
            VkImageCreateInfo image{};
            image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image.imageType = VK_IMAGE_TYPE_2D;
            image.format = VK_FORMAT_R32_UINT;
            image.extent.width = swapChainExtent.width;
            image.extent.height = swapChainExtent.height;
            image.extent.depth = 1;
            image.mipLevels = 1;
            image.arrayLayers = 6;
            image.samples = VK_SAMPLE_COUNT_1_BIT;
            image.tiling = VK_IMAGE_TILING_OPTIMAL;
            image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                         VK_IMAGE_USAGE_STORAGE_BIT |
                         VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            image.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

            VkMemoryAllocateInfo memAlloc{};
            VkMemoryRequirements memReqs;
            vkCreateImage(device.device(), &image, nullptr, &binSwapChainImages[i]);
            vkGetImageMemoryRequirements(device.device(), binSwapChainImages[i], &memReqs);
            memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memAlloc.allocationSize = memReqs.size;
            memAlloc.memoryTypeIndex =
                device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            vkAllocateMemory(device.device(), &memAlloc, nullptr, &binSwapChainImageMemory[i]);
            vkBindImageMemory(device.device(), binSwapChainImages[i], binSwapChainImageMemory[i], 0);
        }


    }

    void StSwapChain::createImageViews() {

        binSwapChainImageViews.resize(MAX_FRAMES_IN_FLIGHT*FACE_COUNT);
        binBindDescriptorInfo.resize(MAX_FRAMES_IN_FLIGHT*FACE_COUNT);


        for (size_t frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++) {
            for (size_t face = 0; face < FACE_COUNT; face++)
            {
                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = binSwapChainImages[frame];
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = VK_FORMAT_R32_UINT;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = face;
                viewInfo.subresourceRange.layerCount = 1;

                if(vkCreateImageView(device.device(),&viewInfo,nullptr,&binSwapChainImageViews[frame*FACE_COUNT+face])!=
                    VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture image view!");
                    }


                VkDescriptorImageInfo imageInfo{};
                imageInfo.sampler = binSampler;
                imageInfo.imageView = binSwapChainImageViews[frame*FACE_COUNT+face];
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                binBindDescriptorInfo[frame*FACE_COUNT+face]=imageInfo;

            }

        }
    }
    void StSwapChain::createRenderPass() {
        

        VkAttachmentDescription binAttachment{};
        binAttachment.format = VK_FORMAT_R32_UINT;
        binAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        binAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        binAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        binAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        binAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        binAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        binAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference binAttachmentRef = {};
        binAttachmentRef.attachment = 0;
        binAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        std::array<VkAttachmentReference,1> colorAttachmentRefs{binAttachmentRef};
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = colorAttachmentRefs.size();
        subpass.pColorAttachments = colorAttachmentRefs.data();
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        VkSubpassDependency dependency = {};
        dependency.dstSubpass = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        std::array<VkAttachmentDescription, 2> attachments = {binAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
        if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }
    void StSwapChain::createFramebuffers() {
        swapChainFramebuffers.resize(MAX_FRAMES_IN_FLIGHT * FACE_COUNT);
        for (size_t frame = 0; frame < imageCount(); frame++) {
            for (size_t face = 0 ; face < FACE_COUNT; face++)
            {
                std::array<VkImageView, 2> attachments = {binSwapChainImageViews[frame*FACE_COUNT+face], depthImageViews[frame*FACE_COUNT+face]};
                VkExtent2D swapChainExtent = getSwapChainExtent();
                VkFramebufferCreateInfo framebufferInfo = {};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = swapChainExtent.width;
                framebufferInfo.height = swapChainExtent.height;
                framebufferInfo.layers = 1;
                if (vkCreateFramebuffer(
                    device.device(),
                    &framebufferInfo,
                    nullptr,
                    &swapChainFramebuffers[frame*FACE_COUNT+face]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create framebuffer!");
                    }
            }

        }
        
    }
    void StSwapChain::createDepthResources() {
        VkFormat depthFormat = findDepthFormat();
        swapChainDepthFormat = depthFormat;
        VkExtent2D swapChainExtent = getSwapChainExtent();
        depthImages.resize(MAX_FRAMES_IN_FLIGHT);
        depthImageMemorys.resize(MAX_FRAMES_IN_FLIGHT);
        depthImageViews.resize(MAX_FRAMES_IN_FLIGHT*FACE_COUNT);
        for (int i = 0; i < depthImages.size(); i++) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.arrayLayers = FACE_COUNT;
            imageInfo.extent.width = swapChainExtent.width;
            imageInfo.extent.height = swapChainExtent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.flags = 0;
            device.createImageWithInfo(
                imageInfo,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                depthImages[i],
                depthImageMemorys[i]);
            for (size_t face = 0; face < FACE_COUNT; face++)
            {
                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = depthImages[i];
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = depthFormat;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = face;
                viewInfo.subresourceRange.layerCount = 1;
                if (vkCreateImageView(device.device(), &viewInfo, nullptr, &depthImageViews[i*FACE_COUNT+face]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create texture image view!");
                }
            }

        }
    }
    void StSwapChain::createSyncObjects() {

        inFlightFences.resize(imageCount());


        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < imageCount(); i++) {
            if (vkCreateFence(device.device(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    VkFormat StSwapChain::findDepthFormat() {
        return device.findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }
}  // namespace lve