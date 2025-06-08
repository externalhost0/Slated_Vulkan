//
// Created by Hayden Rivas on 6/4/25.
//
#include <Slate/IApplication.h>
class App : public Slate::IApplication {
	void onInitialize() override;
	void onTick() override;
	void onRender() override;
	void onShutdown() override;
};
void App::onInitialize() {
	// take user start toml
	Slate::WindowSpec spec = {};
	spec.resizeable = true;
	spec.videomode = Slate::VideoMode::Windowed;
	spec.title = "Slate App";
	createWindow(spec);

}
void App::onTick() {

}
void App::onRender() {

}
void App::onShutdown() {

}

int main(int argc, char* argv[]) {
	App app;
	app.run();
	return 0;
}