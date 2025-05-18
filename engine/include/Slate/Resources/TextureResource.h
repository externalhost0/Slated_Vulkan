//
// Created by Hayden Rivas on 4/22/25.
//

#pragma once
#include "Slate/Common/Handles.h"
#include "Slate/Resources/IResource.h"

// forward declare
class VkExtent2D;
class VkExtent3D;
namespace Slate {
	enum class TextureType {
		Type_2D,
		Type_3D,
		Type_Cube
	};

	struct TextureResource : public IResource {
	public:
		TextureHandle getHandle();
		void assignHandle(TextureHandle handle);
		VkExtent2D getDimensions() const;
		const void* getData() const;
	private:
		const void* _data = nullptr;
		uint32_t _width;
		uint32_t _height;
		TextureHandle _handle;
		TextureType type;
	private:
		Result _loadResourceImpl(const std::filesystem::path& path) override;
	};
}