//
// Created by Hayden Rivas on 1/16/25.
//


#include <volk.h>
#include <fmt/core.h>
#include <iostream>

#include "Slate/Filesystem.h"
#include "Slate/Shader.h"

// shader headers
#include <nlohmann/json.hpp>


#include "Slate/VK/vkinfo.h"
#include "Slate/VK/vkutil.h"


namespace Slate::vkutil {
	VkImageAspectFlags AspectMaskFromLayout(VkImageLayout layout) {
		switch (layout) {
			case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_DEPTH_BIT;
			case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_STENCIL_BIT;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

			default: return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}
	VkImageAspectFlags AspectMaskFromFormat(VkFormat format) {
		switch (format) {
			case VK_FORMAT_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;
			case VK_FORMAT_D32_SFLOAT_S8_UINT: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

			default: return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}

	void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout) {
		VkImageMemoryBarrier2 barrier = { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, .pNext = nullptr };
		barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
		barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		barrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;
		barrier.oldLayout = currentLayout;
		barrier.newLayout = newLayout;
		barrier.subresourceRange = vkinfo::CreateImageSubresourceRange(AspectMaskFromLayout(newLayout));
		barrier.image = image;
		VkDependencyInfo info = { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
		info.imageMemoryBarrierCount = 1;
		info.pImageMemoryBarriers = &barrier;
		vkCmdPipelineBarrier2KHR(cmd, &info);
	}
	std::vector<UBO> glslReflection(const std::string& filename);

	ShaderProgram CreateShaderProgram(VkDevice device, const std::filesystem::path& path) {
		ShaderProgram program;
		// read file
		const std::vector<uint32_t> result = Filesystem::ReadSpvFile("shaders/compiled_shaders" / path);
		// attach vkshadermodules
		auto temp = CreateShaderModule(device, result);
		program.vertModule = temp;
		program.fragModule = temp;

		// get filename
		program.filename = path.filename().stem();

		program.uniformBlockObjects = glslReflection(program.filename + ".json");

		return program;
	}

//	std::vector<Resource> slangReflection(const std::vector<uint32_t>& result) {
//
//		slang::IGlobalSession* globalSession;
//		slang::createGlobalSession(&globalSession);
//
//		std::vector<slang::TargetDesc> targetDescs;
//		for (auto target : kTargets)
//		{
//			auto profile = globalSession->findProfile(target.profile);
//
//			slang::TargetDesc targetDesc;
//			targetDesc.format = target.format;
//			targetDesc.profile = profile;
//			targetDescs.add(targetDesc);
//		}
//
//		slang::SessionDesc sessionDesc;
//		sessionDesc.targetCount = (SlangInt) targetDescs.size();
//		sessionDesc.targets = targetDescs.data();
//
//		slang::ISession* session = nullptr;
//		globalSession->createSession(sessionDesc, &session);
//
//		slang::IBlob* diagnostics;
//		slang::IModule* module = session->loadModule(reinterpret_cast<const char*>(result.data()), &diagnostics);
////		if (!module) return SLANG_FAIL;
//
//	}

	std::string GetGLSLTypeString(const spirv_cross::SPIRType& type) {
		if (type.columns > 1) { // Matrices
			return "mat" + std::to_string(type.columns) + "x" + std::to_string(type.vecsize);
		}

		if (type.vecsize > 1) { // Vectors
			return "vec" + std::to_string(type.vecsize);
		}

		switch (type.basetype) { // Scalars
			case spirv_cross::SPIRType::BaseType::Float:   return "float";
			case spirv_cross::SPIRType::BaseType::Int:     return "int";
			case spirv_cross::SPIRType::BaseType::Boolean: return "bool";
			case spirv_cross::SPIRType::BaseType::Double:  return "double";
			case spirv_cross::SPIRType::BaseType::Struct:  return "struct";
			case spirv_cross::SPIRType::BaseType::Image:   return "image";
			case spirv_cross::SPIRType::BaseType::Sampler: return "sampler";
			default: return "unknown";
		}
	}
	std::string GetGLSLType(const nlohmann::json& type) {
		std::string kind = type.value("kind", "");

		if (kind == "scalar") {
			return type.value("scalarType", "unknown");
		}
		if (kind == "vector") {
			return "vec" + std::to_string(type.value("elementCount", 0)) + "<" +
				   type["elementType"].value("scalarType", "unknown") + ">";
		}
		if (kind == "matrix") {
			return "mat" + std::to_string(type.value("rowCount", 0)) + "x" +
				   std::to_string(type.value("columnCount", 0)) + "<" +
				   type["elementType"].value("scalarType", "unknown") + ">";
		}
		if (kind == "struct") {
			return type.value("name", "struct");
		}
		if (kind == "array") {
			return "array[" + std::to_string(type.value("elementCount", 0)) + "]<" +
				   GetGLSLType(type["elementType"]) + ">";
		}

		return "unknown";
	}

//	CustomType ParseUniformBlock(const nlohmann::json& json) {
//		CustomType rootBlock{};
//		rootBlock.name = json.value("name", "root");
//		rootBlock.usertype = json["type"].value("name", "");
//		rootBlock.id = 300;
//
//		for (const auto& member : json["type"]["fields"]) {
//			ParseUniformMember(member);
//		}
//
//		return rootBlock;
//	}
	std::vector<UBO> glslReflection(const std::string& filename) {
		std::filesystem::path basepath("shaders/compiled_shaders/reflection");
		std::filesystem::path path = basepath / filename;
		nlohmann::json json_data = Filesystem::ReadJsonFile(path);

		if (!json_data.contains("parameters")) return {};

		std::vector<UBO> result_vector;

		for (const auto& parameter : json_data["parameters"]) {
			// require its a buffer/struct
			if (!parameter["type"].contains("elementType")) continue;

			UBO block {};
			block.name = parameter.value("name", "unknown");
			block.usertype = parameter["type"]["elementType"].value("name", "unknown");
			// make sure its a struct or constantbuffer
			for (const auto &member: parameter["type"]["elementType"]["fields"]) {
				// if its a custom type (struct on gpu)
				if (member["type"].contains("fields")) {
					CustomType customMember {};
					customMember.name = member.value("name", "unknown");
					customMember.usertype = member["type"].value("name", "");
					customMember.id = 200;
//					for (const auto& inner_member : member["type"]["fields"]) {
//						customMember.members.push_back(ParseUniformMember(inner_member, rootBlock));
//					}
					block.members.push_back(customMember);
				} else {
					PlainType plainMember {};
					plainMember.name = member.value("name", "unknown");
					plainMember.glsltype = Util::EngineTypeFromJsonType(member["type"]);
					plainMember.id = 100;

					block.members.push_back(plainMember);
				}
			}
			// append
			result_vector.push_back(block);
		}
		return result_vector;
	}



	void DestroyShaderProgramModules(VkDevice device, const ShaderProgram& program) {
		// because we are refrencing the same module we only have to delete the first one
		vkDestroyShaderModule(device, program.vertModule, nullptr);
	}




	// TODO: make this concrete later
	// ONLY FOR EDITOR USAGE, no games should use this
	VkShaderModule CreateShaderModuleEXT(VkDevice device, const std::filesystem::path& path) {
		const std::vector<uint32_t> result = Filesystem::ReadSpvFile("shaders/compiled_shaders" / path);
		return CreateShaderModule(device, result);
	}

	static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<uint32_t>& code) {
		VkShaderModuleCreateInfo create_info = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		create_info.codeSize = code.size() * sizeof (uint32_t);
		create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &create_info, nullptr, &shaderModule) != VK_SUCCESS) {
			return VK_NULL_HANDLE;
		}
		return shaderModule;
	}


	void BlitImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize) {
		VkImageBlit2 blitRegion = { .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

		blitRegion.srcOffsets[1].x = static_cast<int32_t>(srcSize.width);
		blitRegion.srcOffsets[1].y = static_cast<int32_t>(srcSize.height);
		blitRegion.srcOffsets[1].z = 1;

		blitRegion.dstOffsets[1].x = static_cast<int32_t>(dstSize.width);
		blitRegion.dstOffsets[1].y = static_cast<int32_t>(dstSize.height);
		blitRegion.dstOffsets[1].z = 1;

		blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.srcSubresource.baseArrayLayer = 0;
		blitRegion.srcSubresource.layerCount = 1;
		blitRegion.srcSubresource.mipLevel = 0;

		blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blitRegion.dstSubresource.baseArrayLayer = 0;
		blitRegion.dstSubresource.layerCount = 1;
		blitRegion.dstSubresource.mipLevel = 0;

		VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
		blitInfo.dstImage = destination;
		blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		blitInfo.srcImage = source;
		blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		blitInfo.filter = VK_FILTER_LINEAR;
		blitInfo.regionCount = 1;
		blitInfo.pRegions = &blitRegion;

		vkCmdBlitImage2KHR(cmd, &blitInfo);
	}

	void SetViewport(VkCommandBuffer cmd, VkExtent2D extent2D) {
		// we flip the viewport because Vulkan is reversed using LH instead of GL's RH
		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = static_cast<float>(extent2D.height);
		viewport.width = static_cast<float>(extent2D.width);
		viewport.height = -static_cast<float>(extent2D.height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(cmd, 0, 1, &viewport);
	}
	void SetScissor(VkCommandBuffer cmd, VkExtent2D extent2D, VkOffset2D offset2D) {
		VkRect2D scissor = {};
		scissor.offset.x = offset2D.x;
		scissor.offset.y = offset2D.y;
		scissor.extent = extent2D;
		vkCmdSetScissor(cmd, 0, 1, &scissor);
	}

	void GenerateMipmaps(VkCommandBuffer cmd, VkImage image, VkExtent2D imageSize) {
		int mipLevels = int(std::floor(std::log2(std::max(imageSize.width, imageSize.height)))) + 1;
		for (int mip = 0; mip < mipLevels; mip++) {

			VkExtent2D halfSize = imageSize;
			halfSize.width /= 2;
			halfSize.height /= 2;

			VkImageMemoryBarrier2 imageBarrier { .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, .pNext = nullptr };

			imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
			imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
			imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

			imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

			VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBarrier.subresourceRange = vkinfo::CreateImageSubresourceRange(aspectMask);
			imageBarrier.subresourceRange.levelCount = 1;
			imageBarrier.subresourceRange.baseMipLevel = mip;
			imageBarrier.image = image;

			VkDependencyInfo depInfo { .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .pNext = nullptr };
			depInfo.imageMemoryBarrierCount = 1;
			depInfo.pImageMemoryBarriers = &imageBarrier;

			vkCmdPipelineBarrier2KHR(cmd, &depInfo);

			if (mip < mipLevels - 1) {
				VkImageBlit2 blitRegion { .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

				blitRegion.srcOffsets[1].x = imageSize.width;
				blitRegion.srcOffsets[1].y = imageSize.height;
				blitRegion.srcOffsets[1].z = 1;

				blitRegion.dstOffsets[1].x = halfSize.width;
				blitRegion.dstOffsets[1].y = halfSize.height;
				blitRegion.dstOffsets[1].z = 1;

				blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blitRegion.srcSubresource.baseArrayLayer = 0;
				blitRegion.srcSubresource.layerCount = 1;
				blitRegion.srcSubresource.mipLevel = mip;

				blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blitRegion.dstSubresource.baseArrayLayer = 0;
				blitRegion.dstSubresource.layerCount = 1;
				blitRegion.dstSubresource.mipLevel = mip + 1;

				VkBlitImageInfo2 blitInfo {.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr};
				blitInfo.dstImage = image;
				blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				blitInfo.srcImage = image;
				blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				blitInfo.filter = VK_FILTER_LINEAR;
				blitInfo.regionCount = 1;
				blitInfo.pRegions = &blitRegion;

				vkCmdBlitImage2KHR(cmd, &blitInfo);

				imageSize = halfSize;
			}
		}
		// transition all mip levels into the final read_only layout
		TransitionImageLayout(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}
