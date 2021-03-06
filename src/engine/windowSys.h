#pragma once

#include "drawSys.h"
#include "scene.h"
#include "prog/program.h"
#include "utils/settings.h"

// runs and kills everything, passes events, handles window events and contains settings
class WindowSys {
public:
	WindowSys();

	int start();
	void close() { run = false; }
	
	vec2i resolution() const;
	static vec2i mousePos();
	static string getRendererName(int id);
	static vector<string> getAvailibleRenderers();

	DrawSys* getDrawSys() { return drawSys.get(); }
	Scene* getScene() { return scene.get(); }
	Program* getProgram() { return program.get(); }

	const Settings& getSettings() const { return sets; }
	FontSet& getFontSet() { return sets.getFontSet(); }
	void setResolution(const vec2i& res);
	void setResolution(const string& line);
	void setViewPos(const vec2f& pos) { sets.viewPos = pos; }
	void setViewSize(const vec2f& size) { sets.viewSize = size; }
	void setViewport(const string& line);
	void setRenderer(const string& name);
	void setFullscreen(bool on);
	void setFont(const string& font);
	void setScrollSpeed(int ss) { sets.scrollSpeed = ss; }
	void resetSettings();

private:
	uptr<Program> program;
	uptr<Scene> scene;
	uptr<DrawSys> drawSys;
	SDL_Window* window;

	Settings sets;
	bool run;		// whether the loop in which the program runs should continue
	
	void createWindow();
	void destroyWindow();
	
	void handleEvent(const SDL_Event& event);	// pass events to their specific handlers
	void eventWindow(const SDL_WindowEvent& window);
};
