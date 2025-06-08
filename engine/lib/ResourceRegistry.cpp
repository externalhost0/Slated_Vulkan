//
// Created by Hayden Rivas on 5/22/25.
//
#include "Slate/ResourceRegistry.h"
namespace Slate {

	template<class T>
	ResourceHandle<T> ResourceRegistry::load(const std::filesystem::path& filepath) {
		ResourceType type_enum;
		{
			const char* extension = filepath.extension().c_str();
			extension++;
			type_enum = ResolveResourceTypeFromFileExtension(extension);
		}
		switch (type_enum) {
			case ResourceType::Shader: return _shaderPool.create(filepath);
			case ResourceType::Mesh: return _meshPool.create(filepath);
			case ResourceType::Audio:
				break;
			case ResourceType::Font:
				break;
			case ResourceType::Script: return _scriptPool.create(filepath);
			case ResourceType::Image: return _texturePool.create(filepath);
			default: break;
		}
	}
	template<class T>
	T& ResourceRegistry::get(ResourceHandle<T> handle) {

	}
}