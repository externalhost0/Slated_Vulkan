
#include "EditorApplication.h"

int main() {

	auto editorapp = Slate::Application::Create<Slate::EditorApplication>();

	editorapp->Run();

	return 0;
}
