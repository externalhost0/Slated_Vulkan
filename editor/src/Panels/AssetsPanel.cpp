//
// Created by Hayden Rivas on 1/16/25.
//
#include "../Editor.h"
namespace Slate {
	void EditorApplication::_onAssetPanelUpdate() {
		ImGui::Begin("Assets");

		if (ImGui::Button("Make new image")) {
			const uint32_t texWidth = 256;
			const uint32_t texHeight = 256;
			std::vector<uint32_t> pixels(texWidth * texHeight);

			const float centerX = texWidth / 2.0f;
			const float centerY = texHeight / 2.0f;
			const float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

			for (uint32_t y = 0; y < texHeight; y++) {
				for (uint32_t x = 0; x < texWidth; x++) {
					float dx = x - centerX;
					float dy = y - centerY;
					float dist = std::sqrt(dx * dx + dy * dy);
					uint8_t value = static_cast<uint8_t>(255.0f * (1.0f - (dist / maxDist)));

					// Make it blueish tinted
					pixels[y * texWidth + x] = 0xFF000000 | (value << 16) | (value << 8) | 0xFF;
				}
			}
			getGX().createTexture({
					.dimension = { texWidth, texHeight },
					.usage = TextureUsageBits::TextureUsageBits_Sampled,
					.format = VK_FORMAT_R8G8B8A8_UNORM,
					.data = pixels.data(),
					.debugName = "Radial Gradient Texture"
			});
		}
		ImGui::End();
	}
}