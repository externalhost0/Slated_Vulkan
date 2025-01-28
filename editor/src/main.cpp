// entry point into the editor
// we want to inherit all of the functionality the application class provides
#include "EditorApplication.h"
int main(int argc, char* argv[]) {
	auto editorapp = Slate::Application::Create<Slate::EditorApplication>();
	editorapp->Run();
	return 0;
}
