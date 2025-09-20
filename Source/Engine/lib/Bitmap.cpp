//
// Created by Hayden Rivas on 5/13/25.
//

#include "Slate/Bitmap.h"

namespace Slate {
	glm::vec3 FaceCoordsToXYZ(int i, int j, int faceID, int faceSize) {
		const float A = 2.0f * float(i) / static_cast<float>(faceSize);
		const float B = 2.0f * float(j) / static_cast<float>(faceSize);

		if (faceID == 0) return {-1.0f, A - 1.0f, B - 1.0f};
		if (faceID == 1) return {A - 1.0f, -1.0f, 1.0f - B};
		if (faceID == 2) return {1.0f, A - 1.0f, 1.0f - B};
		if (faceID == 3) return {1.0f - A, 1.0f, 1.0f - B};
		if (faceID == 4) return {B - 1.0f, A - 1.0f, 1.0f};
		if (faceID == 5) return {1.0f - B, A - 1.0f, -1.0f};
		return {};
	}

	Bitmap ConvertEquirectangularMapToVerticalCross(const Bitmap& b) {
		if (b._type != BitmapType_2D) return {};

		const int faceSize = b._width / 4;
		const int w = faceSize * 3;
		const int h = faceSize * 4;

		Bitmap result(w, h, b._comp, b._format);

		const glm::ivec2 kFaceOffsets[] = {
				glm::ivec2(faceSize, faceSize * 3),
				glm::ivec2(0, faceSize),
				glm::ivec2(faceSize, faceSize),
				glm::ivec2(faceSize * 2, faceSize),
				glm::ivec2(faceSize, 0),
				glm::ivec2(faceSize, faceSize * 2)
		};
		const int clampW = b._width - 1;
		const int clampH = b._height - 1;

		for (int face = 0; face != 6; face++)
		{
			for (int i = 0; i != faceSize; i++)
			{
				for (int j = 0; j != faceSize; j++)
				{
					const glm::vec3 P = FaceCoordsToXYZ(i, j, face, faceSize);
					const float R = hypot(P.x, P.y);
					const float theta = atan2(P.y, P.x);
					const float phi = atan2(P.z, R);
					//	float point source coordinates
					const float Uf = float(2.0f * (float)faceSize * (theta + M_PI) / M_PI);
					const float Vf = float(2.0f * (float)faceSize * (M_PI / 2.0f - phi) / M_PI);
					// 4-samples for bilinear interpolation
					const int U1 = glm::clamp(int(floor(Uf)), 0, clampW);
					const int V1 = glm::clamp(int(floor(Vf)), 0, clampH);
					const int U2 = glm::clamp(U1 + 1, 0, clampW);
					const int V2 = glm::clamp(V1 + 1, 0, clampH);
					// fractional part
					const float s = Uf - (float)U1;
					const float t = Vf - (float)V1;
					// fetch 4-samples
					const glm::vec4 A = b.getPixel(U1, V1);
					const glm::vec4 B = b.getPixel(U2, V1);
					const glm::vec4 C = b.getPixel(U1, V2);
					const glm::vec4 D = b.getPixel(U2, V2);
					// bilinear interpolation
					const glm::vec4 color = A * (1 - s) * (1 - t) + B * (s) * (1 - t) + C * (1 - s) * t + D * (s) * (t);
					result.setPixel(i + kFaceOffsets[face].x, j + kFaceOffsets[face].y, color);
				}
			};
		}
		return result;
	}

	Bitmap ConvertVerticalCrossToCubeMapFaces(const Bitmap& b) {
		const int faceWidth  = b._width / 3;
		const int faceHeight = b._height / 4;

		Bitmap cubemap(faceWidth, faceHeight, 6, b._comp, b._format);
		cubemap._type = BitmapType_Cube;

		const uint8_t* src = b._data.data();
		uint8_t* dst = cubemap._data.data();
		/*
			------
   		   | +Y |
   		----------------
   		| -X | -Z | +X |
   		----------------
   		   | -Y |
   		   ------
   		   | +Z |
   		   ------
		*/

		const int pixelSize = cubemap._comp * Bitmap::getBytesPerComponent(cubemap._format);

		for (int face = 0; face != 6; ++face) {
			for (int j = 0; j != faceHeight; ++j) {
				for (int i = 0; i != faceWidth; ++i) {
					int x = 0;
					int y = 0;
					switch (face) {
						// CUBE_MAP_POSITIVE_X
						case 0:
							x = 2 * faceWidth + i;
							y = 1 * faceHeight + j;
							break;
						// CUBE_MAP_NEGATIVE_X
						case 1:
							x = i;
							y = faceHeight + j;
							break;
						// CUBE_MAP_POSITIVE_Y
						case 2:
							x = 1 * faceWidth + i;
							y = j;
							break;
						// CUBE_MAP_NEGATIVE_Y
						case 3:
							x = 1 * faceWidth + i;
							y = 2 * faceHeight + j;
							break;
						// CUBE_MAP_POSITIVE_Z
						case 4:
							x = faceWidth + i;
							y = faceHeight + j;
							break;
						// CUBE_MAP_NEGATIVE_Z
						case 5:
							x = 2 * faceWidth - (i + 1);
							y = b._height - (j + 1);
							break;
						default:
							break;
					}
					memcpy(dst, src + (y * b._width + x) * pixelSize, pixelSize);
					dst += pixelSize;
				}
			}
		}
		return cubemap;
	}
}