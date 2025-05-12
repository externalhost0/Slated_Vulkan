//
// Created by Hayden Rivas on 3/16/25.
//
#include "Slate/Loaders/ShaderLoader.h"
#include "Slate/Common/Debug.h"
#include "Slate/Filesystem.h"

#include <slang/slang.h>

namespace Slate {
	Slang::ComPtr<slang::ISession> ShaderLoader::GetSession() {
		if (session) {
			return session;
		} else {
			CreateSession();
			return session;
		}
	}
	void ShaderLoader::CreateSession() {

		SlangGlobalSessionDesc globalDesc = {
				.apiVersion = SLANG_API_VERSION,
				.enableGLSL = false
		};
		ASSERT_MSG(slang::createGlobalSession(&globalDesc, globalSession.writeRef()) == 0, "Failed to create global Slang session!");

		// session info
		// profile spirv_1_5 for Vulkan 1.2
		// profile spirv_1_6 for Vulkan 1.3
		slang::TargetDesc targetDesc = {
				.format = SLANG_SPIRV,
				.profile = globalSession->findProfile("spirv_1_5")
		};
		std::array<slang::CompilerOptionEntry, 5> entries = {
				slang::CompilerOptionEntry{
						.name = slang::CompilerOptionName::VulkanUseEntryPointName,
						.value = {
								.kind = slang::CompilerOptionValueKind::Int,
								.intValue0 = true
						}
				},
				slang::CompilerOptionEntry{
						.name = slang::CompilerOptionName::EmitSpirvDirectly,
						.value = {
								.kind = slang::CompilerOptionValueKind::Int,
								.intValue0 = true
						}
				},
				slang::CompilerOptionEntry{
						.name = slang::CompilerOptionName::Optimization,
						.value = {
								.kind = slang::CompilerOptionValueKind::Int,
								.intValue0 = SLANG_OPTIMIZATION_LEVEL_DEFAULT
						}
				},
				slang::CompilerOptionEntry{
						.name = slang::CompilerOptionName::VulkanUseGLLayout,
						.value = {
								.kind = slang::CompilerOptionValueKind::Int,
								.intValue0 = true
						}
				},
				slang::CompilerOptionEntry{
					.name = slang::CompilerOptionName::VulkanInvertY,
					.value = {
							.kind = slang::CompilerOptionValueKind::Int,
							.intValue0 = true
					}
				}
		};

		std::string builtin = Filesystem::GetRelativePath("shaders/BuiltIn/");
		std::array<const char*, 1> paths = { builtin.c_str() };

		slang::SessionDesc sessionDesc = {
				.targets = &targetDesc,
				.targetCount = 1,

				.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,

				.searchPaths = paths.data(),
				.searchPathCount = paths.size(),

				.compilerOptionEntries = entries.data(),
				.compilerOptionEntryCount = entries.size(),
		};
		globalSession->createSession(sessionDesc, session.writeRef());
	}
}