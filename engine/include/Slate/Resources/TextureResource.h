//
// Created by Hayden Rivas on 4/22/25.
//

#pragma once
#include "Slate/Common/Handles.h"
#include "Slate/Resources/IResource.h"

// forward declare
struct VkExtent2D;
struct VkExtent3D;
namespace Slate {

	struct TextureResource : public IResource {
	public:
		TextureHandle getHandle();
		void assignHandle(TextureHandle handle);
		VkExtent2D getDimensions() const;
		const void* getData() const;
	private:
		const void* _data;
		uint32_t _width;
		uint32_t _height;
		TextureHandle _handle;
	private:
		Result LoadResourceImpl(const std::filesystem::path& path) override;
	};
}