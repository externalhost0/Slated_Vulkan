//
// Created by Hayden Rivas on 2/26/25.
//

#pragma once
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

namespace Slate {
	class ShaderLoader {
	public:
		static void CreateSession();
		static Slang::ComPtr<slang::ISession> GetSession();
	private:
		static inline Slang::ComPtr<slang::IGlobalSession> globalSession = nullptr;
		static inline Slang::ComPtr<slang::ISession> session = nullptr;
	};

}