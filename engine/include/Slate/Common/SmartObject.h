//
// Created by Hayden Rivas on 5/8/25.
//

#pragma once
namespace Slate {
	class GX;
	void destroy(GX* gx, BufferHandle handle);

	template<typename HandleType>
	class Holder final {
	public:
		Holder() = default;
		Holder(GX* ctx, HandleType handle) : ctx_(ctx), handle_(handle) {}
		~Holder() {
			destroy(ctx_, handle_);
		}
		Holder(const Holder&) = delete;
		Holder(Holder&& other)  noexcept : ctx_(other.ctx_), handle_(other.handle_) {
			other.ctx_ = nullptr;
			other.handle_ = HandleType{};
		}
		Holder& operator=(const Holder&) = delete;
		Holder& operator=(Holder&& other) {
			std::swap(ctx_, other.ctx_);
			std::swap(handle_, other.handle_);
			return *this;
		}
		Holder& operator=(std::nullptr_t) {
			this->reset();
			return *this;
		}

		// we want the conversions to be implicit
		inline operator HandleType() const {
			return handle_;
		}

		bool valid() const {
			return handle_.valid();
		}

		bool empty() const {
			return handle_.empty();
		}

		void reset() {
			destroy(ctx_, handle_);
			ctx_ = nullptr;
			handle_ = HandleType{};
		}

		HandleType release() {
			ctx_ = nullptr;
			return std::exchange(handle_, HandleType{});
		}

		uint32_t gen() const {
			return handle_.gen();
		}
		uint32_t index() const {
			return handle_.index();
		}
		void* indexAsVoid() const {
			return handle_.indexAsVoid();
		}

	private:
		GX* ctx_ = nullptr;
		HandleType handle_ = {};
	};

}