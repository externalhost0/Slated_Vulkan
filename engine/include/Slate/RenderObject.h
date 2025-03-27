//
// Created by Hayden Rivas on 3/11/25.
//

#pragma once
#include "Node.h"
#include "Slate/VK/vktypes.h"
#include "volk.h"

namespace Slate {
	// things every draw call needs
	struct RenderObject : IRenderable {
		GPU::DrawPushConstants push = {};
		Shared<Material> mat_instance;
	};

	struct VertexOnlyObject_EXT : protected RenderObject {
		uint32_t vertexCount = 0;

		void Draw(DrawContext& ctx) const {
			vkCmdPushConstants(ctx.cmd, nullptr, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &this->push);
			vkCmdDraw(ctx.cmd, this->vertexCount, 1, 0, 0);
		}
	};

	struct IndexedObject : protected RenderObject {
		uint32_t indexCount = 0;
		uint32_t firstIndex = 0;
		vktypes::AllocatedBuffer indexBuffer;

		virtual void Draw(DrawContext& ctx) const {
			vkCmdBindIndexBuffer(ctx.cmd, this->indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdPushConstants(ctx.cmd, nullptr, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &this->push);
			vkCmdDrawIndexed(ctx.cmd, this->indexCount, 1, this->firstIndex, 0, 0);
		}
	};

	struct InstancedObject : private IndexedObject {
		uint32_t instanceCount = 0;

		void Draw(DrawContext& ctx) const override {
			vkCmdBindIndexBuffer(ctx.cmd, this->indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdPushConstants(ctx.cmd, nullptr, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPU::DrawPushConstants), &this->push);
			//			vkCmdDrawIndexedIndirect(ctx.cmd, );
		}
	};

}