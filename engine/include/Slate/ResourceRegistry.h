//
// Created by Hayden Rivas on 5/22/25.
//

#pragma once

#include <Slate/ResourcePool.h>

#include <Slate/Resources/MeshResource.h>
#include <Slate/Resources/ShaderResource.h>
#include <Slate/Resources/TextureResource.h>
#include <Slate/Resources/ScriptResource.h>

namespace Slate {
	enum class ResourceType {
		Unknown = 0,
		Shader,
		Mesh,
		Audio,
		Font,
		Script,
		Image
	};
	constexpr ResourceType ResolveResourceTypeFromFileExtension(const char* requested_extension) {
		struct ExtensionMapEntry {
			ResourceType type;
			std::string_view extension;
		};
		constexpr std::array<ExtensionMapEntry, 12> kExtensionMap = {{
				{ ResourceType::Audio, "mp3" },
				{ ResourceType::Audio, "wav" },

				{ ResourceType::Shader, "slang" },

				{ ResourceType::Font, "ttf" },
				{ ResourceType::Font, "otf" },

				{ ResourceType::Mesh, "gltf" },
				{ ResourceType::Mesh, "glb" },

				{ ResourceType::Script, "lua" },

				{ ResourceType::Image, "png" },
				{ ResourceType::Image, "jpg" },
				{ ResourceType::Image, "jpeg" },
				{ ResourceType::Image, "bmp" },
		}};
		for (const auto& entry : kExtensionMap) {
			if (requested_extension == entry.extension) {
				return entry.type;
			}
		}
		return ResourceType::Unknown;
	}


	class ResourceRegistry {
	public:
		ResourceRegistry() = default;
		~ResourceRegistry() = default;

		template<class T>
		ResourceHandle<T> load(const std::filesystem::path& filepath);

		template<class T>
		T& get(ResourceHandle<T> handle);


	private:
		ResourcePool<MeshResource> _meshPool;
		ResourcePool<ScriptResource> _scriptPool;
		ResourcePool<ShaderResource> _shaderPool;
		ResourcePool<TextureResource> _texturePool;
//		ResourcePool<AudioResource> _audioPool;
	};

}