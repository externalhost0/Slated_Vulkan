//
// Created by Hayden Rivas on 5/15/25.
//


#include "Slate/ScriptEngine.h"
#include "Slate/Filesystem.h"
#include "Slate/Common/Logger.h"


#include <string>

#define SLATE_ADD_INTERNAL_CALL(FuncName) mono_add_internal_call("Slate.InternalCalls::" #FuncName, Name);

namespace Slate {
	ScriptEngine::ScriptEngine() {

	}
	ScriptEngine::~ScriptEngine() {
		
	}
}