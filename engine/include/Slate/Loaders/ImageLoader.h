//
// Created by Hayden Rivas on 4/17/25.
//

#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Slate/GX.h"
#include <filesystem>

namespace Slate {
	struct TextureSpec;

	class ImageLoader final {
	public:
		static TextureSpec loadFromPath(const std::filesystem::path& file_path);
	private:

	};
}