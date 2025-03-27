// entry point into the editor
// we want to inherit all of the functionality the application class provides
#include "Editor.h"
int main(int argc, char* argv[]) {
	Slate::Editor app;
	app.Run();
	return 0;
}
