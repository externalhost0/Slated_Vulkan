//
// Created by Hayden Rivas on 4/28/25.
//

#pragma once

#include <cstdint>

#include "Slate/Common/Debug.h"

namespace Slate {
	struct SubmitHandle
	{
		SubmitHandle() = default;
		explicit SubmitHandle(uint64_t handle) : bufferIndex_(uint32_t(handle & 0xffffffff)), submitId_(uint32_t(handle >> 32)) {
			ASSERT_MSG(submitId_, "Submit handle is invalid!");
		}

		uint32_t bufferIndex_ = 0;
		uint32_t submitId_ = 0;
		bool empty() const {
			return submitId_ == 0;
		}
		uint64_t handle() const {
			return (uint64_t(submitId_) << 32) + bufferIndex_;
		}
	};
}