//
// Created by Hayden Rivas on 5/3/25.
//

#pragma once
#include <cstddef>

#include <volk.h>

// forward predeclare
namespace slang {
	class TypeLayoutReflection;
}
class AllocatedImage;
namespace Slate {
	struct ICursor {
	public:
		template<class Struct>
		void write(const Struct& type) {
			write(&type, sizeof(Struct));
		}
		void write(const void* data, size_t size);

		ICursor* field(int index);
		ICursor* field(const char* name);

		ICursor* element(int index);
	private:
		std::byte*      _bufferData;
		size_t			_byteOffset;

		VkBuffer        _buffer;
//		VkDescriptorSet	_descriptorSet = VK_NULL_HANDLE;

		uint32_t		_bindingIndex;
		uint32_t 		_bindingArrayIndex;

		slang::TypeLayoutReflection* _typeLayout;

		bool _isHostVisible = false;
	};

	struct StructCursor : public ICursor {
	public:
		void write(const void* data, size_t size);
	private:

	};
	struct TextureCursor : public ICursor {
	public:
		void write(const AllocatedImage& texture);
	private:

	};
	struct SamplerCursor : public ICursor {
	public:

	private:

	};
}




