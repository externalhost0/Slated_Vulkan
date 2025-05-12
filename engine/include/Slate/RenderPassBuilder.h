//
// Created by Hayden Rivas on 5/11/25.
//

#pragma once

#include <volk.h>

#include "Slate/GX.h"
namespace Slate {

	struct IAttachment {
		TextureHandle texture = {};
		LoadOperation loadOp = LoadOperation::NO_CARE;
		StoreOperation storeOp = StoreOperation::NO_CARE;
	};
	struct ColorAttachment : IAttachment {
		RGBA clear = {0, 0, 0, 0};
	};
	struct DepthStencilAttachment : IAttachment {
		float clear = {};
	};
	struct MultisampledColorAttachment : ColorAttachment {
		ResolveMode resolveMode = ResolveMode::AVERAGE;
	};
	struct MultisampledDepthStencilAttachment : DepthStencilAttachment {
		ResolveMode resolveMode = ResolveMode::SAMPLE_ZERO;
	};

	class RenderPassBuilder {
	public:
		RenderPassBuilder() { Clear(); }
		~RenderPassBuilder() = default;
	public:
		RenderPassBuilder& addColorAttachment(TextureHandle texture);
		RenderPassBuilder& addDepthStencilAttachment(TextureHandle texture);

		RenderPassBuilder& addMultisampledColorAttachment();

	private:
		void Clear();
	private:

	};
}