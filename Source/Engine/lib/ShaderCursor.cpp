//
// Created by Hayden Rivas on 5/3/25.
//
#include <Slate/ShaderCursor.h>
#include <cstring>


namespace Slate {
	void ICursor::write(const void* data, size_t size) {
		if (_isHostVisible) {
			memcpy(_bufferData + _byteOffset, data, size);
		} else {
//			vkCmdUpdateBuffer(cmd, _buffer);
		}
	}
}