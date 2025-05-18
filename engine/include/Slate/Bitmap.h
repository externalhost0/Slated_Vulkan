//
// Created by Hayden Rivas on 5/13/25.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Slate {
	enum BitmapType
	{
		BitmapType_2D,
		BitmapType_Cube
	};
	enum BitmapFormat
	{
		BitmapFormat_UnsignedByte,
		BitmapFormat_Float,
	};

	struct Bitmap
	{
		Bitmap() = default;
		Bitmap(int w, int h, int comp, BitmapFormat fmt)
			: _width(w), _height(h), _comp(comp), _format(fmt), _data(w * h * comp * getBytesPerComponent(fmt))
		{
			initGetSetFuncs();
		}
		Bitmap(int w, int h, int d, int comp, BitmapFormat fmt)
			: _width(w), _height(h), _depth(d), _comp(comp), _format(fmt), _data(w * h * d * comp * getBytesPerComponent(fmt))
		{
			initGetSetFuncs();
		}
		Bitmap(int w, int h, int comp, BitmapFormat fmt, const void* ptr)
			: _width(w), _height(h), _comp(comp), _format(fmt), _data(w * h * comp * getBytesPerComponent(fmt))
		{
			initGetSetFuncs();
			memcpy(_data.data(), ptr, _data.size());
		}
		int _width = 0;
		int _height = 0;
		int _depth = 1;
		int _comp = 3;
		BitmapFormat _format = BitmapFormat_UnsignedByte;
		BitmapType _type = BitmapType_2D;
		std::vector<uint8_t> _data;

		static int getBytesPerComponent(BitmapFormat fmt) {
			if (fmt == BitmapFormat_UnsignedByte) return 1;
			if (fmt == BitmapFormat_Float) return 4;
			return 0;
		}
		void setPixel(int x, int y, const glm::vec4& c) {
			(*this.*setPixelFunc)(x, y, c);
		}
		glm::vec4 getPixel(int x, int y) const {
			return ((*this.*getPixelFunc)(x, y));
		}
	private:
		using setPixel_t = void(Bitmap::*)(int, int, const glm::vec4&);
		using getPixel_t = glm::vec4(Bitmap::*)(int, int) const;
		setPixel_t setPixelFunc = &Bitmap::setPixelUnsignedByte;
		getPixel_t getPixelFunc = &Bitmap::getPixelUnsignedByte;

		void initGetSetFuncs() {
			switch (_format) {
				case BitmapFormat_UnsignedByte:
					setPixelFunc = &Bitmap::setPixelUnsignedByte;
					getPixelFunc = &Bitmap::getPixelUnsignedByte;
					break;
				case BitmapFormat_Float:
					setPixelFunc = &Bitmap::setPixelFloat;
					getPixelFunc = &Bitmap::getPixelFloat;
					break;
			}
		}

		void setPixelFloat(int x, int y, const glm::vec4& c) {
			const int ofs = _comp * (y * _width + x);
			float* data = reinterpret_cast<float*>(_data.data());
			if (_comp > 0) data[ofs + 0] = c.x;
			if (_comp > 1) data[ofs + 1] = c.y;
			if (_comp > 2) data[ofs + 2] = c.z;
			if (_comp > 3) data[ofs + 3] = c.w;
		}
		glm::vec4 getPixelFloat(int x, int y) const {
			const int ofs = _comp * (y * _width + x);
			const float* data = reinterpret_cast<const float*>(_data.data());
			return {
					_comp > 0 ? data[ofs + 0] : 0.0f,
					_comp > 1 ? data[ofs + 1] : 0.0f,
					_comp > 2 ? data[ofs + 2] : 0.0f,
					_comp > 3 ? data[ofs + 3] : 0.0f
			};
		}
		void setPixelUnsignedByte(int x, int y, const glm::vec4& c) {
			const int ofs = _comp * (y * _width + x);
			if (_comp > 0) _data[ofs + 0] = uint8_t(c.x * 255.0f);
			if (_comp > 1) _data[ofs + 1] = uint8_t(c.y * 255.0f);
			if (_comp > 2) _data[ofs + 2] = uint8_t(c.z * 255.0f);
			if (_comp > 3) _data[ofs + 3] = uint8_t(c.w * 255.0f);
		}
		glm::vec4 getPixelUnsignedByte(int x, int y) const {
			const int ofs = _comp * (y * _width + x);
			return {
					_comp > 0 ? float(_data[ofs + 0]) / 255.0f : 0.0f,
					_comp > 1 ? float(_data[ofs + 1]) / 255.0f : 0.0f,
					_comp > 2 ? float(_data[ofs + 2]) / 255.0f : 0.0f,
					_comp > 3 ? float(_data[ofs + 3]) / 255.0f : 0.0f
			};
		}
	};

	Bitmap ConvertEquirectangularMapToVerticalCross(const Bitmap& b);
	Bitmap ConvertVerticalCrossToCubeMapFaces(const Bitmap& b);
	inline Bitmap ConvertEquirectangularMapToCubeMapFaces(const Bitmap& b) {
		return ConvertVerticalCrossToCubeMapFaces(ConvertEquirectangularMapToVerticalCross(b));
	}
}